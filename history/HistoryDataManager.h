#ifndef HISTORYDATAMANAGER_H
#define HISTORYDATAMANAGER_H

#include "../AppData.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDir>

// 历史数据管理器，负责获取和管理历史数据
class HistoryDataManager : public QObject
{
    Q_OBJECT
public:
    explicit HistoryDataManager(QObject *parent = nullptr);
    ~HistoryDataManager();

    // 设置数据存储目录
    void setDataDir(const QString &dirPath);
    QString getDataDir() const;

    // 加载历史数据
    bool loadHistoricalData(const QString &symbol, 
                           const QDateTime &startTime, 
                           const QDateTime &endTime,
                           QVector<AppData::MarketData> &data,
                           AppData::TimeFrame timeFrame = AppData::Tick);

    // 保存历史数据
    bool saveHistoricalData(const QString &symbol, 
                           const QVector<AppData::MarketData> &data,
                           AppData::TimeFrame timeFrame = AppData::Tick);

    // 获取数据文件路径
    QString getDataFilePath(const QString &symbol, 
                           AppData::TimeFrame timeFrame = AppData::Tick) const;

    // 检查数据是否存在
    bool hasHistoricalData(const QString &symbol, 
                         AppData::TimeFrame timeFrame = AppData::Tick) const;

    // 从CSV文件加载数据
    static bool loadFromCsv(const QString &filePath, 
                           QVector<AppData::MarketData> &data);

    // 保存数据到CSV文件
    static bool saveToCsv(const QString &filePath, 
                         const QVector<AppData::MarketData> &data);

    // 从东方财富获取A股历史数据
    bool fetchEastMoneyData(const QString &symbol, 
                           const QDateTime &startTime, 
                           const QDateTime &endTime,
                           QVector<AppData::MarketData> &data);

    // 从欧易获取加密货币历史数据
    bool fetchOKXData(const QString &symbol, 
                     const QDateTime &startTime, 
                     const QDateTime &endTime,
                     QVector<AppData::MarketData> &data);

    // 从币安获取加密货币历史数据
    bool fetchBinanceData(const QString &symbol, 
                         const QDateTime &startTime, 
                         const QDateTime &endTime,
                         QVector<AppData::MarketData> &data);

signals:
    void progressUpdated(int progress);
    void logMessage(const QString &message, int level = 0);

private:
    QString m_dataDir; // 数据存储目录
};

#endif // HISTORYDATAMANAGER_H
