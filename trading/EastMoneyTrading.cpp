#include "EastMoneyTrading.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QTimer>
#include <QWebSocket>

EastMoneyTrading::EastMoneyTrading(const QString &accountId, const QString &token, QObject *parent)
    : TradingEngine(parent), m_accountId(accountId), m_token(token)
{
    m_restManager = new QNetworkAccessManager(this);
    m_wsSocket = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);
    
    connect(m_wsSocket, &QWebSocket::connected, this, &EastMoneyTrading::onWsConnected);
    connect(m_wsSocket, &QWebSocket::disconnected, this, &EastMoneyTrading::onWsDisconnected);
    connect(m_wsSocket, &QWebSocket::textMessageReceived, this, &EastMoneyTrading::onWsMessageReceived);
    
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &EastMoneyTrading::sendPing);
    m_pingTimer->start(30000); // 30秒心跳
}

void EastMoneyTrading::connectToExchange()
{
    QUrl wsUrl("wss://trade.eastmoney.com/websocket");
    QNetworkRequest request(wsUrl);
    request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    m_wsSocket->open(request);
}

void EastMoneyTrading::disconnectFromExchange()
{
    m_wsSocket->close();
}

void EastMoneyTrading::placeOrder(const AppData::Order &order)
{
    QUrl url("https://trade.eastmoney.com/api/order/place");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    
    QJsonObject orderJson;
    orderJson["accountId"] = m_accountId;
    orderJson["symbol"] = order.symbol;
    orderJson["direction"] = (order.direction == AppData::Long) ? "buy" : "sell";
    orderJson["priceType"] = "limit";
    orderJson["price"] = order.price;
    orderJson["quantity"] = order.quantity;
    orderJson["clientOrderId"] = order.orderId;
    
    QNetworkReply *reply = m_restManager->post(request, QJsonDocument(orderJson).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, order]() {
        handleOrderResponse(reply, order);
    });
}

void EastMoneyTrading::cancelOrder(const QString &orderId)
{
    QUrl url("https://trade.eastmoney.com/api/order/cancel");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    
    QJsonObject cancelJson;
    cancelJson["accountId"] = m_accountId;
    cancelJson["orderId"] = orderId;
    
    QNetworkReply *reply = m_restManager->post(request, QJsonDocument(cancelJson).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, orderId]() {
        handleCancelResponse(reply, orderId);
    });
}

void EastMoneyTrading::queryAccount()
{
    QUrl url("https://trade.eastmoney.com/api/account/query");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    
    QJsonObject queryJson;
    queryJson["accountId"] = m_accountId;
    
    QNetworkReply *reply = m_restManager->post(request, QJsonDocument(queryJson).toJson());
    connect(reply, &QNetworkReply::finished, this, &EastMoneyTrading::handleAccountResponse);
}

void EastMoneyTrading::onWsConnected()
{
    // 订阅账户和订单频道
    QJsonObject subscribeMsg;
    subscribeMsg["type"] = "subscribe";
    subscribeMsg["channels"] = QJsonArray({
        "account:" + m_accountId,
        "orders:" + m_accountId
    });
    
    m_wsSocket->sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
    emit statusUpdated("WebSocket connected to EastMoney");
}

void EastMoneyTrading::onWsDisconnected()
{
    emit statusUpdated("WebSocket disconnected from EastMoney");
}

void EastMoneyTrading::onWsMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "login") {
        if (obj["success"].toBool()) {
            emit statusUpdated("EastMoney WebSocket login successful");
        } else {
            emit statusUpdated("EastMoney WebSocket login failed: " + obj["error"].toString());
        }
    } else if (type == "subscribe") {
        // 订阅成功
    } else if (obj.contains("channel")) {
        // 处理数据推送
        QString channel = obj["channel"].toString();
        
        if (channel.startsWith("account:")) {
            handleWsAccountUpdate(obj["data"].toObject());
        } else if (channel.startsWith("orders:")) {
            handleWsOrderUpdate(obj["data"].toObject());
        }
    }
}

void EastMoneyTrading::sendPing()
{
    if (m_wsSocket->state() == QAbstractSocket::ConnectedState) {
        m_wsSocket->ping();
    }
}

void EastMoneyTrading::handleOrderResponse(QNetworkReply *reply, const AppData::Order &order)
{
    // 处理订单响应
    // ...
    reply->deleteLater();
}

void EastMoneyTrading::handleCancelResponse(QNetworkReply *reply, const QString &orderId)
{
    // 处理取消订单响应
    // ...
    reply->deleteLater();
}

void EastMoneyTrading::handleAccountResponse(QNetworkReply *reply)
{
    // 处理账户查询响应
    // ...
    reply->deleteLater();
}

void EastMoneyTrading::handleWsAccountUpdate(const QJsonObject &data)
{
    // 处理WebSocket账户更新
    // ...
}

void EastMoneyTrading::handleWsOrderUpdate(const QJsonObject &data)
{
    // 处理WebSocket订单更新
    // ...
}