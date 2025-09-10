#ifndef REALTIMEDATAMODEL_H
#define REALTIMEDATAMODEL_H

#include "BaseModel.h"
#include <QTimer>
#include <memory>
#include "../online/onlinemarket.h"

class RealtimeDataModel : public BaseModel
{
    Q_OBJECT
public:
    explicit RealtimeDataModel(QObject *parent = nullptr);
    ~RealtimeDataModel();

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

    // 设置数据源
    void setDataSource(std::shared_ptr<onLineMarket> dataSource);

private slots:
    void onNewMarketData(const AppData::MarketData &data);
    void onNewTrade(const AppData::Trade &trade);
    void onAccountUpdate(const AppData::Account &account);

private:
    std::shared_ptr<onLineMarket> m_dataSource; // 数据源
    QVector<AppData::MarketData> m_marketData;  // 市场数据
    QVector<AppData::Trade> m_trades;            // 成交数据
    AppData::Account m_account;                 // 账户数据
    QTimer m_updateTimer;                       // 数据更新定时器
};

#endif // REALTIMEDATAMODEL_H
