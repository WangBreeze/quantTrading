#ifndef MARKETDATAMODEL_H
#define MARKETDATAMODEL_H

#include "BaseModel.h"
#include <QVector>
#include <QAbstractListModel>

// 为 ChartView 提供的 MarketData 列表模型
class MarketDataModel : public QAbstractListModel
{
    Q_OBJECT
    
    // 定义角色枚举，用于 QML 访问数据
    enum Roles {
        SymbolRole = Qt::UserRole + 1,
        TimestampRole,
        PriceRole,
        OpenRole,
        HighRole,
        LowRole,
        CloseRole,
        VolumeRole,
        AmountRole,
        TickCountRole,
        OpenInterestRole,
        BidPriceRole,
        AskPriceRole,
        BidVolumeRole,
        AskVolumeRole
    };

public:
    explicit MarketDataModel(QObject *parent = nullptr);
    
    // QAbstractListModel 接口实现
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // 数据操作方法
    void appendData(const AppData::MarketData &data);
    void appendData(const QVector<AppData::MarketData> &dataList);
    void updateData(int index, const AppData::MarketData &data);
    void clearData();
    
    // 获取数据
    QVector<AppData::MarketData> getMarketDataList() const;
    AppData::MarketData getMarketData(int index) const;
    
    // 设置最大数据量
    void setMaxDataCount(int count);
    
public slots:
    // 更新数据槽函数
    void onNewMarketData(const AppData::MarketData &data);
    
signals:
    // 数据更新信号
    void dataChanged();
    
private:
    QVector<AppData::MarketData> m_dataList;  // 市场数据列表
    int m_maxDataCount;                       // 最大数据量
};

#endif // MARKETDATAMODEL_H