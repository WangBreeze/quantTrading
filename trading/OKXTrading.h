#ifndef OKXTRADING_H
#define OKXTRADING_H

#include "TradingEngine.h"
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QTimer>

class OKXTrading : public TradingEngine
{
    Q_OBJECT

public:
    explicit OKXTrading(const QString &apiKey, const QString &secretKey, QObject *parent = nullptr);
    ~OKXTrading();

    void connectToExchange() override;
    void disconnectFromExchange() override;
    void placeOrder(const AppData::Order &order) override;
    void cancelOrder(const QString &orderId) override;
    void queryAccount() override;

private slots:
    void onWsConnected();
    void onWsDisconnected();
    void onWsMessageReceived(const QString &message);
    void sendPing();

private:
    QString generateSignature(const QString &timestamp, const QString &method, const QString &path);
    void handleOrderResponse(QNetworkReply *reply, const AppData::Order &order);
    void handleCancelResponse(QNetworkReply *reply, const QString &orderId);
    void handleAccountResponse(QNetworkReply *reply);
    void handleWsAccountUpdate(const QJsonArray &data);
    void handleWsOrderUpdate(const QJsonArray &data);

    QString m_apiKey;
    QString m_secretKey;
    QNetworkAccessManager *m_restManager;
    QWebSocket *m_wsSocket;
    QTimer *m_pingTimer;
};

#endif // OKXTRADING_H