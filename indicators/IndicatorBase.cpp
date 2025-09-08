#include "IndicatorBase.h"

IndicatorBase::IndicatorBase(IndicatorType type, QObject *parent)
    : QObject(parent), m_type(type)
{
}

IndicatorBase::~IndicatorBase()
{
}

IndicatorBase::IndicatorType IndicatorBase::type() const
{
    return m_type;
}