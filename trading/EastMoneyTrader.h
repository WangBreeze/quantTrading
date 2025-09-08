#ifndef EASTMONEYTRADER_H
#define EASTMONEYTRADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include "../online/AppData.h"

class EastMoneyTrader : public QObject
{
    Q_OBJECT
public:
    explicit EastMoneyTrader(QObject *parent = nullptr);
    
    // 登录交易账户
    bool login(const QString &username, const QString &password);
    
    // 查询账户信息
    bool queryAccount();
    
    // 查询持仓
    bool queryPositions();
    
    // 下单
    QString placeOrder(const QString &symbol, AppData::Direction direction, 
                      double price, int quantity, AppData::OrderType type);
    
    // 撤单
    bool cancelOrder(const QString &orderId);
    
    // 查询订单状态
    bool queryOrderStatus(const QString &orderId);
    
    // 查询成交记录
    bool queryTrades();
    
signals:
    void accountUpdated(const AppData::Account &account);
    void positionUpdated(const QMap<QString, AppData::Position> &positions);
    void orderStatusUpdated(const AppData::Order &order);
    void tradeExecuted(const AppData::Trade &trade);
    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager m_networkManager;
    QString m_sessionId;
    AppData::Account m_account;
    QMap<QString, AppData::Position> m_positions;
};

#endif // EASTMONEYTRADER_H