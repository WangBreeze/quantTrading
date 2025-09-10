#include "BacktestEngine.h"
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <algorithm>
#include <cmath>

BacktestEngine::BacktestEngine(QObject *parent)
    : QObject(parent)
{
}

BacktestEngine::~BacktestEngine()
{
}

void BacktestEngine::setBacktestParams(const AppData::BacktestParams &params)
{
    m_params = params;
}

AppData::BacktestParams BacktestEngine::getBacktestParams() const
{
    return m_params;
}

void BacktestEngine::addStrategy(std::shared_ptr<Strategy> strategy)
{
    if (strategy) {
        strategy->setBacktestMode(true);
        strategy->setOrderCallback([this](const AppData::Order &order) {
            processOrder(order);
        });
        strategy->setCancelOrderCallback([this](const QString &orderId) {
            processCancelOrder(orderId);
        });
        m_strategies.append(strategy);
    }
}

bool BacktestEngine::runBacktest()
{
    if (!initialize()) {
        emit logMessage(u8"回测初始化失败", 2);
        return false;
    }

    execute();
    cleanup();
    calculateMetrics();

    emit backtestFinished();
    return true;
}

AppData::BacktestResult BacktestEngine::getBacktestResult() const
{
    return m_result;
}

void BacktestEngine::setDataManager(std::shared_ptr<HistoryDataManager> dataManager)
{
    m_dataManager = dataManager;
}

bool BacktestEngine::initialize()
{
    // 初始化账户
    m_account.accountId = "backtest_account";
    m_account.name = "回测账户";
    m_account.balance = m_params.initialCapital;
    m_account.available = m_params.initialCapital;
    m_account.margin = 0.0;
    m_account.unrealizedPnL = 0.0;
    m_account.realizedPnL = 0.0;

    // 加载市场数据
    for (const auto &symbol : m_params.symbols) {
        QVector<AppData::MarketData> symbolData;
        if (!m_dataManager->loadHistoricalData(symbol, 
                                              m_params.startDate, 
                                              m_params.endDate, 
                                              symbolData, 
                                              m_params.timeFrame)) {
            emit logMessage(tr("加载历史数据失败: %1").arg(symbol), 2);
            return false;
        }
        m_marketData.append(symbolData);
    }

    // 按时间排序市场数据
    std::sort(m_marketData.begin(), m_marketData.end(), 
              [](const AppData::MarketData &a, const AppData::MarketData &b) {
                  return a.timestamp < b.timestamp;
              });

    // 初始化策略
    for (auto &strategy : m_strategies) {
        strategy->setAccount(m_account);
        strategy->initialize();
    }

    return true;
}

void BacktestEngine::execute()
{
    int totalSteps = m_marketData.size();
    int currentStep = 0;

    for (const auto &data : m_marketData) {
        m_currentTime = data.timestamp;

        // 更新策略
        for (auto &strategy : m_strategies) {
            strategy->onTick(data);
        }

        // 撮合订单
        matchOrders(data);

        // 更新进度
        currentStep++;
        int progress = static_cast<int>(currentStep * 100.0 / totalSteps);
        emit progressUpdated(progress);

        // 处理事件循环
        QCoreApplication::processEvents();
    }
}

void BacktestEngine::cleanup()
{
    for (auto &strategy : m_strategies) {
        strategy->cleanup();
    }
}

void BacktestEngine::processOrder(const AppData::Order &order)
{
    m_activeOrders[order.orderId] = order;
}

void BacktestEngine::processCancelOrder(const QString &orderId)
{
    if (m_activeOrders.contains(orderId)) {
        m_activeOrders.remove(orderId);
    }
}

void BacktestEngine::matchOrders(const AppData::MarketData &data)
{
    QVector<QString> toRemove;

    for (auto it = m_activeOrders.begin(); it != m_activeOrders.end(); ++it) {
        const auto &order = it.value();

        // 检查订单是否匹配
        bool matched = false;
        double fillPrice = 0.0;

        if (order.type == AppData::Market) {
            // 市价单立即成交
            matched = true;
            fillPrice = data.close;
        } else if (order.type == AppData::Limit) {
            // 限价单检查价格
            if ((order.direction == AppData::Long && order.price >= data.low) ||
                (order.direction == AppData::Short && order.price <= data.high)) {
                matched = true;
                fillPrice = order.price;
            }
        } else if (order.type == AppData::Stop) {
            // 止损单检查价格
            if ((order.direction == AppData::Long && data.high >= order.stopPrice) ||
                (order.direction == AppData::Short && data.low <= order.stopPrice)) {
                matched = true;
                fillPrice = order.stopPrice;
            }
        }

        if (matched) {
            // 创建成交记录
            AppData::Trade trade;
            trade.tradeId = QString("trade_%1").arg(m_trades.size() + 1);
            trade.orderId = order.orderId;
            trade.symbol = order.symbol;
            trade.tradeTime = m_currentTime;
            trade.direction = order.direction;
            trade.price = fillPrice;
            trade.quantity = order.quantity;
            trade.commission = trade.price * trade.quantity * m_params.commission;
            trade.accountId = m_account.accountId;

            // 更新订单状态
            AppData::Order updatedOrder = order;
            updatedOrder.status = AppData::Completed;
            updatedOrder.filledQuantity = order.quantity;
            updatedOrder.avgFillPrice = fillPrice;
            updatedOrder.updateTime = m_currentTime;

            // 添加到成交记录
            m_trades.append(trade);

            // 更新账户
            updateAccount(trade);

            // 通知策略
            for (auto &strategy : m_strategies) {
                strategy->updateOrder(updatedOrder);
                strategy->onTrade(trade);
            }

            // 标记订单为待删除
            toRemove.append(order.orderId);
        }
    }

    // 删除已成交的订单
    for (const auto &orderId : toRemove) {
        m_activeOrders.remove(orderId);
    }
}

void BacktestEngine::updateAccount(const AppData::Trade &trade)
{
    // 计算盈亏
    double pnl = 0.0;
    if (trade.direction == AppData::Long) {
        // 做多：卖出时计算盈亏
        if (m_account.positions.contains(trade.symbol)) {
            auto &position = m_account.positions[trade.symbol];
            if (position.direction == AppData::Long) {
                pnl = (trade.price - position.avgPrice) * trade.quantity;
            }
        }
    } else {
        // 做空：买入时计算盈亏
        if (m_account.positions.contains(trade.symbol)) {
            auto &position = m_account.positions[trade.symbol];
            if (position.direction == AppData::Short) {
                pnl = (position.avgPrice - trade.price) * trade.quantity;
            }
        }
    }

    // 更新账户余额
    m_account.realizedPnL += pnl;
    m_account.balance -= trade.commission;
    m_account.available = m_account.balance;

    // 更新持仓
    if (trade.direction == AppData::Long) {
        // 买入
        if (!m_account.positions.contains(trade.symbol)) {
            AppData::Position position;
            position.symbol = trade.symbol;
            position.direction = AppData::Long;
            position.quantity = trade.quantity;
            position.avgPrice = trade.price;
            position.openTime = m_currentTime;
            m_account.positions[trade.symbol] = position;
        } else {
            auto &position = m_account.positions[trade.symbol];
            position.avgPrice = (position.avgPrice * position.quantity + trade.price * trade.quantity) / 
                               (position.quantity + trade.quantity);
            position.quantity += trade.quantity;
        }
    } else {
        // 卖出
        if (!m_account.positions.contains(trade.symbol)) {
            AppData::Position position;
            position.symbol = trade.symbol;
            position.direction = AppData::Short;
            position.quantity = trade.quantity;
            position.avgPrice = trade.price;
            position.openTime = m_currentTime;
            m_account.positions[trade.symbol] = position;
        } else {
            auto &position = m_account.positions[trade.symbol];
            position.avgPrice = (position.avgPrice * position.quantity + trade.price * trade.quantity) / 
                               (position.quantity + trade.quantity);
            position.quantity += trade.quantity;
        }
    }

    // 更新策略账户信息
    for (auto &strategy : m_strategies) {
        strategy->setAccount(m_account);
    }
}

void BacktestEngine::calculateMetrics()
{
    // 计算回测结果
    m_result.finalCapital = m_account.balance;
    m_result.totalReturn = (m_account.balance - m_params.initialCapital) / m_params.initialCapital;
    
    // 计算年化收益率
    double days = m_params.startDate.daysTo(m_params.endDate);
    if (days > 0) {
        m_result.annualReturn = std::pow(1 + m_result.totalReturn, 365.0 / days) - 1;
    }
    
    // 计算夏普比率
    double sumReturns = 0.0;
    double sumSquaredReturns = 0.0;
    int count = 0;
    
    for (int i = 1; i < m_trades.size(); i++) {
        double ret = (m_trades[i].price - m_trades[i-1].price) / m_trades[i-1].price;
        sumReturns += ret;
        sumSquaredReturns += ret * ret;
        count++;
    }
    
    if (count > 0) {
        double meanReturn = sumReturns / count;
        double stdDev = std::sqrt((sumSquaredReturns - sumReturns * sumReturns / count) / (count - 1));
        if (stdDev > 0) {
            m_result.sharpeRatio = meanReturn / stdDev * std::sqrt(252); // 假设252个交易日
        }
    }
    
    // 计算最大回撤
    double peak = m_params.initialCapital;
    double maxDrawdown = 0.0;
    double equity = m_params.initialCapital;
    
    for (const auto &trade : m_trades) {
        equity += (trade.direction == AppData::Long ? 1 : -1) * 
                 (trade.price * trade.quantity - trade.commission);
        
        if (equity > peak) {
            peak = equity;
        } else {
            double drawdown = (peak - equity) / peak;
            if (drawdown > maxDrawdown) {
                maxDrawdown = drawdown;
            }
        }
    }
    
    m_result.maxDrawdown = maxDrawdown;
    
    // 计算胜率
    int winTrades = 0;
    int lossTrades = 0;
    double totalProfit = 0.0;
    double totalLoss = 0.0;
    
    for (int i = 1; i < m_trades.size(); i++) {
        if (m_trades[i].price > m_trades[i-1].price) {
            winTrades++;
            totalProfit += m_trades[i].price - m_trades[i-1].price;
        } else {
            lossTrades++;
            totalLoss += m_trades[i-1].price - m_trades[i].price;
        }
    }
    
    m_result.winTrades = winTrades;
    m_result.lossTrades = lossTrades;
    m_result.totalTrades = winTrades + lossTrades;
    m_result.winRate = winTrades * 1.0 / (winTrades + lossTrades);
    m_result.profitFactor = totalProfit / totalLoss;
    m_result.averageProfit = winTrades > 0 ? totalProfit / winTrades : 0.0;
    m_result.averageLoss = lossTrades > 0 ? totalLoss / lossTrades : 0.0;
    
    // 保存交易记录
    m_result.trades = m_trades;
    
    // 生成权益曲线
    double currentEquity = m_params.initialCapital;
    for (const auto &trade : m_trades) {
        currentEquity += (trade.direction == AppData::Long ? 1 : -1) * 
                        (trade.price * trade.quantity - trade.commission);
        m_result.equityCurve.append(currentEquity);
        m_result.equityTimes.append(trade.tradeTime);
    }
}
