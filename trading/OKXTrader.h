#ifndef OKXTRADER_H
#define OKXTRADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include "../online/AppData.h"

class OKXTrader : public QObject
{
    Q_OBJECT
public:
    explicit OKXTrader(const QString &apiKey, const QString &secretKey, 
                      const QString &passphrase, QObject *parent = nullptr);
    
    // 设置交易账户
    bool setAccount(const QString &accountId);
    
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
    QString generateSignature(const QString &method, const QString &path, 
                             const QString &body = "");
    
    QNetworkAccessManager m_networkManager;
    QString m_apiKey;
    QString m_secretKey;
    QString m_passphrase;
    QString m_accountId;
    AppData::Account m_account;
    QMap<QString, AppData::Position> m_positions;
};

#endif // OKXTRADER_H