#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include "../AppData.h"

class BaseModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BaseModel(QObject *parent = nullptr);
    
    // 基本模型接口
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override = 0;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override = 0;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override = 0;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    
    // 数据访问接口
    virtual QVector<AppData::MarketData> getMarketData() const = 0;
    virtual QVector<AppData::Trade> getTradeData() const = 0;
    virtual AppData::Account getAccountData() const = 0;
    
    // 数据更新接口
    virtual void updateData() = 0;
    
signals:
    void dataUpdated();
    void errorOccurred(const QString &message);
    
protected:
    QVector<QString> m_headers; // 表头数据
};

#endif // BASEMODEL_H