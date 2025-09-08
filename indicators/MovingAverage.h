#ifndef MOVINGAVERAGE_H
#define MOVINGAVERAGE_H

#include "IndicatorBase.h"

class MovingAverage : public IndicatorBase
{
    Q_OBJECT
public:
    enum MAType {
        SMA,    // 简单移动平均
        EMA,    // 指数移动平均
        WMA     // 加权移动平均
    };

    explicit MovingAverage(int period, MAType maType = SMA, QObject *parent = nullptr);
    
    // 计算指标值
    void calculate(const QVector<AppData::MarketData> &data) override;
    
    // 实时更新指标
    void update(const AppData::MarketData &newData) override;
    
    // 获取指标名称
    QString name() const override;
    
    // 获取计算结果
    QVector<double> values() const override;
    
    // 获取最新值
    double lastValue() const override;

private:
    int m_period;
    MAType m_maType;
    QVector<double> m_values;
};

#endif // MOVINGAVERAGE_H