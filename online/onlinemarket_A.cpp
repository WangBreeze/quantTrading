#include "onlinemarket_A.h"
#include <QUrlQuery>
#include <QRandomGenerator>

onLineMarket_A::onLineMarket_A(QObject *parent)
    : onLineMarket(parent)
    , m_networkManager(nullptr)
    , m_dataFetchTimer(nullptr)
    , m_webSocket(nullptr)
    , m_webSocketConnected(false)
    , m_heartbeatTimer(nullptr)
    , m_symbol("600000")  // 默认股票代码：浦发银行
    , m_isActive(false)
    , m_dataFetchMode(REST_API)  // 默认使用REST API模式
{
    initialize();
}

onLineMarket_A::~onLineMarket_A()
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
    
    // 清理心跳定时器
    if (m_heartbeatTimer) {
        m_heartbeatTimer->stop();
        delete m_heartbeatTimer;
    }
}

void onLineMarket_A::initialize()
{
    // 初始化REST API相关资源
    m_networkManager = new QNetworkAccessManager(this);
    
    // 初始化定时器，默认1秒获取一次数据（秒级数据）
    m_dataFetchTimer = new QTimer(this);
    m_dataFetchTimer->setInterval(1000);
    connect(m_dataFetchTimer, &QTimer::timeout, this, &onLineMarket_A::fetchMarketData);
    
    // 构建API URL
    m_apiUrl = QUrl(API_BASE_URL + API_QUOTE_ENDPOINT);
    
    // 格式化股票代码
    m_formattedSymbol = formatStockCode(m_symbol);
    
    // 初始化WebSocket（但不连接）
    initializeWebSocket();
    
    // 初始化心跳定时器
    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(30000); // 30秒发送一次心跳
    connect(m_heartbeatTimer, &QTimer::timeout, this, &onLineMarket_A::sendHeartbeat);
}

void onLineMarket_A::initializeWebSocket()
{
    // 创建WebSocket对象
    if (!m_webSocket) {
        m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
        
        // 连接WebSocket信号
        connect(m_webSocket, &QWebSocket::connected, this, &onLineMarket_A::onWebSocketConnected);
        connect(m_webSocket, &QWebSocket::disconnected, this, &onLineMarket_A::onWebSocketDisconnected);
        connect(m_webSocket, &QWebSocket::textMessageReceived, this, &onLineMarket_A::onWebSocketTextMessageReceived);
        connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), 
                this, &onLineMarket_A::onWebSocketError);
    }
}

void onLineMarket_A::setDataFetchMode(DataFetchMode mode)
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

void onLineMarket_A::setSymbol(const QString &symbol)
{
    // 更新股票代码
    m_symbol = symbol;
    
    // 更新格式化后的股票代码
    m_formattedSymbol = formatStockCode(symbol);
    
    // 如果当前正在获取数据，需要重新订阅
    if (m_isActive) {
        if (m_dataFetchMode == WEBSOCKET && m_webSocketConnected) {
            // 先取消之前的订阅
            QJsonObject unsubObj;
            unsubObj["action"] = "unsub";
            unsubObj["symbol"] = m_formattedSymbol;
            QString unsubMessage = QJsonDocument(unsubObj).toJson(QJsonDocument::Compact);
            m_webSocket->sendTextMessage(unsubMessage);
            
            // 订阅新的股票
            subscribeToChannel();
        }
    }
}

void onLineMarket_A::setUpdateInterval(int milliseconds)
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

void onLineMarket_A::startFetchingData()
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

void onLineMarket_A::stopFetchingData()
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
                unsubObj["action"] = "unsub";
                unsubObj["symbol"] = m_formattedSymbol;
                QString unsubMessage = QJsonDocument(unsubObj).toJson(QJsonDocument::Compact);
                m_webSocket->sendTextMessage(unsubMessage);
                
                // 停止心跳
                m_heartbeatTimer->stop();
                
                // 关闭WebSocket连接
                m_webSocket->close();
            }
        }
    }
}

void onLineMarket_A::fetchMarketData()
{
    // 构建请求URL，添加查询参数
    QUrl requestUrl = m_apiUrl;
    QUrlQuery query;
    
    // 添加必要的参数
    query.addQueryItem("secid", m_formattedSymbol);
    query.addQueryItem("fields", "f43,f44,f45,f46,f47,f48,f49,f50,f51,f52,f57,f58,f60,f107,f168");
    query.addQueryItem("_", QString::number(QDateTime::currentMSecsSinceEpoch()));
    
    requestUrl.setQuery(query);
    
    // 创建请求
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Referer", "https://quote.eastmoney.com/");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    
    // 发送GET请求
    QNetworkReply *reply = m_networkManager->get(request);
    
    // 连接信号处理响应
    connect(reply, &QNetworkReply::finished, this, &onLineMarket_A::onNetworkReplyFinished);
}

void onLineMarket_A::onNetworkReplyFinished()
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

void onLineMarket_A::parseMarketData(const QByteArray &data)
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
    if (rootObj["rc"].toInt() != 0) {
        QString errorMsg = rootObj["msg"].toString();
        emit errorOccurred(tr("API错误: %1").arg(errorMsg));
        return;
    }
    
    // 获取数据对象
    QJsonObject dataObj = rootObj["data"].toObject();
    if (dataObj.isEmpty()) {
        emit errorOccurred(tr("没有返回数据"));
        return;
    }
    
    // 提取行情数据
    QString symbol = m_symbol;
    double lastPrice = dataObj["f43"].toDouble() / 100.0;  // 最新价
    double volume = dataObj["f47"].toDouble() / 100.0;     // 成交量（手）
    double turnover = dataObj["f48"].toDouble() / 10000.0; // 成交额（万元）
    double high = dataObj["f44"].toDouble() / 100.0;       // 最高
    double low = dataObj["f45"].toDouble() / 100.0;        // 最低
    double open = dataObj["f46"].toDouble() / 100.0;       // 开盘价
    double preClose = dataObj["f60"].toDouble() / 100.0;   // 昨收价
    
    // 更新缓存
    m_lastData["lastPrice"] = lastPrice;
    m_lastData["volume"] = volume;
    m_lastData["high"] = high;
    m_lastData["low"] = low;
    m_lastData["open"] = open;
    m_lastData["preClose"] = preClose;
    
    // 发出信号通知新数据
    emit newMarketData(symbol, lastPrice, volume, high, low, open, preClose);
}

// WebSocket相关方法实现
void onLineMarket_A::onWebSocketConnected()
{
    m_webSocketConnected = true;
    
    // 连接成功后，订阅行情频道
    if (m_isActive) {
        subscribeToChannel();
        
        // 启动心跳定时器
        m_heartbeatTimer->start();
    }
}

void onLineMarket_A::onWebSocketDisconnected()
{
    m_webSocketConnected = false;
    
    // 停止心跳定时器
    m_heartbeatTimer->stop();
    
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

void onLineMarket_A::onWebSocketTextMessageReceived(const QString &message)
{
    // 解析WebSocket消息
    parseWebSocketMessage(message);
}

void onLineMarket_A::onWebSocketError(QAbstractSocket::SocketError error)
{
    emit errorOccurred(tr("WebSocket错误: %1").arg(m_webSocket->errorString()));
    
    // 如果发生错误，标记为断开连接
    m_webSocketConnected = false;
}

void onLineMarket_A::subscribeToChannel()
{
    if (!m_webSocketConnected || !m_webSocket) {
        return;
    }
    
    // 构建订阅消息
    QJsonObject subObj;
    subObj["action"] = "sub";
    subObj["symbol"] = m_formattedSymbol;
    subObj["type"] = "ticker";  // 订阅逐笔成交数据
    
    // 转换为JSON字符串并发送
    QString subMessage = QJsonDocument(subObj).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(subMessage);
}

void onLineMarket_A::sendHeartbeat()
{
    if (m_webSocketConnected && m_webSocket) {
        // 发送心跳消息
        QJsonObject heartbeatObj;
        heartbeatObj["action"] = "heartbeat";
        heartbeatObj["ts"] = QDateTime::currentMSecsSinceEpoch();
        
        QString heartbeatMessage = QJsonDocument(heartbeatObj).toJson(QJsonDocument::Compact);
        m_webSocket->sendTextMessage(heartbeatMessage);
    }
}

void onLineMarket_A::parseWebSocketMessage(const QString &message)
{
    // 解析JSON消息
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(tr("WebSocket JSON解析错误: %1").arg(parseError.errorString()));
        return;
    }
    
    QJsonObject rootObj = jsonDoc.object();
    
    // 检查是否是心跳响应
    if (rootObj.contains("action") && rootObj["action"].toString() == "heartbeat") {
        // 心跳响应，不需要处理
        return;
    }
    
    // 检查是否是订阅确认
    if (rootObj.contains("action") && rootObj["action"].toString() == "sub") {
        bool success = rootObj["result"].toBool();
        if (!success) {
            QString errorMsg = rootObj["message"].toString();
            emit errorOccurred(tr("订阅失败: %1").arg(errorMsg));
        }
        return;
    }
    
    // 检查是否是数据消息
    if (rootObj.contains("data")) {
        QJsonObject dataObj = rootObj["data"].toObject();
        if (dataObj.isEmpty()) {
            return;
        }
        
        // 提取行情数据
        QString symbol = m_symbol;
        double lastPrice = dataObj.contains("price") ? dataObj["price"].toDouble() : m_lastData.value("lastPrice", 0.0);
        double volume = dataObj.contains("volume") ? dataObj["volume"].toDouble() : m_lastData.value("volume", 0.0);
        
        // 其他数据可能不在每个tick中提供，使用缓存的值
        double high = dataObj.contains("high") ? dataObj["high"].toDouble() : m_lastData.value("high", 0.0);
        double low = dataObj.contains("low") ? dataObj["low"].toDouble() : m_lastData.value("low", 0.0);
        double open = dataObj.contains("open") ? dataObj["open"].toDouble() : m_lastData.value("open", 0.0);
        double preClose = dataObj.contains("preClose") ? dataObj["preClose"].toDouble() : m_lastData.value("preClose", 0.0);
        
        // 更新缓存
        if (dataObj.contains("price")) m_lastData["lastPrice"] = lastPrice;
        if (dataObj.contains("volume")) m_lastData["volume"] = volume;
        if (dataObj.contains("high")) m_lastData["high"] = high;
        if (dataObj.contains("low")) m_lastData["low"] = low;
        if (dataObj.contains("open")) m_lastData["open"] = open;
        if (dataObj.contains("preClose")) m_lastData["preClose"] = preClose;
        
        // 发出信号通知新数据
        emit newMarketData(symbol, lastPrice, volume, high, low, open, preClose);
    }
}

QString onLineMarket_A::getMarketType(const QString &symbol)
{
    // 判断股票代码所属市场
    if (symbol.startsWith("6")) {
        return "1";  // 上海市场
    } else if (symbol.startsWith("0") || symbol.startsWith("3")) {
        return "0";  // 深圳市场
    } else if (symbol.startsWith("4") || symbol.startsWith("8")) {
        return "0";  // 北交所，暂时归类为深圳
    } else {
        // 默认返回上海市场
        return "1";
    }
}

QString onLineMarket_A::formatStockCode(const QString &symbol)
{
    // 格式化股票代码，添加市场前缀
    QString marketType = getMarketType(symbol);
    return marketType + "." + symbol;
}