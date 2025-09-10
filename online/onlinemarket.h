#ifndef ONLINEMARKET_H
#define ONLINEMARKET_H

#include <QObject>
#include <QString>
#include "../AppData.h"

class onLineMarket : public QObject
{
    Q_OBJECT
public:
    explicit onLineMarket(QObject *parent = nullptr);
    virtual ~onLineMarket() = default;

    // 设置要获取的交易对
    virtual void setSymbol(const QString &symbol) = 0;
    
    // 设置数据获取间隔（毫秒）
    virtual void setUpdateInterval(int milliseconds) = 0;
    
    // 开始获取行情数据
    virtual void startFetchingData() = 0;
    
    // 停止获取行情数据
    virtual void stopFetchingData() = 0;

signals:
    // 当收到新的行情数据时发出信号
    void newMarketData(const AppData::MarketData &data);
    
    // 错误信号
    void errorOccurred(const QString &errorMessage);
    
    // 当收到新的交易数据时发出信号
    void newTrade(const AppData::Trade &trade);
    
    // 当账户信息更新时发出信号
    void accountUpdate(const AppData::Account &account);
};

#endif // ONLINEMARKET_H
