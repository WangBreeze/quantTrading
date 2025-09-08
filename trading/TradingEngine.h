#ifndef TRADINGENGINE_H
#define TRADINGENGINE_H

#include <QObject>
#include <QMap>
#include <memory>
#include "../online/AppData.h"
#include "../history/Strategy.h"

class TradingEngine : public QObject
{
    Q_OBJECT
public:
    explicit TradingEngine(QObject *parent = nullptr);
    ~TradingEngine();

    // 初始化交易引擎
    bool initialize();

    // 添加交易策略
    void addStrategy(std::shared_ptr<Strategy> strategy);

    // 设置交易账户
    void setAccount(const AppData::Account &account);

    // 开始交易
    bool startTrading();

    // 停止交易
    void stopTrading();

    // 获取当前持仓
    QMap<QString, AppData::Position> getPositions() const;

signals:
    void orderSent(const AppData::Order &order);
    void tradeExecuted(const AppData::Trade &trade);
    void errorOccurred(const QString &message);
    void statusUpdated(const QString &status);

private slots:
    void onMarketData(const AppData::MarketData &data);
    void onStrategySignal(const AppData::Signal &signal);

private:
    // 执行订单
    void executeOrder(const AppData::Order &order);

    // 取消订单
    void cancelOrder(const QString &orderId);

    QMap<QString, AppData::Order> m_activeOrders;
    QMap<QString, AppData::Position> m_positions;
    AppData::Account m_account;
    QVector<std::shared_ptr<Strategy>> m_strategies;
    bool m_isTrading;
};

#endif // TRADINGENGINE_H