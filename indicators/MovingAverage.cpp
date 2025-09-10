#include "MovingAverage.h"
#include "../AppData.h"

MovingAverage::MovingAverage(int period, MAType maType, QObject *parent)
    : IndicatorBase(AppData::MA, parent), m_period(period), m_maType(maType)
{
}

void MovingAverage::calculate(const QVector<AppData::MarketData> &data)
{
    m_values.clear();
    if (data.size() < m_period) return;

    switch (m_maType) {
    case SMA: {
        for (int i = m_period - 1; i < data.size(); ++i) {
            double sum = 0.0;
            for (int j = 0; j < m_period; ++j) {
                sum += data[i - j].close;
            }
            m_values.append(sum / m_period);
        }
        break;
    }
    case EMA: {
        double multiplier = 2.0 / (m_period + 1);
        double ema = data[0].close;
        m_values.append(ema);
        
        for (int i = 1; i < data.size(); ++i) {
            ema = (data[i].close - ema) * multiplier + ema;
            m_values.append(ema);
        }
        break;
    }
    case WMA: {
        double weightSum = m_period * (m_period + 1) / 2.0;
        for (int i = m_period - 1; i < data.size(); ++i) {
            double sum = 0.0;
            for (int j = 0; j < m_period; ++j) {
                sum += data[i - j].close * (j + 1);
            }
            m_values.append(sum / weightSum);
        }
        break;
    }
    }
    
    emit indicatorUpdated();
}

void MovingAverage::update(const AppData::MarketData &newData)
{
    if (m_values.isEmpty()) {
        m_values.append(newData.close);
        return;
    }

    switch (m_maType) {
    case SMA: {
        // 需要维护一个数据窗口来更新SMA
        break;
    }
    case EMA: {
        double multiplier = 2.0 / (m_period + 1);
        double lastEma = m_values.last();
        double newEma = (newData.close - lastEma) * multiplier + lastEma;
        m_values.append(newEma);
        break;
    }
    case WMA: {
        // 需要维护一个数据窗口来更新WMA
        break;
    }
    }
    
    emit indicatorUpdated();
}

QString MovingAverage::name() const
{
    switch (m_maType) {
    case SMA: return QString("SMA(%1)").arg(m_period);
    case EMA: return QString("EMA(%1)").arg(m_period);
    case WMA: return QString("WMA(%1)").arg(m_period);
    }
    return QString();
}

QVector<double> MovingAverage::values() const
{
    return m_values;
}

double MovingAverage::lastValue() const
{
    return m_values.isEmpty() ? 0.0 : m_values.last();
}
