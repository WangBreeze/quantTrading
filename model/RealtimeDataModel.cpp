#include "RealtimeDataModel.h"
#include <QDateTime>

RealtimeDataModel::RealtimeDataModel(QObject *parent)
    : BaseModel(parent)
{
    // 设置更新定时器，每秒更新一次
    m_updateTimer.setInterval(1000);
    connect(&m_updateTimer, &QTimer::timeout, this, &RealtimeDataModel::updateData);
    m_updateTimer.start();
}

RealtimeDataModel::~RealtimeDataModel()
{
    if (m_dataSource) {
        disconnect(m_dataSource.get(), nullptr, this, nullptr);
    }
}

int RealtimeDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_trades.size();
}

int RealtimeDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_headers.size();
}

QVariant RealtimeDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_trades.size() || index.column() >= m_headers.size()) {
        return QVariant();
    }
    
    if (role == Qt::DisplayRole) {
        const auto &trade = m_trades[index.row()];
        
        switch (index.column()) {
            case 0: return trade.tradeTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
            case 1: return trade.symbol;
            case 2: return QString::number(trade.price, 'f', 4);
            case 3: return QString::number(trade.quantity, 'f', 4);
            case 4: return trade.direction == AppData::Long ? "买入" : "卖出";
            case 5: return QString::number(trade.price * trade.quantity, 'f', 2);
            default: return QVariant();
        }
    }
    
    return QVariant();
}

QVector<AppData::MarketData> RealtimeDataModel::getMarketData() const
{
    return m_marketData;
}

QVector<AppData::Trade> RealtimeDataModel::getTradeData() const
{
    return m_trades;
}

AppData::Account RealtimeDataModel::getAccountData() const
{
    return m_account;
}

void RealtimeDataModel::updateData()
{
    beginResetModel();
    // 这里可以添加数据更新逻辑
    endResetModel();
    emit dataUpdated();
}

void RealtimeDataModel::setDataSource(std::shared_ptr<onLineMarket> dataSource)
{
    if (m_dataSource) {
        disconnect(m_dataSource.get(), nullptr, this, nullptr);
    }
    
    m_dataSource = dataSource;
    
    if (m_dataSource) {
        connect(m_dataSource.get(), &onLineMarket::newMarketData,
                this, &RealtimeDataModel::onNewMarketData);
        connect(m_dataSource.get(), &onLineMarket::newTrade,
                this, &RealtimeDataModel::onNewTrade);
        connect(m_dataSource.get(), &onLineMarket::accountUpdate,
                this, &RealtimeDataModel::onAccountUpdate);
    }
}

void RealtimeDataModel::onNewMarketData(const AppData::MarketData &data)
{
    // 更新市场数据
    if (m_marketData.isEmpty() || m_marketData.last().timestamp != data.timestamp) {
        m_marketData.append(data);
    } else {
        m_marketData.last() = data;
    }
    
    emit dataUpdated();
}

void RealtimeDataModel::onNewTrade(const AppData::Trade &trade)
{
    // 添加新成交记录
    beginInsertRows(QModelIndex(), m_trades.size(), m_trades.size());
    m_trades.append(trade);
    endInsertRows();
    
    emit dataUpdated();
}

void RealtimeDataModel::onAccountUpdate(const AppData::Account &account)
{
    m_account = account;
    emit dataUpdated();
}