#include "Logger.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

Logger* Logger::m_instance = nullptr;

Logger::Logger(QObject* parent)
    : QObject(parent) {
    QDir logDir(getLogDir());
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    
    QString logPath = logDir.filePath(QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log");
    m_logFile = new QFile(logPath, this);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << logPath;
    }
    
    cleanupOldLogs();
    qInstallMessageHandler(qtMessageHandler);
}

Logger::~Logger() {
    m_instance = nullptr;
}

Logger* Logger::instance() {
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

void Logger::log(const QString& message, const QString& type) {
    if (!m_logFile || !m_logFile->isOpen()) return;
    
    QTextStream stream(m_logFile);
    stream << QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]") 
          << " [" << type << "] " << message << "\n";
    stream.flush();
}

void Logger::qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context);
    
    QString typeStr;
    switch (type) {
    case QtDebugMsg: typeStr = "DEBUG"; break;
    case QtInfoMsg: typeStr = "INFO"; break;
    case QtWarningMsg: typeStr = "WARNING"; break;
    case QtCriticalMsg: typeStr = "CRITICAL"; break;
    case QtFatalMsg: typeStr = "FATAL"; break;
    }
    
    Logger::instance()->log(msg, typeStr);
}

QString Logger::getLogDir() const {
    return QCoreApplication::applicationDirPath() + "/logs/" + 
           QDateTime::currentDateTime().toString("yyyy-MM");
}

void Logger::cleanupOldLogs() {
    QDir logDir(QCoreApplication::applicationDirPath() + "/logs");
    if (!logDir.exists()) return;
    
    QDateTime twoMonthsAgo = QDateTime::currentDateTime().addMonths(-2);
    
    foreach (const QFileInfo& monthDir, logDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDateTime dirDate = QDateTime::fromString(monthDir.fileName(), "yyyy-MM");
        if (dirDate.isValid() && dirDate < twoMonthsAgo) {
            QDir(monthDir.absoluteFilePath()).removeRecursively();
        }
    }
}