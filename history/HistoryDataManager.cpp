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
