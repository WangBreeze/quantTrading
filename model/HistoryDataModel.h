#ifndef HISTORYDATAMODEL_H
#define HISTORYDATAMODEL_H

#include "BaseModel.h"
#include <QVector>
#include <QString>

class HistoryDataModel : public BaseModel
{
    Q_OBJECT
public:
    explicit HistoryDataModel(QObject *parent = nullptr);
    
    // 模型接口实现
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    // 数据访问接口实现
    QVector<AppData::MarketData> getMarketData() const override;
    QVector<AppData::Trade> getTradeData() const override;
    AppData::Account getAccountData() const override;
    
    // 数据更新接口实现
    void updateData() override;
    
    // 加载数据文件
    bool loadFromCsv(const QString &filePath);
    
private:
    QVector<AppData::MarketData> m_marketData; // 市场数据
    QVector<AppData::Trade> m_trades;          // 成交数据
    AppData::Account m_account;                // 账户数据
};

#endif // HISTORYDATAMODEL_H