#ifndef BINANCETRADER_H
#define BINANCETRADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include "../online/AppData.h"

class BinanceTrader : public QObject
{
    Q_OBJECT
public:
    explicit BinanceTrader(const QString &apiKey, const QString &secretKey, QObject *parent = nullptr);
    
    // 查询账户信息
    bool queryAccount();
    
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
    
signals:
    void accountUpdated(const AppData::Account &account);
    void positionUpdated(const QMap<QString, AppData::Position> &positions);
    void orderStatusUpdated(const AppData::Order &order);
    void tradeExecuted(const AppData::Trade &trade);
    void errorOccurred(const QString &message);

private:
    // 生成签名
    QString generateSignature(const QString &queryString);
    
    QNetworkAccessManager m_networkManager;
    QString m_apiKey;
    QString m_secretKey;
    AppData::Account m_account;
    QMap<QString, AppData::Position> m_positions;
};

#endif // BINANCETRADER_H