#ifndef EASTMONEYTRADING_H
#define EASTMONEYTRADING_H

#include "TradingEngine.h"
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QTimer>

class EastMoneyTrading : public TradingEngine
{
    Q_OBJECT

public:
    explicit EastMoneyTrading(const QString &accountId, const QString &token, QObject *parent = nullptr);
    ~EastMoneyTrading();

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
    void handleOrderResponse(QNetworkReply *reply, const AppData::Order &order);
    void handleCancelResponse(QNetworkReply *reply, const QString &orderId);
    void handleAccountResponse(QNetworkReply *reply);
    void handleWsAccountUpdate(const QJsonObject &data);
    void handleWsOrderUpdate(const QJsonObject &data);

    QString m_accountId;
    QString m_token;
    QNetworkAccessManager *m_restManager;
    QWebSocket *m_wsSocket;
    QTimer *m_pingTimer;
};

#endif // EASTMONEYTRADING_H