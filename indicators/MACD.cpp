#include "MACD.h"
#include <cmath>

MACD::MACD(int fastPeriod, int slowPeriod, int signalPeriod, QObject *parent)
    : IndicatorBase(MACD, parent), 
      m_fastPeriod(fastPeriod), 
      m_slowPeriod(slowPeriod), 
      m_signalPeriod(signalPeriod)
{
}

void MACD::calculate(const QVector<AppData::MarketData> &data)
{
    m_difValues.clear();
    m_deaValues.clear();
    m_macdHist.clear();
    
    if (data.size() < m_slowPeriod + m_signalPeriod) return;
    
    // 计算EMA12和EMA26
    QVector<double> emaFast(data.size());
    QVector<double> emaSlow(data.size());
    
    double fastMultiplier = 2.0 / (m_fastPeriod + 1);
    double slowMultiplier = 2.0 / (m_slowPeriod + 1);
    
    emaFast[0] = data[0].close;
    emaSlow[0] = data[0].close;
    
    for (int i = 1; i < data.size(); ++i) {
        emaFast[i] = (data[i].close - emaFast[i-1]) * fastMultiplier + emaFast[i-1];
        emaSlow[i] = (data[i].close - emaSlow[i-1]) * slowMultiplier + emaSlow[i-1];
    }
    
    // 计算DIF
    for (int i = 0; i < data.size(); ++i) {
        m_difValues.append(emaFast[i] - emaSlow[i]);
    }
    
    // 计算DEA
    double deaMultiplier = 2.0 / (m_signalPeriod + 1);
    m_deaValues.append(m_difValues[0]);
    
    for (int i = 1; i < m_difValues.size(); ++i) {
        double dea = (m_difValues[i] - m_deaValues.last()) * deaMultiplier + m_deaValues.last();
        m_deaValues.append(dea);
    }
    
    // 计算MACD柱
    for (int i = 0; i < m_difValues.size(); ++i) {
        m_macdHist.append(m_difValues[i] - m_deaValues[i]);
    }
    
    emit indicatorUpdated();
}

void MACD::update(const AppData::MarketData &newData)
{
    // 需要维护足够的历史数据才能实时更新MACD
    // 实际实现中需要保存足够的历史数据
    emit indicatorUpdated();
}

QString MACD::name() const
{
    return QString("MACD(%1,%2,%3)").arg(m_fastPeriod).arg(m_slowPeriod).arg(m_signalPeriod);
}

QVector<double> MACD::values() const
{
    return m_macdHist;
}

double MACD::lastValue() const
{
    return m_macdHist.isEmpty() ? 0.0 : m_macdHist.last();
}

QVector<double> MACD::difLine() const
{
    return m_difValues;
}

QVector<double> MACD::deaLine() const
{
    return m_deaValues;
}

QVector<double> MACD::macdHist() const
{
    return m_macdHist;
}