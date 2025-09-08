#ifndef RSI_H
#define RSI_H

#include "IndicatorBase.h"

class RSI : public IndicatorBase
{
    Q_OBJECT
public:
    explicit RSI(int period = 14, QObject *parent = nullptr);
    
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
    QVector<double> m_values;
    double m_lastClose;
    double m_avgGain;
    double m_avgLoss;
};

#endif // RSI_H