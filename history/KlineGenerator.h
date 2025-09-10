#ifndef KLINEGENERATOR_H
#define KLINEGENERATOR_H

#include "../AppData.h"
#include <QObject>
#include <QVector>
#include <QMap>
#include <QCache>
#include <QMutex>
#include <QDateTime>
#include <QThread>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>
#include <QScopedPointer>

/**
 * @brief K线生成器类，负责将tick级数据转换为指定周期的K线数据
 * 
 * 该类实现了高效的K线生成和缓存机制，支持快速切换不同周期的K线数据
 */
class KlineGenerator : public QObject
{
    Q_OBJECT
public:
    explicit KlineGenerator(QObject *parent = nullptr);
    ~KlineGenerator();

    /**
     * @brief 从tick数据生成指定周期的K线数据
     * @param tickData tick级别数据
     * @param timeFrame 目标时间周期
     * @param forceRegenerate 是否强制重新生成（忽略缓存）
     * @return 生成的K线数据
     */
    QVector<AppData::MarketData> generateKlineFromTicks(
        const QVector<AppData::MarketData> &tickData,
        AppData::TimeFrame timeFrame,
        bool forceRegenerate = false);

    /**
     * @brief 从低周期K线生成高周期K线数据
     * @param sourceData 源K线数据
     * @param sourceTimeFrame 源K线周期
     * @param targetTimeFrame 目标K线周期
     * @param forceRegenerate 是否强制重新生成（忽略缓存）
     * @return 生成的K线数据
     */
    QVector<AppData::MarketData> generateKlineFromKline(
        const QVector<AppData::MarketData> &sourceData,
        AppData::TimeFrame sourceTimeFrame,
        AppData::TimeFrame targetTimeFrame,
        bool forceRegenerate = false);

    /**
     * @brief 预生成并缓存多个周期的K线数据
     * @param tickData tick级别数据
     * @param timeFrames 需要预生成的时间周期列表
     */
    void preGenerateKlines(
        const QVector<AppData::MarketData> &tickData,
        const QVector<AppData::TimeFrame> &timeFrames);

    /**
     * @brief 清除指定周期的缓存数据
     * @param timeFrame 要清除的时间周期，如果为AppData::Unknown则清除所有缓存
     */
    void clearCache(AppData::TimeFrame timeFrame = AppData::KUnknown);

    /**
     * @brief 获取缓存状态
     * @return 缓存状态信息
     */
    QString getCacheStatus() const;

    /**
     * @brief 设置缓存大小（每个周期可缓存的K线数据集数量）
     * @param size 缓存大小
     */
    void setCacheSize(int size);

signals:
    /**
     * @brief K线生成进度信号
     * @param progress 进度百分比(0-100)
     * @param timeFrame 当前处理的时间周期
     */
    void generationProgress(int progress, AppData::TimeFrame timeFrame);
    
    /**
     * @brief 日志消息信号
     * @param message 日志消息
     * @param level 日志级别(0:信息, 1:警告, 2:错误)
     */
    void logMessage(const QString &message, int level = 0);

private:
    /**
     * @brief 生成缓存键
     * @param symbol 交易对符号
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param timeFrame 时间周期
     * @return 缓存键
     */
    QString generateCacheKey(
        const QString &symbol,
        const QDateTime &startTime,
        const QDateTime &endTime,
        AppData::TimeFrame timeFrame) const;

    /**
     * @brief 将tick数据聚合为K线
     * @param tickData tick数据
     * @param interval 时间间隔（秒）
     * @return 聚合后的K线数据
     */
    QVector<AppData::MarketData> aggregateTicksToKline(
        const QVector<AppData::MarketData> &tickData,
        int interval);

    /**
     * @brief 将低周期K线聚合为高周期K线
     * @param klineData 低周期K线数据
     * @param sourceInterval 源K线时间间隔（秒）
     * @param targetInterval 目标K线时间间隔（秒）
     * @return 聚合后的高周期K线数据
     */
    QVector<AppData::MarketData> aggregateKlineToHigherTimeframe(
        const QVector<AppData::MarketData> &klineData,
        int sourceInterval,
        int targetInterval);

    /**
     * @brief 获取时间周期对应的秒数
     * @param timeFrame 时间周期
     * @return 对应的秒数
     */
    int getTimeFrameSeconds(AppData::TimeFrame timeFrame) const;

    /**
     * @brief 判断高周期是否是低周期的整数倍
     * @param lowTimeFrame 低周期
     * @param highTimeFrame 高周期
     * @return 是否是整数倍
     */
    bool isMultipleTimeframe(
        AppData::TimeFrame lowTimeFrame,
        AppData::TimeFrame highTimeFrame) const;

    /**
     * @brief 对齐时间戳到指定周期的起始时间
     * @param timestamp 原始时间戳
     * @param intervalSeconds 周期秒数
     * @return 对齐后的时间戳
     */
    QDateTime alignTimestamp(
        const QDateTime &timestamp,
        int intervalSeconds) const;

private:
    // 缓存不同周期的K线数据，键为缓存键，值为K线数据
    QMap<AppData::TimeFrame, QCache<QString, QVector<AppData::MarketData>>*> m_klineCache;
    
    // 缓存大小
    int m_cacheSize;
    
    // 线程池，用于并行生成K线
    QThreadPool m_threadPool;
    
    // 互斥锁，保护缓存访问
    mutable QMutex m_cacheMutex;
};

#endif // KLINEGENERATOR_H
