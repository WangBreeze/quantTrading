#ifndef BACKTESTENGINE_H
#define BACKTESTENGINE_H

#include "Strategy.h"
#include "HistoryDataManager.h"
#include "../online/AppData.h"
#include <QObject>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <memory>

// 回测引擎类
class BacktestEngine : public QObject
{
    Q_OBJECT
public:
    explicit BacktestEngine(QObject *parent = nullptr);
    ~BacktestEngine();

    // 设置回测参数
    void setBacktestParams(const AppData::BacktestParams &params);
    AppData::BacktestParams getBacktestParams() const;

    // 添加策略
    void addStrategy(std::shared_ptr<Strategy> strategy);

    // 运行回测
    bool runBacktest();

    // 获取回测结果
    AppData::BacktestResult getBacktestResult() const;

    // 设置数据管理器
    void setDataManager(std::shared_ptr<HistoryDataManager> dataManager);

signals:
    void progressUpdated(int progress);
    void logMessage(const QString &message, int level = 0);
    void backtestFinished();

private:
    // 初始化回测
    bool initialize();

    // 执行回测
    void execute();

    // 清理回测
    void cleanup();

    // 处理订单
    void processOrder(const AppData::Order &order);

    // 处理取消订单
    void processCancelOrder(const QString &orderId);

    // 模拟撮合
    void matchOrders(const AppData::MarketData &data);

    // 更新账户
    void updateAccount(const AppData::Trade &trade);

    // 计算回测指标
    void calculateMetrics();

    AppData::BacktestParams m_params; // 回测参数
    AppData::BacktestResult m_result;  // 回测结果
    QVector<std::shared_ptr<Strategy>> m_strategies; // 策略列表
    std::shared_ptr<HistoryDataManager> m_dataManager; // 数据管理器

    QVector<AppData::MarketData> m_marketData; // 市场数据
    QMap<QString, AppData::Order> m_activeOrders; // 活动订单
    QVector<AppData::Trade> m_trades; // 成交记录
    AppData::Account m_account; // 账户信息
    QDateTime m_currentTime; // 当前回测时间
};

#endif // BACKTESTENGINE_H