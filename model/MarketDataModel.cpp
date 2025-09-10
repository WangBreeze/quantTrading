#include "MarketDataModel.h"

MarketDataModel::MarketDataModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_maxDataCount(1000)  // 默认最大数据量为1000
{
}

int MarketDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_dataList.size();
}

QVariant MarketDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_dataList.size())
        return QVariant();
    
    const AppData::MarketData &data = m_dataList.at(index.row());
    
    switch (role) {
        case SymbolRole:
            return data.symbol;
        case TimestampRole:
            return data.timestamp;
        case PriceRole:
            return data.price;
        case OpenRole:
            return data.open;
        case HighRole:
            return data.high;
        case LowRole:
            return data.low;
        case CloseRole:
            return data.close;
        case VolumeRole:
            return data.volume;
        case AmountRole:
            return data.amount;
        case TickCountRole:
            return data.tickCount;
        case OpenInterestRole:
            return data.openInterest;
        case BidPriceRole:
            return data.bidPrice;
        case AskPriceRole:
            return data.askPrice;
        case BidVolumeRole:
            return data.bidVolume;
        case AskVolumeRole:
            return data.askVolume;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> MarketDataModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SymbolRole] = "symbol";
    roles[TimestampRole] = "timestamp";
    roles[PriceRole] = "price";
    roles[OpenRole] = "open";
    roles[HighRole] = "high";
    roles[LowRole] = "low";
    roles[CloseRole] = "close";
    roles[VolumeRole] = "volume";
    roles[AmountRole] = "amount";
    roles[TickCountRole] = "tickCount";
    roles[OpenInterestRole] = "openInterest";
    roles[BidPriceRole] = "bidPrice";
    roles[AskPriceRole] = "askPrice";
    roles[BidVolumeRole] = "bidVolume";
    roles[AskVolumeRole] = "askVolume";
    return roles;
}

void MarketDataModel::appendData(const AppData::MarketData &data)
{
    beginInsertRows(QModelIndex(), m_dataList.size(), m_dataList.size());
    m_dataList.append(data);
    endInsertRows();
    
    // 如果数据量超过最大值，移除最早的数据
    if (m_dataList.size() > m_maxDataCount) {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_dataList.removeFirst();
        endRemoveRows();
    }
    
    emit dataChanged();
}

void MarketDataModel::appendData(const QVector<AppData::MarketData> &dataList)
{
    if (dataList.isEmpty())
        return;
    
    beginInsertRows(QModelIndex(), m_dataList.size(), m_dataList.size() + dataList.size() - 1);
    m_dataList.append(dataList);
    endInsertRows();
    
    // 如果数据量超过最大值，移除最早的数据
    if (m_dataList.size() > m_maxDataCount) {
        int removeCount = m_dataList.size() - m_maxDataCount;
        beginRemoveRows(QModelIndex(), 0, removeCount - 1);
        m_dataList.remove(0, removeCount);
        endRemoveRows();
    }
    
    emit dataChanged();
}

void MarketDataModel::updateData(int index, const AppData::MarketData &data)
{
    if (index < 0 || index >= m_dataList.size())
        return;
    
    m_dataList[index] = data;
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit dataChanged();
}

void MarketDataModel::clearData()
{
    beginResetModel();
    m_dataList.clear();
    endResetModel();
    
    emit dataChanged();
}

QVector<AppData::MarketData> MarketDataModel::getMarketDataList() const
{
    return m_dataList;
}

AppData::MarketData MarketDataModel::getMarketData(int index) const
{
    if (index < 0 || index >= m_dataList.size())
        return AppData::MarketData();
    
    return m_dataList.at(index);
}

void MarketDataModel::setMaxDataCount(int count)
{
    if (count <= 0)
        return;
    
    m_maxDataCount = count;
    
    // 如果当前数据量超过新的最大值，移除多余的数据
    if (m_dataList.size() > m_maxDataCount) {
        int removeCount = m_dataList.size() - m_maxDataCount;
        beginRemoveRows(QModelIndex(), 0, removeCount - 1);
        m_dataList.remove(0, removeCount);
        endRemoveRows();
        
        emit dataChanged();
    }
}

void MarketDataModel::onNewMarketData(const AppData::MarketData &data)
{
    // 检查是否有相同 symbol 和 timestamp 的数据，如果有则更新，否则添加
    for (int i = 0; i < m_dataList.size(); ++i) {
        if (m_dataList[i].symbol == data.symbol && m_dataList[i].timestamp == data.timestamp) {
            updateData(i, data);
            return;
        }
    }
    
    // 没有找到匹配的数据，添加新数据
    appendData(data);
}