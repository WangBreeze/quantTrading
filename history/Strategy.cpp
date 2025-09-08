#include "Strategy.h"

Strategy::Strategy(QObject *parent)
    : QObject(parent)
    , m_isBacktest(false)
{
}

Strategy::~Strategy()
{
}

void Strategy::setParameter(const QString &name, const QVariant &value)
{
    m_parameters[name] = value;
}

QVariant Strategy::getParameter(const QString &name) const
{
    return m_parameters.value(name);
}

QString Strategy::getName() const
{
    return m_name;
}

void Strategy::setName(const QString &name)
{
    m_name = name;
}

QString Strategy::getDescription() const
{
    return m_description;
}

void Strategy::setDescription(const QString &description)
{
    m_description = description;
}

QString Strategy::getAuthor() const
{
    return m_author;
}

void Strategy::setAuthor(const QString &author)
{
    m_author = author;
}

QString Strategy::getVersion() const
{
    return m_version;
}

void Strategy::setVersion(const QString &version)
{
    m_version = version;
}

void Strategy::setBacktestMode(bool isBacktest)
{
    m_isBacktest = isBacktest;
}

bool Strategy::isBacktestMode() const
{
    return m_isBacktest;
}

void Strategy::setOrderCallback(std::function<void(const AppData::Order&)> callback)
{
    m_orderCallback = callback;
}

void Strategy::setCancelOrderCallback(std::function<void(const QString&)> callback)
{
    m_cancelOrderCallback = callback;
}

AppData::Order Strategy::buyMarket(const QString &symbol, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Long;
    order.type = AppData::Market;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

AppData::Order Strategy::sellMarket(const QString &symbol, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Short;
    order.type = AppData::Market;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

AppData::Order Strategy::buyLimit(const QString &symbol, double price, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Long;
    order.type = AppData::Limit;
    order.price = price;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

AppData::Order Strategy::sellLimit(const QString &symbol, double price, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Short;
    order.type = AppData::Limit;
    order.price = price;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

AppData::Order Strategy::buyStop(const QString &symbol, double stopPrice, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Long;
    order.type = AppData::Stop;
    order.stopPrice = stopPrice;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

AppData::Order Strategy::sellStop(const QString &symbol, double stopPrice, double quantity)
{
    AppData::Order order;
    order.symbol = symbol;
    order.direction = AppData::Short;
    order.type = AppData::Stop;
    order.stopPrice = stopPrice;
    order.quantity = quantity;
    order.createTime = QDateTime::currentDateTime();
    order.status = AppData::Created;
    
    if (m_orderCallback) {
        m_orderCallback(order);
    }
    
    return order;
}

bool Strategy::cancelOrder(const QString &orderId)
{
    if (m_cancelOrderCallback) {
        m_cancelOrderCallback(orderId);
        return true;
    }
    return false;
}

QVector<AppData::Position> Strategy::getPositions() const
{
    QVector<AppData::Position> positions;
    for (const auto &position : m_positions) {
        positions.append(position);
    }
    return positions;
}

AppData::Position Strategy::getPosition(const QString &symbol) const
{
    return m_positions.value(symbol);
}

QVector<AppData::Order> Strategy::getActiveOrders() const
{
    QVector<AppData::Order> activeOrders;
    for (const auto &order : m_orders) {
        if (order.status == AppData::Created || 
            order.status == AppData::Submitted || 
            order.status == AppData::Accepted || 
            order.status == AppData::Partial) {
            activeOrders.append(order);
        }
    }
    return activeOrders;
}

AppData::Order Strategy::getOrder(const QString &orderId) const
{
    return m_orders.value(orderId);
}

AppData::Account Strategy::getAccount() const
{
    return m_account;
}

void Strategy::setAccount(const AppData::Account &account)
{
    m_account = account;
}

void Strategy::addPosition(const AppData::Position &position)
{
    m_positions[position.symbol] = position;
}

void Strategy::addOrder(const AppData::Order &order)
{
    m_orders[order.orderId] = order;
}

void Strategy::updateOrder(const AppData::Order &order)
{
    if (m_orders.contains(order.orderId)) {
        m_orders[order.orderId] = order;
    }
}