#include "OKXTrading.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QTimer>
#include <QtWebSockets/QWebSocket>

OKXTrading::OKXTrading(const QString &apiKey, const QString &secretKey, QObject *parent)
    : TradingEngine(parent), m_apiKey(apiKey), m_secretKey(secretKey)
{
    m_restManager = new QNetworkAccessManager(this);
    m_wsSocket = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);
    
    connect(m_wsSocket, &QWebSocket::connected, this, &OKXTrading::onWsConnected);
    connect(m_wsSocket, &QWebSocket::disconnected, this, &OKXTrading::onWsDisconnected);
    connect(m_wsSocket, &QWebSocket::textMessageReceived, this, &OKXTrading::onWsMessageReceived);
    
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &OKXTrading::sendPing);
    m_pingTimer->start(30000); // 30秒心跳
}

void OKXTrading::connectToExchange()
{
    QUrl wsUrl("wss://ws.okx.com:8443/ws/v5/private");
    m_wsSocket->open(wsUrl);
}

void OKXTrading::disconnectFromExchange()
{
    m_wsSocket->close();
}

void OKXTrading::placeOrder(const AppData::Order &order)
{
    QUrl url("https://www.okx.com/api/v5/trade/order");
    QNetworkRequest request(url);
    
    // 设置请求头
    QString timestamp = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzzZ");
    QString sign = generateSignature(timestamp, "POST", "/api/v5/trade/order");
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("OK-ACCESS-KEY", m_apiKey.toUtf8());
    request.setRawHeader("OK-ACCESS-SIGN", sign.toUtf8());
    request.setRawHeader("OK-ACCESS-TIMESTAMP", timestamp.toUtf8());
    request.setRawHeader("OK-ACCESS-PASSPHRASE", m_secretKey.toUtf8());
    
    // 构建订单JSON
    QJsonObject orderJson;
    orderJson["instId"] = order.symbol;
    orderJson["tdMode"] = "cash";
    orderJson["side"] = (order.direction == AppData::Long) ? "buy" : "sell";
    orderJson["ordType"] = "limit";
    orderJson["px"] = QString::number(order.price, 'f', 8);
    orderJson["sz"] = QString::number(order.quantity, 'f', 8);
    
    QNetworkReply *reply = m_restManager->post(request, QJsonDocument(orderJson).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, order]() {
        handleOrderResponse( order);
    });
}

void OKXTrading::cancelOrder(const QString &orderId)
{
    QUrl url("https://www.okx.com/api/v5/trade/cancel-order");
    QNetworkRequest request(url);
    
    QString timestamp = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzzZ");
    QString sign = generateSignature(timestamp, "POST", "/api/v5/trade/cancel-order");
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("OK-ACCESS-KEY", m_apiKey.toUtf8());
    request.setRawHeader("OK-ACCESS-SIGN", sign.toUtf8());
    request.setRawHeader("OK-ACCESS-TIMESTAMP", timestamp.toUtf8());
    request.setRawHeader("OK-ACCESS-PASSPHRASE", m_secretKey.toUtf8());
    
    QJsonObject cancelJson;
    cancelJson["ordId"] = orderId;
    
    QNetworkReply *reply = m_restManager->post(request, QJsonDocument(cancelJson).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, orderId]() {
        handleCancelResponse( orderId);
    });
}

void OKXTrading::queryAccount()
{
    QUrl url("https://www.okx.com/api/v5/account/balance");
    QNetworkRequest request(url);
    
    QString timestamp = QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzzZ");
    QString sign = generateSignature(timestamp, "GET", "/api/v5/account/balance");
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("OK-ACCESS-KEY", m_apiKey.toUtf8());
    request.setRawHeader("OK-ACCESS-SIGN", sign.toUtf8());
    request.setRawHeader("OK-ACCESS-TIMESTAMP", timestamp.toUtf8());
    request.setRawHeader("OK-ACCESS-PASSPHRASE", m_secretKey.toUtf8());
    
    QNetworkReply *reply = m_restManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OKXTrading::handleAccountResponse);
}

QString OKXTrading::generateSignature(const QString &timestamp, const QString &method, const QString &path)
{
    QString message = timestamp + method + path;
    return QCryptographicHash::hash(message.toUtf8(), QCryptographicHash::Sha256).toHex();
}

void OKXTrading::onWsConnected()
{
    // 订阅账户和订单频道
    QJsonObject subscribeMsg;
    subscribeMsg["op"] = "subscribe";
    subscribeMsg["args"] = QJsonArray({
        QJsonObject{{"channel", "account"}, {"ccy", "USDT"}},
        QJsonObject{{"channel", "orders"}, {"instType", "SPOT"}}
    });
    
    m_wsSocket->sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
    emit statusUpdated("WebSocket connected to OKX");
}

void OKXTrading::onWsDisconnected()
{
    emit statusUpdated("WebSocket disconnected from OKX");
}

void OKXTrading::onWsMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString event = obj["event"].toString();
    
    if (event == "login") {
        if (obj["code"].toString() == "0") {
            emit statusUpdated("OKX WebSocket login successful");
        } else {
            emit statusUpdated("OKX WebSocket login failed: " + obj["msg"].toString());
        }
    } else if (event == "subscribe") {
        // 订阅成功
    } else if (obj.contains("arg")) {
        // 处理数据推送
        QJsonObject arg = obj["arg"].toObject();
        QString channel = arg["channel"].toString();
        
        if (channel == "account") {
            handleWsAccountUpdate(obj["data"].toArray());
        } else if (channel == "orders") {
            handleWsOrderUpdate(obj["data"].toArray());
        }
    }
}

void OKXTrading::sendPing()
{
    if (m_wsSocket->state() == QAbstractSocket::ConnectedState) {
        m_wsSocket->ping();
    }
}

void OKXTrading::handleOrderResponse( const AppData::Order &order)
{
    // 处理订单响应
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    // ...
    reply->deleteLater();
}

void OKXTrading::handleCancelResponse( const QString &orderId)
{
    // 处理取消订单响应
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    // ...
    reply->deleteLater();
}

void OKXTrading::handleAccountResponse()
{
    // 处理账户查询响应
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    // ...
    reply->deleteLater();
}

void OKXTrading::handleWsAccountUpdate(const QJsonArray &data)
{
    // 处理WebSocket账户更新
    // ...
}

void OKXTrading::handleWsOrderUpdate(const QJsonArray &data)
{
    // 处理WebSocket订单更新
    // ...
}
