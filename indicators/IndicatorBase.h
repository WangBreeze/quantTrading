#ifndef INDICATORBASE_H
#define INDICATORBASE_H

#include <QObject>
#include <QVector>
#include "../online/AppData.h"

class IndicatorBase : public QObject
{
    Q_OBJECT
public:
    enum IndicatorType {
        MA,
        MACD,
        RSI,
        BOLL
    };

    explicit IndicatorBase(IndicatorType type, QObject *parent = nullptr);
    virtual ~IndicatorBase();

    // 计算指标值
    virtual void calculate(const QVector<AppData::MarketData> &data) = 0;
    
    // 实时更新指标
    virtual void update(const AppData::MarketData &newData) = 0;
    
    // 获取指标名称
    virtual QString name() const = 0;
    
    // 获取指标类型
    IndicatorType type() const;
    
    // 获取计算结果
    virtual QVector<double> values() const = 0;
    
    // 获取最新值
    virtual double lastValue() const = 0;

signals:
    void indicatorUpdated();

protected:
    IndicatorType m_type;
};

#endif // INDICATORBASE_H