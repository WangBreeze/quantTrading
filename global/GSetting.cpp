#include "GSetting.h"

GSetting* GSetting::m_instance = nullptr;

GSetting::GSetting(QObject* parent) 
    : QObject(parent) {
    m_settings = new QSettings("kquant.ini", QSettings::IniFormat, this);
}

GSetting::~GSetting() {
    m_instance = nullptr;
}

GSetting* GSetting::instance() {
    if (!m_instance) {
        m_instance = new GSetting();
    }
    return m_instance;
}

void GSetting::setValue(const QString& key, const QVariant& value) {
    m_settings->setValue(key, value);
    m_settings->sync();
}

QVariant GSetting::value(const QString& key, const QVariant& defaultValue) const {
    return m_settings->value(key, defaultValue);
}