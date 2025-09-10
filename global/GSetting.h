#ifndef GSETTING_H
#define GSETTING_H

#include <QObject>
#include <QSettings>

class GSetting : public QObject {
    Q_OBJECT
public:
    static GSetting* instance();
    
    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    
private:
    explicit GSetting(QObject* parent = nullptr);
    ~GSetting();
    
    QSettings* m_settings;
    static GSetting* m_instance;
};

#endif // GSETTING_H