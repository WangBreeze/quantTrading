#include "IndicatorBase.h"

IndicatorBase::IndicatorBase(AppData::IndicatorType type, QObject *parent)
    : QObject(parent), m_type(type)
{
}

IndicatorBase::~IndicatorBase()
{
}

AppData::IndicatorType IndicatorBase::type() const
{
    return m_type;
}
