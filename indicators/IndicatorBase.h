#ifndef INDICATORBASE_H
#define INDICATORBASE_H

#include <QObject>
#include <QVector>
#include "../AppData.h"

class IndicatorBase : public QObject
{
    Q_OBJECT
public:


    explicit IndicatorBase(AppData::IndicatorType type, QObject *parent = nullptr);
    virtual ~IndicatorBase();

    // 计算指标值
    virtual void calculate(const QVector<AppData::MarketData> &data) = 0;
    
    // 实时更新指标
    virtual void update(const AppData::MarketData &newData) = 0;
    
    // 获取指标名称
    virtual QString name() const = 0;
    
    // 获取指标类型
    AppData::IndicatorType type() const;
    
    // 获取计算结果
    virtual QVector<double> values() const = 0;
    
    // 获取最新值
    virtual double lastValue() const = 0;

signals:
    void indicatorUpdated();

protected:
    AppData::IndicatorType m_type;
};

#endif // INDICATORBASE_H
