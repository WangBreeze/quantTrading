#ifndef STRATEGYLOADER_H
#define STRATEGYLOADER_H

#include <QObject>
#include <QMap>
#include <memory>
#include "../history/Strategy.h"
#include "../online/AppData.h"

class StrategyLoader : public QObject
{
    Q_OBJECT
public:
    explicit StrategyLoader(QObject *parent = nullptr);
    
    // 从文件加载策略配置
    bool loadFromFile(const QString &filePath);
    
    // 从回测结果加载策略
    bool loadFromBacktestResult(const AppData::BacktestResult &result);
    
    // 获取策略实例
    std::shared_ptr<Strategy> getStrategy(const QString &strategyName);
    
    // 获取所有策略
    QMap<QString, std::shared_ptr<Strategy>> getAllStrategies();
    
    // 清除所有策略
    void clearStrategies();
    
signals:
    void strategyLoaded(const QString &strategyName);
    void errorOccurred(const QString &message);

private:
    // 创建策略实例
    std::shared_ptr<Strategy> createStrategy(const QString &className);
    
    QMap<QString, std::shared_ptr<Strategy>> m_strategies;
    QMap<QString, QVariantMap> m_strategyConfigs;
};

#endif // STRATEGYLOADER_H