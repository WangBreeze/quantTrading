#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QtGlobal>

class Logger : public QObject {
    Q_OBJECT
public:
    static Logger* instance();
    
    void log(const QString& message, const QString& type = "INFO");
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    
private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();
    
    void cleanupOldLogs();
    QString getLogDir() const;
    
    QFile* m_logFile;
    static Logger* m_instance;
};

#endif // LOGGER_H