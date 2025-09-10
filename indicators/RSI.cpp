#include "RSI.h"
#include <cmath>
#include "../AppData.h"

RSI::RSI(int period, QObject *parent)
    : IndicatorBase(AppData::RSI, parent),
      m_period(period),
      m_lastClose(0.0),
      m_avgGain(0.0),
      m_avgLoss(0.0)
{
}

void RSI::calculate(const QVector<AppData::MarketData> &data)
{
    m_values.clear();
    if (data.size() <= m_period) return;
    
    // 计算初始平均增益和平均损失
    double sumGain = 0.0;
    double sumLoss = 0.0;
    
    for (int i = 1; i <= m_period; ++i) {
        double change = data[i].close - data[i-1].close;
        if (change > 0) {
            sumGain += change;
        } else {
            sumLoss += std::abs(change);
        }
    }
    
    m_avgGain = sumGain / m_period;
    m_avgLoss = sumLoss / m_period;
    
    // 计算第一个RSI值
    double rs = m_avgLoss == 0.0 ? 100.0 : 100.0 - (100.0 / (1.0 + m_avgGain / m_avgLoss));
    m_values.append(rs);
    
    // 计算后续RSI值
    for (int i = m_period + 1; i < data.size(); ++i) {
        double change = data[i].close - data[i-1].close;
        double gain = change > 0 ? change : 0.0;
        double loss = change < 0 ? std::abs(change) : 0.0;
        
        m_avgGain = (m_avgGain * (m_period - 1) + gain) / m_period;
        m_avgLoss = (m_avgLoss * (m_period - 1) + loss) / m_period;
        
        double rs = m_avgLoss == 0.0 ? 100.0 : 100.0 - (100.0 / (1.0 + m_avgGain / m_avgLoss));
        m_values.append(rs);
    }
    
    m_lastClose = data.last().close;
    emit indicatorUpdated();
}

void RSI::update(const AppData::MarketData &newData)
{
    if (m_values.isEmpty()) {
        m_lastClose = newData.close;
        return;
    }
    
    double change = newData.close - m_lastClose;
    double gain = change > 0 ? change : 0.0;
    double loss = change < 0 ? std::abs(change) : 0.0;
    
    m_avgGain = (m_avgGain * (m_period - 1) + gain) / m_period;
    m_avgLoss = (m_avgLoss * (m_period - 1) + loss) / m_period;
    
    double rs = m_avgLoss == 0.0 ? 100.0 : 100.0 - (100.0 / (1.0 + m_avgGain / m_avgLoss));
    m_values.append(rs);
    m_lastClose = newData.close;
    
    emit indicatorUpdated();
}

QString RSI::name() const
{
    return QString("RSI(%1)").arg(m_period);
}

QVector<double> RSI::values() const
{
    return m_values;
}

double RSI::lastValue() const
{
    return m_values.isEmpty() ? 50.0 : m_values.last();
}
