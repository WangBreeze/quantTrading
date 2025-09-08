#include "onlinemarket_okb.h"
#include <QUrlQuery>

onLineMarket_OKB::onLineMarket_OKB(QObject *parent)
    : onLineMarket(parent)
    , m_networkManager(nullptr)
    , m_dataFetchTimer(nullptr)
    , m_webSocket(nullptr)
    , m_webSocketConnected(false)
    , m_symbol("BTC-USDT")  // 默认交易对
    , m_isActive(false)
    , m_dataFetchMode(REST_API)  // 默认使用REST API模式
{
    initialize();
}

onLineMarket_OKB::~onLineMarket_OKB()
{
    // 停止所有数据获取
    stopFetchingData();
    
    // 清理REST API资源
    if (m_dataFetchTimer) {
        m_dataFetchTimer->stop();
        delete m_dataFetchTimer;
    }
    
    if (m_networkManager) {
        delete m_networkManager;
    }
    
    // 清理WebSocket资源
    if (m_webSocket) {
        if (m_webSocket->isValid()) {
            m_webSocket->close();
        }
        delete m_webSocket;
    }
}

void onLineMarket_OKB::initialize()
{
    // 初始化REST API相关资源
    m_networkManager = new QNetworkAccessManager(this);
    
    // 初始化定时器，默认1秒获取一次数据（秒级数据）
    m_dataFetchTimer = new QTimer(this);
    m_dataFetchTimer->setInterval(1000);
    connect(m_dataFetchTimer, &QTimer::timeout, this, &onLineMarket_OKB::fetchMarketData);
    
    // 构建API URL
    m_apiUrl = QUrl(API_BASE_URL + API_MARKET_ENDPOINT);
    
    // 初始化WebSocket（但不连接）
    initializeWebSocket();
}

void onLineMarket_OKB::initializeWebSocket()
{
    // 创建WebSocket对象
    if (!m_webSocket) {
        m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
        
        // 连接WebSocket信号
        connect(m_webSocket, &QWebSocket::connected, this, &onLineMarket_OKB::onWebSocketConnected);
        connect(m_webSocket, &QWebSocket::disconnected, this, &onLineMarket_OKB::onWebSocketDisconnected);
        connect(m_webSocket, &QWebSocket::textMessageReceived, this, &onLineMarket_OKB::onWebSocketTextMessageReceived);
        connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
                this, &onLineMarket_OKB::onWebSocketError);
    }
}

void onLineMarket_OKB::setDataFetchMode(DataFetchMode mode)
{
    // 如果模式没有变化，直接返回
    if (m_dataFetchMode == mode) {
        return;
    }
    
    // 如果当前正在获取数据，先停止
    bool wasActive = m_isActive;
    if (wasActive) {
        stopFetchingData();
    }
    
    // 设置新的数据获取模式
    m_dataFetchMode = mode;
    
    // 如果之前是活动状态，使用新模式重新开始获取数据
    if (wasActive) {
        startFetchingData();
    }
}

void onLineMarket_OKB::setSymbol(const QString &symbol)
{
    m_symbol = symbol;
}

void onLineMarket_OKB::setUpdateInterval(int milliseconds)
{
    // 确保间隔时间合理（至少100毫秒，避免过于频繁的请求）
    int interval = qMax(100, milliseconds);
    
    // 设置定时器间隔
    if (m_dataFetchTimer) {
        m_dataFetchTimer->setInterval(interval);
        
        // 如果定时器已经在运行，重新启动以应用新的间隔
        if (m_isActive && m_dataFetchTimer->isActive()) {
            m_dataFetchTimer->stop();
            m_dataFetchTimer->start();
        }
    }
}

void onLineMarket_OKB::startFetchingData()
{
    if (!m_isActive) {
        m_isActive = true;
        
        if (m_dataFetchMode == REST_API) {
            // REST API模式：立即获取一次数据，然后启动定时器
            fetchMarketData();
            m_dataFetchTimer->start();
        } else {
            // WebSocket模式：连接WebSocket并订阅频道
            if (!m_webSocketConnected && m_webSocket) {
                m_webSocket->open(QUrl(WS_BASE_URL));
            } else if (m_webSocketConnected) {
                // 如果已经连接，直接订阅频道
                subscribeToChannel();
            }
        }
    }
}

void onLineMarket_OKB::stopFetchingData()
{
    if (m_isActive) {
        m_isActive = false;
        
        if (m_dataFetchMode == REST_API) {
            // REST API模式：停止定时器
            m_dataFetchTimer->stop();
        } else {
            // WebSocket模式：取消订阅并关闭连接
            if (m_webSocketConnected && m_webSocket) {
                // 发送取消订阅消息
                QJsonObject unsubObj;
                unsubObj["op"] = "unsubscribe";
                
                QJsonArray argsArray;
                QJsonObject channelObj;
                channelObj["channel"] = "tickers";
                channelObj["instId"] = m_symbol;
                argsArray.append(channelObj);
                
                unsubObj["args"] = argsArray;
                
                QString unsubMessage = QJsonDocument(unsubObj).toJson(QJsonDocument::Compact);
                m_webSocket->sendTextMessage(unsubMessage);
                
                // 关闭WebSocket连接
                m_webSocket->close();
            }
        }
    }
}

void onLineMarket_OKB::fetchMarketData()
{
    // 构建请求URL，添加查询参数
    QUrl requestUrl = m_apiUrl;
    QUrlQuery query;
    query.addQueryItem("instId", m_symbol);
    requestUrl.setQuery(query);
    
    // 创建请求
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // 发送GET请求
    QNetworkReply *reply = m_networkManager->get(request);
    
    // 连接信号处理响应
    connect(reply, &QNetworkReply::finished, this, &onLineMarket_OKB::onNetworkReplyFinished);
}

void onLineMarket_OKB::onNetworkReplyFinished()
{
    // 获取发送信号的回复对象
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // 确保在函数结束时删除reply对象
    reply->deleteLater();
    
    // 检查是否有错误
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(tr("网络错误: %1").arg(reply->errorString()));
        return;
    }
    
    // 读取所有响应数据
    QByteArray responseData = reply->readAll();
    
    // 解析数据
    parseMarketData(responseData);
}

void onLineMarket_OKB::parseMarketData(const QByteArray &data)
{
    // 解析JSON响应
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(tr("JSON解析错误: %1").arg(parseError.errorString()));
        return;
    }
    
    // 检查响应格式
    QJsonObject rootObj = jsonDoc.object();
    
    // 检查是否有错误码
    if (rootObj["code"].toString() != "0") {
        QString errorMsg = rootObj["msg"].toString();
        emit errorOccurred(tr("API错误: %1").arg(errorMsg));
        return;
    }
    
    // 获取数据数组
    QJsonArray dataArray = rootObj["data"].toArray();
    if (dataArray.isEmpty()) {
        emit errorOccurred(tr("没有返回数据"));
        return;
    }
    
    // 获取第一个数据对象（通常只有一个）
    QJsonObject tickerObj = dataArray.first().toObject();
    
    // 提取行情数据
    QString symbol = tickerObj["instId"].toString();
    double lastPrice = tickerObj["last"].toString().toDouble();
    double volume24h = tickerObj["vol24h"].toString().toDouble();
    double high24h = tickerObj["high24h"].toString().toDouble();
    double low24h = tickerObj["low24h"].toString().toDouble();
    double openPrice = tickerObj["open24h"].toString().toDouble();
    
    // 发出信号通知新数据
    emit newMarketData(symbol, lastPrice, volume24h, high24h, low24h, openPrice, lastPrice);
}

// WebSocket相关方法实现
void onLineMarket_OKB::onWebSocketConnected()
{
    m_webSocketConnected = true;
    
    // 连接成功后，订阅行情频道
    if (m_isActive) {
        subscribeToChannel();
    }
}

void onLineMarket_OKB::onWebSocketDisconnected()
{
    m_webSocketConnected = false;
    
    // 如果仍处于活动状态，尝试重新连接
    if (m_isActive && m_dataFetchMode == WEBSOCKET) {
        // 使用定时器延迟重连，避免立即重连可能导致的问题
        QTimer::singleShot(3000, [this]() {
            if (m_isActive && m_dataFetchMode == WEBSOCKET && !m_webSocketConnected) {
                m_webSocket->open(QUrl(WS_BASE_URL));
            }
        });
    }
}

void onLineMarket_OKB::onWebSocketTextMessageReceived(const QString &message)
{
    // 解析WebSocket消息
    parseWebSocketMessage(message);
}

void onLineMarket_OKB::onWebSocketError(QAbstractSocket::SocketError error)
{
    emit errorOccurred(tr("WebSocket错误: %1").arg(m_webSocket->errorString()));
    
    // 如果发生错误，标记为断开连接
    m_webSocketConnected = false;
}

void onLineMarket_OKB::subscribeToChannel()
{
    if (!m_webSocketConnected || !m_webSocket) {
        return;
    }
    
    // 构建订阅消息
    QJsonObject subObj;
    subObj["op"] = "subscribe";
    
    QJsonArray argsArray;
    QJsonObject channelObj;
    channelObj["channel"] = "tickers";  // 订阅ticker频道，提供实时行情数据
    channelObj["instId"] = m_symbol;    // 设置交易对
    argsArray.append(channelObj);
    
    subObj["args"] = argsArray;
    
    // 转换为JSON字符串并发送
    QString subMessage = QJsonDocument(subObj).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(subMessage);
}

void onLineMarket_OKB::parseWebSocketMessage(const QString &message)
{
    // 解析JSON消息
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(tr("WebSocket JSON解析错误: %1").arg(parseError.errorString()));
        return;
    }
    
    QJsonObject rootObj = jsonDoc.object();
    
    // 检查是否是事件消息
    if (rootObj.contains("event")) {
        QString event = rootObj["event"].toString();
        
        // 处理订阅成功事件
        if (event == "subscribe") {
            // 订阅成功，可以记录日志或执行其他操作
            return;
        }
        
        // 处理错误事件
        if (event == "error") {
            QString errorMsg = rootObj["msg"].toString();
            emit errorOccurred(tr("WebSocket API错误: %1").arg(errorMsg));
            return;
        }
        
        // 其他事件类型，暂不处理
        return;
    }
    
    // 检查是否是数据消息
    if (rootObj.contains("data")) {
        // 获取数据数组
        QJsonArray dataArray = rootObj["data"].toArray();
        if (dataArray.isEmpty()) {
            return;
        }
        
        // 获取第一个数据对象
        QJsonObject tickerObj = dataArray.first().toObject();
        
        // 提取行情数据
        QString symbol = tickerObj["instId"].toString();
        double lastPrice = tickerObj["last"].toString().toDouble();
        double volume24h = tickerObj["vol24h"].toString().toDouble();
        double high24h = tickerObj["high24h"].toString().toDouble();
        double low24h = tickerObj["low24h"].toString().toDouble();
        double openPrice = tickerObj["open24h"].toString().toDouble();
        
        // 发出信号通知新数据
        emit newMarketData(symbol, lastPrice, volume24h, high24h, low24h, openPrice, lastPrice);
    }
}