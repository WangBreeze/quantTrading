#include "BinanceTrader.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QTimer>
#include <QtWebSockets/QWebSocket>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonArray>

BinanceTrader::BinanceTrader(const QString &apiKey, const QString &secretKey, QObject *parent)
    : TradingEngine(parent), m_apiKey(apiKey), m_secretKey(secretKey)
{
    m_restManager = new QNetworkAccessManager(this);
    m_wsSocket = new QWebSocket("", QWebSocketProtocol::VersionLatest, this);
    
    connect(m_wsSocket, &QWebSocket::connected, this, &BinanceTrader::onWsConnected);
    connect(m_wsSocket, &QWebSocket::disconnected, this, &BinanceTrader::onWsDisconnected);
    connect(m_wsSocket, &QWebSocket::textMessageReceived, this, &BinanceTrader::onWsMessageReceived);
    
    m_pingTimer = new QTimer(this);
    connect(m_pingTimer, &QTimer::timeout, this, &BinanceTrader::sendPing);
    m_pingTimer->start(30000); // 30秒心跳
}

void BinanceTrader::connectToExchange()
{
    QUrl wsUrl("wss://stream.binance.com:9443/ws");
    m_wsSocket->open(wsUrl);
}

void BinanceTrader::disconnectFromExchange()
{
    m_wsSocket->close();
}

void BinanceTrader::placeOrder(const AppData::Order &order)
{
    QUrl url("https://api.binance.com/api/v3/order");
    QUrlQuery query;
    
    query.addQueryItem("symbol", order.symbol);
    query.addQueryItem("side", order.direction == AppData::Long ? "BUY" : "SELL");
    query.addQueryItem("type", "LIMIT");
    query.addQueryItem("timeInForce", "GTC");
    query.addQueryItem("quantity", QString::number(order.quantity, 'f', 8));
    query.addQueryItem("price", QString::number(order.price, 'f', 8));
    query.addQueryItem("recvWindow", "5000");
    query.addQueryItem("timestamp", QString::number(QDateTime::currentMSecsSinceEpoch()));
    
    QString signature = generateSignature(query.toString());
    query.addQueryItem("signature", signature);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", m_apiKey.toUtf8());
    
    QNetworkReply *reply = m_restManager->post(request, query.toString().toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, order]() {
        handleOrderResponse( order);
    });
}

void BinanceTrader::cancelOrder(const QString &orderId)
{
    QUrl url("https://api.binance.com/api/v3/order");
    QUrlQuery query;
    
    query.addQueryItem("orderId", orderId);
    query.addQueryItem("timestamp", QString::number(QDateTime::currentMSecsSinceEpoch()));
    
    QString signature = generateSignature(query.toString());
    query.addQueryItem("signature", signature);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("X-MBX-APIKEY", m_apiKey.toUtf8());
    
    QNetworkReply *reply = m_restManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, orderId]() {
        handleCancelResponse( orderId);
    });
}

void BinanceTrader::queryAccount()
{
    QUrl url("https://api.binance.com/api/v3/account");
    QUrlQuery query;
    
    query.addQueryItem("timestamp", QString::number(QDateTime::currentMSecsSinceEpoch()));
    query.addQueryItem("recvWindow", "5000");
    
    QString signature = generateSignature(query.toString());
    query.addQueryItem("signature", signature);
    
    QNetworkRequest request(url);
    request.setRawHeader("X-MBX-APIKEY", m_apiKey.toUtf8());
    
    QNetworkReply *reply = m_restManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &BinanceTrader::handleAccountResponse);
}

QString BinanceTrader::generateSignature(const QString &data)
{
    return QMessageAuthenticationCode::hash(
        data.toUtf8(), 
        m_secretKey.toUtf8(), 
        QCryptographicHash::Sha256
    ).toHex();
}

void BinanceTrader::onWsConnected()
{
    // 订阅账户更新
    QJsonObject subscribeMsg;
    subscribeMsg["method"] = "SUBSCRIBE";
    subscribeMsg["params"] = QJsonArray({
        QString("!userData@arr")
    });
    subscribeMsg["id"] = QDateTime::currentMSecsSinceEpoch();
    
    m_wsSocket->sendTextMessage(QJsonDocument(subscribeMsg).toJson(QJsonDocument::Compact));
    emit statusUpdated("WebSocket connected to Binance");
}

void BinanceTrader::onWsDisconnected()
{
    emit statusUpdated("WebSocket disconnected from Binance");
}

void BinanceTrader::onWsMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    
    QJsonObject obj = doc.object();
    QString eventType = obj["e"].toString();
    
    if (eventType == "executionReport") {
        handleWsOrderUpdate(obj);
    } else if (eventType == "outboundAccountPosition") {
        handleWsAccountUpdate(obj);
    }
}

void BinanceTrader::sendPing()
{
    if (m_wsSocket->state() == QAbstractSocket::ConnectedState) {
        m_wsSocket->ping();
    }
}

void BinanceTrader::handleOrderResponse(const AppData::Order &order)
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid order response format");
        reply->deleteLater();
        return;
    }

    QJsonObject response = doc.object();
    if (response.contains("code")) {
        emit errorOccurred(response["msg"].toString());
        reply->deleteLater();
        return;
    }

    // 更新订单状态
    AppData::Order updatedOrder = order;
    updatedOrder.status = AppData::OrderStatus::Completed;
    updatedOrder.filledQuantity = response["executedQty"].toString().toDouble();
    updatedOrder.avgFillPrice = response["price"].toString().toDouble();
    updatedOrder.updateTime = QDateTime::fromMSecsSinceEpoch(response["updateTime"].toVariant().toLongLong());

    emit orderUpdated(updatedOrder);
    reply->deleteLater();
}

void BinanceTrader::handleCancelResponse( const QString &orderId)
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid cancel response format");
        reply->deleteLater();
        return;
    }

    QJsonObject response = doc.object();
    if (response.contains("code")) {
        emit errorOccurred(response["msg"].toString());
        reply->deleteLater();
        return;
    }

    emit orderCanceled(orderId);
    reply->deleteLater();
}

void BinanceTrader::handleAccountResponse()
{
    QNetworkReply *reply = dynamic_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit errorOccurred("Invalid account response format");
        reply->deleteLater();
        return;
    }

    QJsonObject response = doc.object();
    if (response.contains("code")) {
        emit errorOccurred(response["msg"].toString());
        reply->deleteLater();
        return;
    }

    // 更新账户信息
    AppData::Account account;
    account.balance = 0;
    account.available = 0;
    account.realizedPnL = 0;
    account.unrealizedPnL = 0;

    QJsonArray balances = response["balances"].toArray();
    for (const QJsonValue &balance : balances) {
        QJsonObject balanceObj = balance.toObject();
        if (balanceObj["asset"].toString() == "USDT") {
            account.balance = balanceObj["free"].toString().toDouble();
            account.available = balanceObj["free"].toString().toDouble();
            break;
        }
    }

    setAccount(account);
    emit accountUpdated(account);
    reply->deleteLater();
}

void BinanceTrader::handleWsOrderUpdate(const QJsonObject &data)
{
    AppData::Order order;
    order.orderId = data["c"].toString();
    order.symbol = data["s"].toString();
    order.direction = data["S"].toString() == "BUY" ? AppData::Long : AppData::Short;
    order.price = data["p"].toString().toDouble();
    order.quantity = data["q"].toString().toDouble();
    order.filledQuantity = data["z"].toString().toDouble();
    order.status = parseOrderStatus(data["X"].toString());
    order.updateTime = QDateTime::fromMSecsSinceEpoch(data["E"].toVariant().toLongLong());

    emit orderUpdated(order);
}

void BinanceTrader::handleWsAccountUpdate(const QJsonObject &data)
{
    AppData::Account account;
    account.balance = 0;
    account.available = 0;
    account.realizedPnL = 0;
    account.unrealizedPnL = 0;

    QJsonArray balances = data["B"].toArray();
    for (const QJsonValue &balance : balances) {
        QJsonObject balanceObj = balance.toObject();
        if (balanceObj["a"].toString() == "USDT") {
            account.balance = balanceObj["f"].toString().toDouble();
            account.available = balanceObj["f"].toString().toDouble();
            break;
        }
    }

    setAccount(account);
    emit accountUpdated(account);
}

AppData::OrderStatus BinanceTrader::parseOrderStatus(const QString &status)
{
    if (status == "NEW") return AppData::Created;
    if (status == "PARTIALLY_FILLED") return AppData::Partial;
    if (status == "FILLED") return AppData::Completed;
    if (status == "CANCELED") return AppData::Canceled;
    if (status == "REJECTED") return AppData::Rejected;
    if (status == "EXPIRED") return AppData::Expired;
    return AppData::UnknownStatus;
}
