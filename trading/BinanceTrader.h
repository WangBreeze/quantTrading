#ifndef BINANCETRADER_H
#define BINANCETRADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QtWebSockets/QWebSocket>
#include "../AppData.h"
#include "TradingEngine.h"

class BinanceTrader : public TradingEngine
{
    Q_OBJECT
public:
    explicit BinanceTrader(const QString &apiKey, const QString &secretKey, QObject *parent = nullptr);
    
    // 查询账户信息
    void queryAccount() override;
    
    // 查询持仓
    bool queryPositions();
    
    // 下单
    QString placeOrder(const QString &symbol, AppData::Direction direction, 
                      double price, double quantity, AppData::OrderType type);
    
    // 撤单
    bool cancelOrder(const QString &orderId, const QString &symbol);
    
    // 查询订单状态
    bool queryOrderStatus(const QString &orderId, const QString &symbol);
    
    // 查询成交记录
    bool queryTrades();
    AppData::OrderStatus parseOrderStatus(const QString &status);
public slots:
    void handleOrderResponse(const AppData::Order &order);
    void handleCancelResponse(const QString &orderId);
    void handleAccountResponse();
    void handleWsAccountUpdate(const QJsonObject &data);
    void handleWsOrderUpdate(const QJsonObject &data);
signals:
    void accountUpdated(const AppData::Account &account);
    void positionUpdated(const QMap<QString, AppData::Position> &positions);
    void orderUpdated(const AppData::Order &order);
    void tradeExecuted(const AppData::Trade &trade);
    void orderCanceled(const QString &message);
    void errorOccurred(const QString &message);
private slots:
    void onWsConnected();
    void onWsDisconnected();
    void onWsMessageReceived(const QString &message);
    void sendPing();
private:
    // 生成签名
    QString generateSignature(const QString &queryString);

    
    QNetworkAccessManager m_networkManager;
    QString m_apiKey;
    QString m_secretKey;
    AppData::Account m_account;
    QMap<QString, AppData::Position> m_positions;
    QString m_accountId;
    QString m_token;
    QNetworkAccessManager *m_restManager;
    QWebSocket *m_wsSocket;
    QTimer *m_pingTimer;

    // TradingEngine interface
public:
    void connectToExchange() override;
    void disconnectFromExchange() override;
    void placeOrder(const AppData::Order &order) override;
    void cancelOrder(const QString &orderId) override;
};

#endif // BINANCETRADER_H
