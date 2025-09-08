#ifndef STRATEGY_H
#define STRATEGY_H

#include "../online/AppData.h"
#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>
#include <QDateTime>
#include <functional>

// 策略基类，所有交易策略都应该继承自这个类
class Strategy : public QObject
{
    Q_OBJECT
public:
    explicit Strategy(QObject *parent = nullptr);
    virtual ~Strategy();

    // 策略初始化方法，在回测或实盘开始前调用
    virtual void initialize() = 0;

    // 策略清理方法，在回测或实盘结束后调用
    virtual void cleanup() = 0;

    // 处理Tick数据的方法
    virtual void onTick(const AppData::MarketData &data) = 0;

    // 处理K线数据的方法
    virtual void onBar(const AppData::Candle &data) = 0;

    // 处理订单状态变化的方法
    virtual void onOrder(const AppData::Order &order) = 0;

    // 处理成交回报的方法
    virtual void onTrade(const AppData::Trade &trade) = 0;

    // 设置策略参数
    void setParameter(const QString &name, const QVariant &value);
    QVariant getParameter(const QString &name) const;

    // 获取策略名称
    QString getName() const;
    void setName(const QString &name);

    // 获取策略描述
    QString getDescription() const;
    void setDescription(const QString &description);

    // 获取策略作者
    QString getAuthor() const;
    void setAuthor(const QString &author);

    // 获取策略版本
    QString getVersion() const;
    void setVersion(const QString &version);

    // 设置回测模式
    void setBacktestMode(bool isBacktest);
    bool isBacktestMode() const;

    // 设置回测引擎回调函数
    void setOrderCallback(std::function<void(const AppData::Order&)> callback);
    void setCancelOrderCallback(std::function<void(const QString&)> callback);

    // 交易接口
    AppData::Order buyMarket(const QString &symbol, double quantity);
    AppData::Order sellMarket(const QString &symbol, double quantity);
    AppData::Order buyLimit(const QString &symbol, double price, double quantity);
    AppData::Order sellLimit(const QString &symbol, double price, double quantity);
    AppData::Order buyStop(const QString &symbol, double stopPrice, double quantity);
    AppData::Order sellStop(const QString &symbol, double stopPrice, double quantity);
    bool cancelOrder(const QString &orderId);

    // 查询接口
    QVector<AppData::Position> getPositions() const;
    AppData::Position getPosition(const QString &symbol) const;
    QVector<AppData::Order> getActiveOrders() const;
    AppData::Order getOrder(const QString &orderId) const;
    AppData::Account getAccount() const;

    // 设置账户
    void setAccount(const AppData::Account &account);

    // 添加持仓
    void addPosition(const AppData::Position &position);

    // 添加订单
    void addOrder(const AppData::Order &order);

    // 更新订单状态
    void updateOrder(const AppData::Order &order);

signals:
    // 策略信号
    void signalGenerated(const AppData::Signal &signal);
    void logMessage(const QString &message, int level = 0);

protected:
    QString m_name;                  // 策略名称
    QString m_description;           // 策略描述
    QString m_author;                // 策略作者
    QString m_version;               // 策略版本
    bool m_isBacktest;               // 是否为回测模式
    QMap<QString, QVariant> m_parameters; // 策略参数
    AppData::Account m_account;      // 账户信息
    QMap<QString, AppData::Position> m_positions; // 持仓信息
    QMap<QString, AppData::Order> m_orders;       // 订单信息

    // 回测引擎回调函数
    std::function<void(const AppData::Order&)> m_orderCallback;
    std::function<void(const QString&)> m_cancelOrderCallback;
};

#endif // STRATEGY_H