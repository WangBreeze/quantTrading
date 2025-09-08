#include "HistoryDataModel.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>

HistoryDataModel::HistoryDataModel(QObject *parent)
    : BaseModel(parent)
{
}

int HistoryDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_trades.size();
}

int HistoryDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_headers.size();
}

QVariant HistoryDataModel::data(const QModelIndex &index, int role) const
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

QVector<AppData::MarketData> HistoryDataModel::getMarketData() const
{
    return m_marketData;
}

QVector<AppData::Trade> HistoryDataModel::getTradeData() const
{
    return m_trades;
}

AppData::Account HistoryDataModel::getAccountData() const
{
    return m_account;
}

void HistoryDataModel::updateData()
{
    beginResetModel();
    // 这里可以添加数据更新逻辑
    endResetModel();
    emit dataUpdated();
}

bool HistoryDataModel::loadFromCsv(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(tr("无法打开文件: %1").arg(filePath));
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    // 跳过标题行
    if (!in.atEnd()) {
        in.readLine();
    }
    
    QVector<AppData::Trade> newTrades;
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        
        if (fields.size() < 8) {
            continue;
        }
        
        AppData::Trade trade;
        trade.symbol = fields[0];
        trade.tradeTime = QDateTime::fromString(fields[1], Qt::ISODate);
        trade.price = fields[5].toDouble();
        trade.quantity = fields[6].toDouble();
        trade.direction = trade.quantity > 0 ? AppData::Long : AppData::Short;
        trade.quantity = qAbs(trade.quantity);
        trade.commission = fields[7].toDouble();
        
        newTrades.append(trade);
    }
    
    file.close();
    
    beginResetModel();
    m_trades = newTrades;
    endResetModel();
    
    emit dataUpdated();
    return true;
}