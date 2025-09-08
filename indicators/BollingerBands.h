#ifndef BOLLINGERBANDS_H
#define BOLLINGERBANDS_H

#include "IndicatorBase.h"

class BollingerBands : public IndicatorBase
{
    Q_OBJECT
public:
    explicit BollingerBands(int period = 20, double multiplier = 2.0, QObject *parent = nullptr);
    
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

    // 获取上轨线
    QVector<double> upperBand() const;
    
    // 获取中轨线
    QVector<double> middleBand() const;
    
    // 获取下轨线
    QVector<double> lowerBand() const;

private:
    int m_period;
    double m_multiplier;
    QVector<double> m_upperBand;
    QVector<double> m_middleBand;
    QVector<double> m_lowerBand;
};

#endif // BOLLINGERBANDS_H