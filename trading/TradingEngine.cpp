#include "TradingEngine.h"
#include <QDebug>

TradingEngine::TradingEngine(QObject *parent)
    : QObject(parent), m_isTrading(false)
{
}

TradingEngine::~TradingEngine()
{
    stopTrading();
}

bool TradingEngine::initialize()
{
    if (m_isTrading) {
        qWarning() << "Trading is already running";
        return false;
    }

    // 初始化账户和持仓
    m_account = AppData::Account();
    m_positions.clear();
    m_activeOrders.clear();

    // 初始化策略
    for (auto &strategy : m_strategies) {
        strategy->setAccount(m_account);
        strategy->initialize();
    }

    return true;
}

void TradingEngine::addStrategy(std::shared_ptr<Strategy> strategy)
{
    if (m_isTrading) {
        qWarning() << "Cannot add strategy while trading is running";
        return;
    }

    if (strategy) {
        strategy->setOrderCallback([this](const AppData::Order &order) {
            executeOrder(order);
        });
        strategy->setCancelOrderCallback([this](const QString &orderId) {
            cancelOrder(orderId);
        });
        m_strategies.append(strategy);
    }
}

void TradingEngine::setAccount(const AppData::Account &account)
{
    if (!m_isTrading) {
        m_account = account;
    }
}

bool TradingEngine::startTrading()
{
    if (m_isTrading) {
        qWarning() << "Trading is already running";
        return false;
    }

    if (m_strategies.isEmpty()) {
        qWarning() << "No strategies added to trading engine";
        return false;
    }

    m_isTrading = true;
    emit statusUpdated("Trading started");
    return true;
}

void TradingEngine::stopTrading()
{
    if (!m_isTrading) return;

    m_isTrading = false;

    // 取消所有活动订单
    for (const auto &orderId : m_activeOrders.keys()) {
        cancelOrder(orderId);
    }

    // 清理策略
    for (auto &strategy : m_strategies) {
        strategy->cleanup();
    }

    emit statusUpdated("Trading stopped");
}

QMap<QString, AppData::Position> TradingEngine::getPositions() const
{
    return m_positions;
}

void TradingEngine::executeOrder(const AppData::Order &order)
{
    if (!m_isTrading) return;

    // 模拟订单执行
    AppData::Trade trade;
    trade.tradeId = QString("trade_%1").arg(m_account.trades.size() + 1);
    trade.orderId = order.orderId;
    trade.symbol = order.symbol;
    trade.tradeTime = QDateTime::currentDateTime();
    trade.direction = order.direction;
    trade.price = order.price;
    trade.quantity = order.quantity;
    trade.commission = trade.price * trade.quantity * 0.0003; // 0.03%佣金

    // 更新订单状态
    AppData::Order updatedOrder = order;
    updatedOrder.status = AppData::Completed;
    updatedOrder.filledQuantity = order.quantity;
    updatedOrder.avgFillPrice = order.price;
    updatedOrder.updateTime = QDateTime::currentDateTime();

    // 更新账户
    updateAccount(trade);

    // 通知策略
    for (auto &strategy : m_strategies) {
        strategy->updateOrder(updatedOrder);
        strategy->onTrade(trade);
    }

    emit tradeExecuted(trade);
}

void TradingEngine::cancelOrder(const QString &orderId)
{
    if (m_activeOrders.contains(orderId)) {
        m_activeOrders.remove(orderId);
        emit statusUpdated(QString("Order %1 canceled").arg(orderId));
    }
}

void TradingEngine::updateAccount(const AppData::Trade &trade)
{
    // 计算盈亏
    double pnl = 0.0;
    if (trade.direction == AppData::Long) {
        // 做多：卖出时计算盈亏
        if (m_positions.contains(trade.symbol)) {
            auto &position = m_positions[trade.symbol];
            if (position.direction == AppData::Long) {
                pnl = (trade.price - position.avgPrice) * trade.quantity;
            }
        }
    } else {
        // 做空：买入时计算盈亏
        if (m_positions.contains(trade.symbol)) {
            auto &position = m_positions[trade.symbol];
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
        if (!m_positions.contains(trade.symbol)) {
            AppData::Position position;
            position.symbol = trade.symbol;
            position.direction = AppData::Long;
            position.quantity = trade.quantity;
            position.avgPrice = trade.price;
            position.openTime = QDateTime::currentDateTime();
            m_positions[trade.symbol] = position;
        } else {
            auto &position = m_positions[trade.symbol];
            position.avgPrice = (position.avgPrice * position.quantity + trade.price * trade.quantity) / 
                               (position.quantity + trade.quantity);
            position.quantity += trade.quantity;
        }
    } else {
        // 卖出
        if (!m_positions.contains(trade.symbol)) {
            AppData::Position position;
            position.symbol = trade.symbol;
            position.direction = AppData::Short;
            position.quantity = trade.quantity;
            position.avgPrice = trade.price;
            position.openTime = QDateTime::currentDateTime();
            m_positions[trade.symbol] = position;
        } else {
            auto &position = m_positions[trade.symbol];
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