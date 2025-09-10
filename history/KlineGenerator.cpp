#include "KlineGenerator.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <algorithm>

KlineGenerator::KlineGenerator(QObject *parent) : QObject(parent), m_cacheSize(100)
{
    // 设置线程池最大线程数
    m_threadPool.setMaxThreadCount(QThread::idealThreadCount());
    
    // 初始化各个周期的缓存
    m_klineCache[AppData::M1] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::M5] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::M15] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::M30] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::H1] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::H4] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::D1] = new QCache<QString, QVector<AppData::MarketData>>();
    m_klineCache[AppData::W1] = new QCache<QString, QVector<AppData::MarketData>>();
    
    // 设置各个缓存的最大容量
    m_klineCache[AppData::M1]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::M5]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::M15]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::M30]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::H1]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::H4]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::D1]->setMaxCost(m_cacheSize);
    m_klineCache[AppData::W1]->setMaxCost(m_cacheSize);
}

KlineGenerator::~KlineGenerator()
{
    // 等待所有线程完成
    m_threadPool.waitForDone();
    
    // 清除所有缓存并释放内存
    clearCache();
    
    // 删除所有QCache对象
    qDeleteAll(m_klineCache);
    m_klineCache.clear();
}

QVector<AppData::MarketData> KlineGenerator::generateKlineFromTicks(
    const QVector<AppData::MarketData> &tickData,
    AppData::TimeFrame timeFrame,
    bool forceRegenerate)
{
    // 如果是tick级别数据，直接返回
    if (timeFrame == AppData::Tick) {
        return tickData;
    }
    
    // 检查数据是否为空
    if (tickData.isEmpty()) {
        emit logMessage(tr("无法生成K线：tick数据为空"), 1);
        return QVector<AppData::MarketData>();
    }
    
    // 获取数据的时间范围和交易对
    QDateTime startTime = tickData.first().timestamp;
    QDateTime endTime = tickData.last().timestamp;
    QString symbol = tickData.first().symbol;
    
    // 生成缓存键
    QString cacheKey = generateCacheKey(symbol, startTime, endTime, timeFrame);
    
    // 如果不强制重新生成，尝试从缓存获取
    if (!forceRegenerate) {
        QMutexLocker locker(&m_cacheMutex);
        if (m_klineCache.contains(timeFrame) && m_klineCache[timeFrame] && m_klineCache[timeFrame]->contains(cacheKey)) {
            emit logMessage(tr(u8"从缓存获取%1周期K线数据").arg(timeFrame), 0);
            return *m_klineCache[timeFrame]->object(cacheKey);
        }
    }
    
    // 计算时间间隔（秒）
    int intervalSeconds = getTimeFrameSeconds(timeFrame);
    
    // 聚合tick数据生成K线
    QElapsedTimer timer;
    timer.start();
    
    emit logMessage(tr("开始从tick数据生成%1周期K线").arg(timeFrame), 0);
    QVector<AppData::MarketData> klineData = aggregateTicksToKline(tickData, intervalSeconds);
    
    emit logMessage(tr("生成%1周期K线完成，耗时%2毫秒，共%3条K线")
                   .arg(timeFrame)
                   .arg(timer.elapsed())
                   .arg(klineData.size()), 0);
    
    // 缓存结果
    QMutexLocker locker(&m_cacheMutex);
    if (m_klineCache.contains(timeFrame) && m_klineCache[timeFrame]) {
        m_klineCache[timeFrame]->insert(cacheKey, new QVector<AppData::MarketData>(klineData), 1);
    }
    
    return klineData;
}

QVector<AppData::MarketData> KlineGenerator::generateKlineFromKline(
    const QVector<AppData::MarketData> &sourceData,
    AppData::TimeFrame sourceTimeFrame,
    AppData::TimeFrame targetTimeFrame,
    bool forceRegenerate)
{
    // 检查源数据是否为空
    if (sourceData.isEmpty()) {
        emit logMessage(tr("无法生成K线：源数据为空"), 1);
        return QVector<AppData::MarketData>();
    }
    
    // 如果目标周期小于或等于源周期，无法直接生成
    if (targetTimeFrame <= sourceTimeFrame) {
        emit logMessage(tr("无法从低周期生成高周期K线"), 1);
        return QVector<AppData::MarketData>();
    }
    
    // 检查目标周期是否是源周期的整数倍
    if (!isMultipleTimeframe(sourceTimeFrame, targetTimeFrame)) {
        emit logMessage(tr("目标周期不是源周期的整数倍，可能导致不准确的结果"), 1);
    }
    
    // 获取数据的时间范围和交易对
    QDateTime startTime = sourceData.first().timestamp;
    QDateTime endTime = sourceData.last().timestamp;
    QString symbol = sourceData.first().symbol;
    
    // 生成缓存键
    QString cacheKey = generateCacheKey(symbol, startTime, endTime, targetTimeFrame);
    
    // 如果不强制重新生成，尝试从缓存获取
    if (!forceRegenerate) {
        QMutexLocker locker(&m_cacheMutex);
        if (m_klineCache.contains(targetTimeFrame) && m_klineCache[targetTimeFrame] && m_klineCache[targetTimeFrame]->contains(cacheKey)) {
            emit logMessage(tr("从缓存获取%1周期K线数据").arg(targetTimeFrame), 0);
            return *m_klineCache[targetTimeFrame]->object(cacheKey);
        }
    }
    
    // 计算源和目标时间间隔（秒）
    int sourceInterval = getTimeFrameSeconds(sourceTimeFrame);
    int targetInterval = getTimeFrameSeconds(targetTimeFrame);
    
    // 聚合低周期K线生成高周期K线
    QElapsedTimer timer;
    timer.start();
    
    emit logMessage(tr("开始从%1周期生成%2周期K线").arg(sourceTimeFrame).arg(targetTimeFrame), 0);
    QVector<AppData::MarketData> klineData = aggregateKlineToHigherTimeframe(
        sourceData, sourceInterval, targetInterval);
    
    emit logMessage(tr("生成%1周期K线完成，耗时%2毫秒，共%3条K线")
                   .arg(targetTimeFrame)
                   .arg(timer.elapsed())
                   .arg(klineData.size()), 0);
    
    // 缓存结果
    QMutexLocker locker(&m_cacheMutex);
    if (m_klineCache.contains(targetTimeFrame) && m_klineCache[targetTimeFrame]) {
        m_klineCache[targetTimeFrame]->insert(cacheKey, new QVector<AppData::MarketData>(klineData), 1);
    }
    
    return klineData;
}

void KlineGenerator::preGenerateKlines(
    const QVector<AppData::MarketData> &tickData,
    const QVector<AppData::TimeFrame> &timeFrames)
{
    if (tickData.isEmpty() || timeFrames.isEmpty()) {
        return;
    }
    
    emit logMessage(tr("开始预生成多个周期的K线数据"), 0);
    
    // 创建任务列表
    QList<QFuture<void>> tasks;
    
    // 为每个周期创建一个任务
    for (const auto &timeFrame : timeFrames) {
        if (timeFrame == AppData::Tick) {
            continue; // 跳过tick级别
        }
        
        // 使用QtConcurrent并行处理
        QFuture<void> future = QtConcurrent::run(&m_threadPool, [this, tickData, timeFrame]() {
            generateKlineFromTicks(tickData, timeFrame, true);
            emit generationProgress(100, timeFrame);
        });
        
        tasks.append(future);
    }
    
    // 等待所有任务完成（非阻塞，可以在UI线程中调用）
    for (int i = 0; i < tasks.size(); ++i) {
        tasks[i].waitForFinished();
    }
    
    emit logMessage(tr("所有周期K线数据预生成完成"), 0);
}

void KlineGenerator::clearCache(AppData::TimeFrame timeFrame)
{
    QMutexLocker locker(&m_cacheMutex);
    
    if (timeFrame == AppData::KUnknown) {
        // 清除所有缓存
        for (auto it = m_klineCache.begin(); it != m_klineCache.end(); ++it) {
            if (it.value()) {
                it.value()->clear();
            }
        }
        emit logMessage(tr("已清除所有K线缓存"), 0);
    } else {
        // 清除指定周期的缓存
        if (m_klineCache.contains(timeFrame) && m_klineCache[timeFrame]) {
            m_klineCache[timeFrame]->clear();
            emit logMessage(tr("已清除%1周期K线缓存").arg(timeFrame), 0);
        }
    }
}

QString KlineGenerator::getCacheStatus() const
{
    QMutexLocker locker(&m_cacheMutex);
    
    QString status = tr("K线缓存状态:\n");
    
    for (auto it = m_klineCache.begin(); it != m_klineCache.end(); ++it) {
        AppData::TimeFrame tf = it.key();
        if (it.value()) {
            int count = it.value()->count();
            int maxCount = it.value()->maxCost();
            status += tr("  %1周期: %2/%3\n").arg(tf).arg(count).arg(maxCount);
        } else {
            status += tr("  %1周期: 未初始化\n").arg(tf);
        }
    }
    
    return status;
}

void KlineGenerator::setCacheSize(int size)
{
    if (size <= 0) {
        return;
    }
    
    QMutexLocker locker(&m_cacheMutex);
    m_cacheSize = size;
    
    // 更新所有缓存的大小
    for (auto it = m_klineCache.begin(); it != m_klineCache.end(); ++it) {
        it.value()->setMaxCost(size);
    }
    
    emit logMessage(tr("K线缓存大小已设置为%1").arg(size), 0);
}

QString KlineGenerator::generateCacheKey(
    const QString &symbol,
    const QDateTime &startTime,
    const QDateTime &endTime,
    AppData::TimeFrame timeFrame) const
{
    // 生成唯一的缓存键
    QString keyStr = QString("%1_%2_%3_%4")
                    .arg(symbol)
                    .arg(startTime.toString(Qt::ISODate))
                    .arg(endTime.toString(Qt::ISODate))
                    .arg(timeFrame);
    
    // 使用MD5哈希生成固定长度的键
    QByteArray hash = QCryptographicHash::hash(keyStr.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

QVector<AppData::MarketData> KlineGenerator::aggregateTicksToKline(
    const QVector<AppData::MarketData> &tickData,
    int intervalSeconds)
{
    QVector<AppData::MarketData> result;
    
    if (tickData.isEmpty()) {
        return result;
    }
    
    // 获取第一个tick的时间和交易对
    QDateTime firstTickTime = tickData.first().timestamp;
    QString symbol = tickData.first().symbol;
    
    // 对齐第一个K线的开始时间
    QDateTime currentKlineStart = alignTimestamp(firstTickTime, intervalSeconds);
    QDateTime currentKlineEnd = currentKlineStart.addSecs(intervalSeconds);
    
    AppData::MarketData currentKline;
    bool klineStarted = false;
    
    // 遍历所有tick数据
    for (const auto &tick : tickData) {
        // 如果tick时间超出当前K线的结束时间，保存当前K线并开始新的K线
        while (tick.timestamp >= currentKlineEnd) {
            // 保存已完成的K线
            if (klineStarted) {
                result.append(currentKline);
                emit generationProgress(
                    static_cast<int>((currentKlineEnd.toMSecsSinceEpoch() - firstTickTime.toMSecsSinceEpoch()) * 100.0 / 
                                    (tickData.last().timestamp.toMSecsSinceEpoch() - firstTickTime.toMSecsSinceEpoch())),
                    static_cast<AppData::TimeFrame>(intervalSeconds)
                );
            }
            
            // 开始新的K线
            currentKlineStart = currentKlineEnd;
            currentKlineEnd = currentKlineStart.addSecs(intervalSeconds);
            klineStarted = false;
        }
        
        // 如果tick时间在当前K线范围内
        if (tick.timestamp >= currentKlineStart && tick.timestamp < currentKlineEnd) {
            if (!klineStarted) {
                // 初始化新K线
                currentKline = AppData::MarketData();
                currentKline.symbol = symbol;
                currentKline.timestamp = currentKlineStart;
                currentKline.open = tick.close;
                currentKline.high = tick.close;
                currentKline.low = tick.close;
                currentKline.volume = 0;
                currentKline.amount = 0;
                klineStarted = true;
            }
            
            // 更新K线数据
            currentKline.high = qMax(currentKline.high, tick.close);
            currentKline.low = qMin(currentKline.low, tick.close);
            currentKline.close = tick.close;
            currentKline.volume += tick.volume;
            currentKline.amount += tick.amount;
            
            // 更新买卖盘数据（累计最后一个tick的买卖盘数据）
            if (tick.bidPrice > 0) {
                currentKline.bidPrice = tick.bidPrice;
                currentKline.bidVolume += tick.bidVolume;
            }
            if (tick.askPrice > 0) {
                currentKline.askPrice = tick.askPrice;
                currentKline.askVolume += tick.askVolume;
            }
        }
    }
    
    // 保存最后一个未完成的K线
    if (klineStarted) {
        result.append(currentKline);
    }
    
    return result;
}

QVector<AppData::MarketData> KlineGenerator::aggregateKlineToHigherTimeframe(
    const QVector<AppData::MarketData> &klineData,
    int sourceInterval,
    int targetInterval)
{
    QVector<AppData::MarketData> result;
    
    if (klineData.isEmpty()) {
        return result;
    }
    
    // 获取第一个K线的时间和交易对
    QDateTime firstKlineTime = klineData.first().timestamp;
    QString symbol = klineData.first().symbol;
    
    // 对齐第一个高周期K线的开始时间
    QDateTime currentKlineStart = alignTimestamp(firstKlineTime, targetInterval);
    QDateTime currentKlineEnd = currentKlineStart.addSecs(targetInterval);
    
    AppData::MarketData currentKline;
    bool klineStarted = false;
    
    // 遍历所有低周期K线数据
    for (const auto &kline : klineData) {
        // 如果K线时间超出当前高周期K线的结束时间，保存当前K线并开始新的K线
        while (kline.timestamp >= currentKlineEnd) {
            // 保存已完成的K线
            if (klineStarted) {
                result.append(currentKline);
                emit generationProgress(
                    static_cast<int>((currentKlineEnd.toMSecsSinceEpoch() - firstKlineTime.toMSecsSinceEpoch()) * 100.0 / 
                                    (klineData.last().timestamp.toMSecsSinceEpoch() - firstKlineTime.toMSecsSinceEpoch())),
                    static_cast<AppData::TimeFrame>(targetInterval)
                );
            }
            
            // 开始新的K线
            currentKlineStart = currentKlineEnd;
            currentKlineEnd = currentKlineStart.addSecs(targetInterval);
            klineStarted = false;
        }
        
        // 如果K线时间在当前高周期K线范围内
        if (kline.timestamp >= currentKlineStart && kline.timestamp < currentKlineEnd) {
            if (!klineStarted) {
                // 初始化新K线
                currentKline = AppData::MarketData();
                currentKline.symbol = symbol;
                currentKline.timestamp = currentKlineStart;
                currentKline.open = kline.open;
                currentKline.high = kline.high;
                currentKline.low = kline.low;
                currentKline.volume = 0;
                currentKline.amount = 0;
                klineStarted = true;
            }
            
            // 更新K线数据
            currentKline.high = qMax(currentKline.high, kline.high);
            currentKline.low = qMin(currentKline.low, kline.low);
            currentKline.close = kline.close;
            currentKline.volume += kline.volume;
            currentKline.amount += kline.amount;
            
            // 更新买卖盘数据（累计最后一个K线的买卖盘数据）
            if (kline.bidPrice > 0) {
                currentKline.bidPrice = kline.bidPrice;
                currentKline.bidVolume += kline.bidVolume;
            }
            if (kline.askPrice > 0) {
                currentKline.askPrice = kline.askPrice;
                currentKline.askVolume += kline.askVolume;
            }
        }
    }
    
    // 保存最后一个未完成的K线
    if (klineStarted) {
        result.append(currentKline);
    }
    
    return result;
}

int KlineGenerator::getTimeFrameSeconds(AppData::TimeFrame timeFrame) const
{
    switch (timeFrame) {
        case AppData::Tick: return 1; // 1秒
        case AppData::M1: return 60; // 1分钟
        case AppData::M5: return 300; // 5分钟
        case AppData::M15: return 900; // 15分钟
        case AppData::M30: return 1800; // 30分钟
        case AppData::H1: return 3600; // 1小时
        case AppData::H4: return 14400; // 4小时
        case AppData::D1: return 86400; // 1天
        case AppData::W1: return 604800; // 1周
        default: return 60; // 默认1分钟
    }
}

bool KlineGenerator::isMultipleTimeframe(
    AppData::TimeFrame lowTimeFrame,
    AppData::TimeFrame highTimeFrame) const
{
    int lowSeconds = getTimeFrameSeconds(lowTimeFrame);
    int highSeconds = getTimeFrameSeconds(highTimeFrame);
    
    return (highSeconds % lowSeconds) == 0;
}

QDateTime KlineGenerator::alignTimestamp(
    const QDateTime &timestamp,
    int intervalSeconds) const
{
    // 将时间戳对齐到周期的起始时间
    qint64 msecs = timestamp.toMSecsSinceEpoch();
    qint64 intervalMsecs = intervalSeconds * 1000LL;
    qint64 alignedMsecs = (msecs / intervalMsecs) * intervalMsecs;
    
    return QDateTime::fromMSecsSinceEpoch(alignedMsecs);
}
