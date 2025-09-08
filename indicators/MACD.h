#ifndef MACD_H
#define MACD_H

#include "IndicatorBase.h"

class MACD : public IndicatorBase
{
    Q_OBJECT
public:
    explicit MACD(int fastPeriod = 12, int slowPeriod = 26, int signalPeriod = 9, QObject *parent = nullptr);
    
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

    // 获取DIF线
    QVector<double> difLine() const;
    
    // 获取DEA线
    QVector<double> deaLine() const;
    
    // 获取MACD柱
    QVector<double> macdHist() const;

private:
    int m_fastPeriod;
    int m_slowPeriod;
    int m_signalPeriod;
    QVector<double> m_difValues;
    QVector<double> m_deaValues;
    QVector<double> m_macdHist;
};

#endif // MACD_H