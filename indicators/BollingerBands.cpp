#include "BollingerBands.h"
#include <cmath>
#include "../AppData.h"

BollingerBands::BollingerBands(int period, double multiplier, QObject *parent)
    : IndicatorBase(AppData::BOLL, parent),
      m_period(period), 
      m_multiplier(multiplier)
{
}

void BollingerBands::calculate(const QVector<AppData::MarketData> &data)
{
    m_upperBand.clear();
    m_middleBand.clear();
    m_lowerBand.clear();
    
    if (data.size() < m_period) return;
    
    for (int i = m_period - 1; i < data.size(); ++i) {
        // 计算中轨(简单移动平均)
        double sum = 0.0;
        for (int j = 0; j < m_period; ++j) {
            sum += data[i - j].close;
        }
        double middle = sum / m_period;
        m_middleBand.append(middle);
        
        // 计算标准差
        double variance = 0.0;
        for (int j = 0; j < m_period; ++j) {
            variance += std::pow(data[i - j].close - middle, 2);
        }
        double stddev = std::sqrt(variance / m_period);
        
        // 计算上下轨
        m_upperBand.append(middle + m_multiplier * stddev);
        m_lowerBand.append(middle - m_multiplier * stddev);
    }
    
    emit indicatorUpdated();
}

void BollingerBands::update(const AppData::MarketData &newData)
{
    // 需要维护一个数据窗口来实时更新布林带
    // 实际实现中需要保存足够的历史数据
    emit indicatorUpdated();
}

QString BollingerBands::name() const
{
    return QString("BollingerBands(%1,%2)").arg(m_period).arg(m_multiplier);
}

QVector<double> BollingerBands::values() const
{
    return m_middleBand; // 默认返回中轨值
}

double BollingerBands::lastValue() const
{
    return m_middleBand.isEmpty() ? 0.0 : m_middleBand.last();
}

QVector<double> BollingerBands::upperBand() const
{
    return m_upperBand;
}

QVector<double> BollingerBands::middleBand() const
{
    return m_middleBand;
}

QVector<double> BollingerBands::lowerBand() const
{
    return m_lowerBand;
}
