#include <QtNetwork/QNetworkAccessManager>
#include <QEventLoop>
#include <QUrlQuery>
#include <QtNetwork/QNetworkReply>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QDateTime>
#include <QCryptographicHash>
#include <QThread>

#include "HistoryDataManager.h"

// 构造函数
HistoryDataManager::HistoryDataManager(QObject *parent) : QObject(parent)
{
    // 默认设置数据目录为D:/data
    m_dataDir = "D:/data";
    
    // 确保目录存在
    QDir dir(m_dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

// 析构函数
HistoryDataManager::~HistoryDataManager()
{
}

// 设置数据存储目录
void HistoryDataManager::setDataDir(const QString &dirPath)
{
    m_dataDir = dirPath;
    
    // 确保目录存在
    QDir dir(m_dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

// 获取数据存储目录
QString HistoryDataManager::getDataDir() const
{
    return m_dataDir;
}

// 获取数据文件路径
QString HistoryDataManager::getDataFilePath(const QString &symbol, AppData::TimeFrame timeFrame) const
{
    // 创建基础目录结构
    QString timeFrameStr;
    switch (timeFrame) {
        case AppData::Tick: timeFrameStr = "tick"; break;
        case AppData::M1: timeFrameStr = "1m"; break;
        case AppData::M5: timeFrameStr = "5m"; break;
        case AppData::M15: timeFrameStr = "15m"; break;
        case AppData::M30: timeFrameStr = "30m"; break;
        case AppData::H1: timeFrameStr = "1h"; break;
        case AppData::H4: timeFrameStr = "4h"; break;
        case AppData::D1: timeFrameStr = "1d"; break;
        case AppData::W1: timeFrameStr = "1w"; break;
        default: timeFrameStr = "unknown"; break;
    }
    
    // 创建目录结构: D:/data/[timeframe]/[symbol]/
    QString dirPath = QString("%1/%2/%3").arg(m_dataDir, timeFrameStr, symbol);
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 返回最新的数据文件路径
    // 注意：实际文件名会在saveHistoricalData中根据时间范围动态生成
    return dirPath;
}

// 检查数据是否存在
bool HistoryDataManager::hasHistoricalData(const QString &symbol, AppData::TimeFrame timeFrame) const
{
    QString dirPath = getDataFilePath(symbol, timeFrame);
    QDir dir(dirPath);
    
    // 检查目录是否存在且不为空
    return dir.exists() && !dir.entryList(QDir::Files).isEmpty();
}

// 从CSV文件加载数据
bool HistoryDataManager::loadFromCsv(const QString &filePath, QVector<AppData::MarketData> &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    
    // 读取CSV头
    QString header = in.readLine();
    
    // 读取数据行
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        
        // 确保字段数量正确
        if (fields.size() < 8) continue;
        
        AppData::MarketData marketData;
        
        // 解析CSV数据
        marketData.symbol = fields[0];
        marketData.timestamp = QDateTime::fromString(fields[1], Qt::ISODate);
        marketData.open = fields[2].toDouble();
        marketData.high = fields[3].toDouble();
        marketData.low = fields[4].toDouble();
        marketData.close = fields[5].toDouble();
        marketData.volume = fields[6].toDouble();
        marketData.amount = fields[7].toDouble();
        
        // 如果有买卖盘数据
        if (fields.size() >= 11) {
            marketData.bidPrice = fields[8].toDouble();
            marketData.bidVolume = fields[9].toDouble();
            marketData.askPrice = fields[10].toDouble();
            marketData.askVolume = fields[11].toDouble();
        }
        
        data.append(marketData);
    }
    
    file.close();
    return true;
}

// 保存数据到CSV文件
bool HistoryDataManager::saveToCsv(const QString &filePath, const QVector<AppData::MarketData> &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // 写入CSV头
    out << "Symbol,Timestamp,Open,High,Low,Close,Volume,Amount,BidPrice,BidVolume,AskPrice,AskVolume";
    
    // 写入数据行
    for (const auto &marketData : data) {
        out << marketData.symbol << ","
            << marketData.timestamp.toString(Qt::ISODate) << ","
            << QString::number(marketData.open, 'f', 8) << ","
            << QString::number(marketData.high, 'f', 8) << ","
            << QString::number(marketData.low, 'f', 8) << ","
            << QString::number(marketData.close, 'f', 8) << ","
            << QString::number(marketData.volume, 'f', 8) << ","
            << QString::number(marketData.amount, 'f', 8) << ","
            << QString::number(marketData.bidPrice, 'f', 8) << ","
            << QString::number(marketData.bidVolume, 'f', 8) << ","
            << QString::number(marketData.askPrice, 'f', 8) << ","
            << QString::number(marketData.askVolume, 'f', 8) << "";
    }
    
    file.close();
    return true;
}

// 加载历史数据
bool HistoryDataManager::loadHistoricalData(const QString &symbol, 
                                          const QDateTime &startTime, 
                                          const QDateTime &endTime,
                                          QVector<AppData::MarketData> &data,
                                          AppData::TimeFrame timeFrame)
{
    // 获取数据目录
    QString dirPath = getDataFilePath(symbol, timeFrame);
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        emit logMessage(tr("数据目录不存在: %1").arg(dirPath), 1);
        return false;
    }
    
    // 获取目录中的所有CSV文件
    QStringList filters;
    filters << "*.csv";
    dir.setNameFilters(filters);
    QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
    
    if (files.isEmpty()) {
        emit logMessage(tr("没有找到数据文件"), 1);
        return false;
    }
    
    // 遍历所有文件，加载符合时间范围的数据
    bool foundData = false;
    int totalLoaded = 0;
    
    for (const QString &fileName : files) {
        // 从文件名解析时间范围
        QStringList parts = fileName.split('_');
        if (parts.size() < 3) continue;
        
        QDateTime fileStartTime = QDateTime::fromString(parts[1], "yyyyMMdd-HHmmss");
        QDateTime fileEndTime = QDateTime::fromString(parts[2].split('.')[0], "yyyyMMdd-HHmmss");
        
        // 检查文件时间范围是否与请求的时间范围有重叠
        if (fileEndTime < startTime || fileStartTime > endTime) {
            continue;
        }
        
        // 加载数据
        QVector<AppData::MarketData> fileData;
        QString filePath = dir.filePath(fileName);
        
        if (loadFromCsv(filePath, fileData)) {
            // 过滤时间范围内的数据
            for (const auto &marketData : fileData) {
                if (marketData.timestamp >= startTime && marketData.timestamp <= endTime) {
                    data.append(marketData);
                    totalLoaded++;
                }
            }
            
            foundData = true;
            emit logMessage(tr("从文件加载了 %1 条数据: %2").arg(fileData.size()).arg(fileName), 0);
        }
    }
    
    if (foundData) {
        emit logMessage(tr(u8"总共加载了 %1 条数据").arg(totalLoaded), 0);
        
        // 按时间排序
        std::sort(data.begin(), data.end(), [](const AppData::MarketData &a, const AppData::MarketData &b) {
            return a.timestamp < b.timestamp;
        });
        
        return true;
    } else {
        emit logMessage(tr("没有找到符合时间范围的数据"), 1);
        return false;
    }
}

// 保存历史数据
bool HistoryDataManager::saveHistoricalData(const QString &symbol, 
                                          const QVector<AppData::MarketData> &data,
                                          AppData::TimeFrame timeFrame)
{
    if (data.isEmpty()) {
        emit logMessage(tr("没有数据需要保存"), 1);
        return false;
    }
    
    // 获取数据的时间范围
    QDateTime startTime = data.first().timestamp;
    QDateTime endTime = data.last().timestamp;
    
    // 获取数据目录
    QString dirPath = getDataFilePath(symbol, timeFrame);
    
    // 按照时间范围分割数据，确保每个文件不超过1GB
    const qint64 MAX_FILE_SIZE = 1024 * 1024 * 1024; // 1GB
    const int APPROX_RECORD_SIZE = 200; // 每条记录大约占用的字节数
    const int MAX_RECORDS_PER_FILE = MAX_FILE_SIZE / APPROX_RECORD_SIZE;
    
    // 分割数据
    for (int i = 0; i < data.size(); i += MAX_RECORDS_PER_FILE) {
        // 计算当前批次的结束索引
        int endIndex = qMin(i + MAX_RECORDS_PER_FILE, data.size());
        
        // 提取当前批次的数据
        QVector<AppData::MarketData> batchData;
        for (int j = i; j < endIndex; j++) {
            batchData.append(data[j]);
        }
        
        if (batchData.isEmpty()) continue;
        
        // 获取当前批次的时间范围
        QDateTime batchStartTime = batchData.first().timestamp;
        QDateTime batchEndTime = batchData.last().timestamp;
        
        // 生成文件名: symbol_starttime_endtime.csv
        QString fileName = QString("%1_%2_%3.csv")
                            .arg(symbol)
                            .arg(batchStartTime.toString("yyyyMMdd-HHmmss"))
                            .arg(batchEndTime.toString("yyyyMMdd-HHmmss"));
        
        QString filePath = dirPath + "/" + fileName;
        
        // 保存数据到CSV文件
        if (saveToCsv(filePath, batchData)) {
            emit logMessage(tr("保存了 %1 条数据到文件: %2").arg(batchData.size()).arg(fileName), 0);
        } else {
            emit logMessage(tr("保存数据到文件失败: %1").arg(fileName), 2);
            return false;
        }
    }
    
    return true;
}

// 从东方财富获取A股历史数据 (tick级别)
bool HistoryDataManager::fetchEastMoneyData(const QString &symbol, 
                                          const QDateTime &startTime, 
                                          const QDateTime &endTime,
                                          QVector<AppData::MarketData> &data)
{
    emit logMessage(tr("开始获取东方财富A股tick数据: %1").arg(symbol));
    
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    
    // 东方财富API基础URL (逐笔交易API)
    const QString baseUrl = "http://push2ex.eastmoney.com/getStockFenShi";
    
    // 解析股票代码，格式为：sh600000或sz000001
    QString market = symbol.left(2).toLower();
    QString code = symbol.mid(2);
    QString marketId;
    
    if (market == "sh") {
        marketId = "1";
    } else if (market == "sz") {
        marketId = "0";
    } else {
        emit logMessage(tr("不支持的股票代码格式: %1").arg(symbol), 2);
        return false;
    }
    
    // 计算时间范围（每次获取一天的tick数据）
    QDate currentDate = startTime.date();
    QDate endDate = endTime.date();
    int totalFetched = 0;
    bool success = true;
    
    while (currentDate <= endDate && success) {
        // 构建请求URL
        QUrl url(baseUrl);
        QUrlQuery query;
        
        // 设置请求参数
        query.addQueryItem("pagesize", "3000"); // 每页最多3000条记录
        query.addQueryItem("page", "0");
        query.addQueryItem("market", marketId);
        query.addQueryItem("code", code);
        query.addQueryItem("date", currentDate.toString("yyyyMMdd"));
        
        url.setQuery(query);
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        // 发送请求
        QNetworkReply *reply = manager.get(request);
        loop.exec(); // 等待请求完成
        
        // 检查响应
        if (reply->error() != QNetworkReply::NoError) {
            emit logMessage(tr("东方财富API请求失败: %1").arg(reply->errorString()), 2);
            success = false;
            reply->deleteLater();
            break;
        }
        
        // 解析响应数据
        QByteArray responseData = reply->readAll();
        reply->deleteLater();
        
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit logMessage(tr("东方财富API响应解析失败: %1").arg(parseError.errorString()), 2);
            success = false;
            break;
        }
        
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj["rc"].toInt() != 0) {
            emit logMessage(tr("东方财富API返回错误: %1").arg(rootObj["msg"].toString()), 2);
            success = false;
            break;
        }
        
        QJsonObject dataObj = rootObj["data"].toObject();
        QJsonArray ticksArray = dataObj["data"].toArray();
        
        if (ticksArray.isEmpty()) {
            emit logMessage(tr("没有获取到数据，日期: %1").arg(currentDate.toString("yyyy-MM-dd")), 1);
            // 继续下一天
            currentDate = currentDate.addDays(1);
            continue;
        }
        
        // 解析tick数据
        for (const auto &item : ticksArray) {
            QJsonObject tick = item.toObject();
            if (tick.isEmpty()) continue;
            
            AppData::MarketData marketData;
            marketData.symbol = symbol;
            
            // 解析时间戳 (格式为 HHmmss)
            QString timeStr = tick["time"].toString();
            QTime time = QTime::fromString(timeStr, "HHmmss");
            marketData.timestamp = QDateTime(currentDate, time);
            
            // 只处理时间范围内的数据
            if (marketData.timestamp < startTime || marketData.timestamp > endTime) {
                continue;
            }
            
            // 解析价格和成交量
            marketData.close = tick["price"].toDouble(); // 成交价
            marketData.volume = tick["volume"].toDouble(); // 成交量
            marketData.amount = tick["amount"].toDouble(); // 成交额
            
            // 对于tick数据，open/high/low都等于close
            marketData.open = marketData.close;
            marketData.high = marketData.close;
            marketData.low = marketData.close;
            
            // 获取买卖盘数据
            int bs = tick["bs"].toInt(); // 1为买，2为卖
            if (bs == 1) {
                marketData.bidPrice = marketData.close;
                marketData.bidVolume = marketData.volume;
            } else if (bs == 2) {
                marketData.askPrice = marketData.close;
                marketData.askVolume = marketData.volume;
            }
            
            data.append(marketData);
            totalFetched++;
        }
        
        // 更新进度
        emit logMessage(tr("已获取%1条A股tick数据，日期: %2").arg(totalFetched).arg(currentDate.toString("yyyy-MM-dd")), 0);
        emit progressUpdated(static_cast<int>((currentDate.toJulianDay() - startTime.date().toJulianDay()) * 100.0 / 
                                            (endDate.toJulianDay() - startTime.date().toJulianDay() + 1)));
        
        // 进入下一天
        currentDate = currentDate.addDays(1);
        
        // 避免请求过于频繁
        QThread::msleep(500);
    }
    
    emit logMessage(tr("总共获取%1条A股tick数据").arg(totalFetched), 0);
    return success && totalFetched > 0;
}

// 从币安获取加密货币历史数据 (tick级别)
bool HistoryDataManager::fetchBinanceData(const QString &symbol, 
                                        const QDateTime &startTime, 
                                        const QDateTime &endTime,
                                        QVector<AppData::MarketData> &data)
{
    emit logMessage(tr("开始获取币安加密货币tick数据: %1").arg(symbol));
    
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    
    // 币安API基础URL
    const QString baseUrl = "https://api.binance.com";
    
    // 计算时间范围（币安API限制每次最多获取1000条交易记录）
    qint64 startTimestamp = startTime.toMSecsSinceEpoch();
    qint64 endTimestamp = endTime.toMSecsSinceEpoch();
    int totalFetched = 0;
    bool success = true;
    
    // 每次请求的时间间隔 (1小时)
    const qint64 interval = 1000 * 60 * 60; // 1小时的毫秒数
    qint64 currentStart = startTimestamp;
    
    while (currentStart < endTimestamp && success) {
        qint64 currentEnd = currentStart + interval;
        if (currentEnd > endTimestamp) {
            currentEnd = endTimestamp;
        }
        
        // 构建请求URL - 使用历史交易记录接口
        QUrl url(baseUrl + "/api/v3/aggTrades");
        QUrlQuery query;
        query.addQueryItem("symbol", symbol);
        query.addQueryItem("startTime", QString::number(currentStart));
        query.addQueryItem("endTime", QString::number(currentEnd));
        query.addQueryItem("limit", "1000"); // 每次最多1000条
        url.setQuery(query);
        
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        // 发送请求
        QNetworkReply *reply = manager.get(request);
        loop.exec(); // 等待请求完成
        
        // 检查响应
        if (reply->error() != QNetworkReply::NoError) {
            emit logMessage(tr("币安API请求失败: %1").arg(reply->errorString()), 2);
            success = false;
            reply->deleteLater();
            break;
        }
        
        // 解析响应数据
        QByteArray responseData = reply->readAll();
        reply->deleteLater();
        
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit logMessage(tr("币安API响应解析失败: %1").arg(parseError.errorString()), 2);
            success = false;
            break;
        }
        
        QJsonArray tradesArray = jsonDoc.array();
        if (tradesArray.isEmpty()) {
            emit logMessage(tr("没有获取到数据"), 1);
            // 继续下一个时间段
            currentStart = currentEnd + 1;
            continue;
        }
        
        // 解析交易数据
        for (const auto &item : tradesArray) {
            QJsonObject trade = item.toObject();
            if (trade.isEmpty()) continue;
            
            AppData::MarketData marketData;
            marketData.symbol = symbol;
            
            // 解析时间戳
            qint64 timestamp = trade["T"].toVariant().toLongLong();
            marketData.timestamp = QDateTime::fromMSecsSinceEpoch(timestamp);
            
            // 只处理时间范围内的数据
            if (marketData.timestamp < startTime || marketData.timestamp > endTime) {
                continue;
            }
            
            // 解析价格和成交量
            marketData.close = trade["p"].toString().toDouble(); // 成交价
            marketData.volume = trade["q"].toString().toDouble(); // 成交量
            marketData.amount = marketData.close * marketData.volume; // 成交额
            
            // 对于tick数据，open/high/low都等于close
            marketData.open = marketData.close;
            marketData.high = marketData.close;
            marketData.low = marketData.close;
            
            // 获取买卖盘数据
            bool isBuyerMaker = trade["m"].toBool();
            if (isBuyerMaker) {
                marketData.askPrice = marketData.close;
                marketData.askVolume = marketData.volume;
            } else {
                marketData.bidPrice = marketData.close;
                marketData.bidVolume = marketData.volume;
            }
            
            data.append(marketData);
            totalFetched++;
        }
        
        // 更新进度
        emit logMessage(tr("已获取%1条币安tick数据").arg(totalFetched), 0);
        emit progressUpdated(static_cast<int>((currentEnd - startTimestamp) * 100.0 / (endTimestamp - startTimestamp)));
        
        // 更新起始时间
        currentStart = currentEnd + 1; // 加1毫秒避免重复
        
        // 避免请求过于频繁
        QThread::msleep(300);
    }
    
    emit logMessage(tr("总共获取%1条币安tick数据").arg(totalFetched), 0);
    return success && totalFetched > 0;
}


bool HistoryDataManager::fetchOKXData(const QString &symbol, 
                                    const QDateTime &startTime, 
                                    const QDateTime &endTime,
                                    QVector<AppData::MarketData> &data)
{
    // 欧易API获取加密货币tick数据
    emit logMessage(tr("开始获取欧易加密货币tick数据: %1").arg(symbol));
    
    QNetworkAccessManager manager;
    QEventLoop loop;
    QObject::connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    // 欧易API基础URL
    const QString baseUrl = "https://www.okx.com";
    
    // 计算时间范围（欧易API限制每次最多获取100条tick数据）
    QDateTime currentStart = startTime;
    int totalFetched = 0;
    bool success = true;

    while (currentStart < endTime && success) {
        QDateTime currentEnd = currentStart.addSecs(60); // 每次获取1分钟内的tick数据
        if (currentEnd > endTime) {
            currentEnd = endTime;
        }

        // 构建请求URL - 使用tick数据接口
        QUrl url(baseUrl + "/api/v5/market/history-trades");
        QUrlQuery query;
        query.addQueryItem("instId", symbol);
        query.addQueryItem("after", QString::number(currentStart.toMSecsSinceEpoch()));
        query.addQueryItem("before", QString::number(currentEnd.toMSecsSinceEpoch()));
        query.addQueryItem("limit", "100"); // 每次最多100条tick
        url.setQuery(query);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        // 发送请求
        QNetworkReply *reply = manager.get(request);
        loop.exec(); // 等待请求完成

        // 检查响应
        if (reply->error() != QNetworkReply::NoError) {
            emit logMessage(tr("欧易API请求失败: %1").arg(reply->errorString()), 2);
            success = false;
            reply->deleteLater();
            break;
        }

        // 解析响应数据
        QByteArray responseData = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit logMessage(tr("欧易API响应解析失败: %1").arg(parseError.errorString()), 2);
            success = false;
            break;
        }

        QJsonObject rootObj = jsonDoc.object();
        if (rootObj["code"].toString() != "0") {
            emit logMessage(tr("欧易API返回错误: %1").arg(rootObj["msg"].toString()), 2);
            success = false;
            break;
        }

        QJsonArray dataArray = rootObj["data"].toArray();
        if (dataArray.isEmpty()) {
            emit logMessage(tr("没有获取到数据"), 1);
            break;
        }

        // 解析tick数据
        for (const auto &item : dataArray) {
            QJsonObject tick = item.toObject();
            if (tick.isEmpty()) continue;

            AppData::MarketData marketData;
            marketData.symbol = symbol;
            marketData.timestamp = QDateTime::fromString(tick["ts"].toString(), Qt::ISODate);
            marketData.close = tick["px"].toString().toDouble(); // 成交价
            marketData.volume = tick["sz"].toString().toDouble(); // 成交量
            marketData.amount = marketData.close * marketData.volume;
            
            // 对于tick数据，open/high/low都等于close
            marketData.open = marketData.close;
            marketData.high = marketData.close;
            marketData.low = marketData.close;

            // 获取买卖盘数据（如果有）
            if (tick.contains("side")) {
                QString side = tick["side"].toString();
                if (side == "buy") {
                    marketData.bidPrice = marketData.close;
                    marketData.bidVolume = marketData.volume;
                } else {
                    marketData.askPrice = marketData.close;
                    marketData.askVolume = marketData.volume;
                }
            }

            // 只添加时间范围内的数据
            if (marketData.timestamp >= startTime && marketData.timestamp <= endTime) {
                data.append(marketData);
                totalFetched++;
            }
        }

        // 更新进度
        emit logMessage(tr("已获取%1条tick数据").arg(totalFetched), 0);
        emit progressUpdated(static_cast<int>(totalFetched * 100.0 / 10000)); // 假设最多10000条

        // 更新起始时间
        currentStart = currentEnd.addMSecs(1); // 加1毫秒避免重复
    }
    return true;
}
