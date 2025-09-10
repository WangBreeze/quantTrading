#include "StrategyLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QPluginLoader>
#include <QDir>

StrategyLoader::StrategyLoader(QObject *parent)
    : QObject(parent)
{
}

bool StrategyLoader::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Failed to open strategy file: %1").arg(filePath));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("Failed to parse strategy file: %1").arg(parseError.errorString()));
        return false;
    }

    if (!doc.isObject()) {
        emit errorOccurred("Invalid strategy file format: root is not an object");
        return false;
    }

    QJsonObject root = doc.object();
    if (!root.contains("strategies") || !root["strategies"].isArray()) {
        emit errorOccurred("Invalid strategy file format: missing strategies array");
        return false;
    }

    QJsonArray strategies = root["strategies"].toArray();
    for (const QJsonValue &strategyValue : strategies) {
        if (!strategyValue.isObject()) continue;

        QJsonObject strategyObj = strategyValue.toObject();
        if (!strategyObj.contains("name") || !strategyObj.contains("class")) {
            emit errorOccurred("Invalid strategy format: missing name or class");
            continue;
        }

        QString strategyName = strategyObj["name"].toString();
        QString className = strategyObj["class"].toString();

        // 创建策略实例
        auto strategy = createStrategy(className);
        if (!strategy) {
            emit errorOccurred(QString("Failed to create strategy: %1").arg(className));
            continue;
        }

        // 保存配置
        QVariantMap config;
        if (strategyObj.contains("parameters") && strategyObj["parameters"].isObject()) {
            config = strategyObj["parameters"].toObject().toVariantMap();
        }

        // 初始化策略
        if (!(strategy->initialize(config))) {
            emit errorOccurred(QString("Failed to initialize strategy: %1").arg(strategyName));
            continue;
        }

        // 保存策略和配置
        m_strategies[strategyName] = strategy;
        m_strategyConfigs[strategyName] = config;
        emit strategyLoaded(strategyName);
    }

    return true;
}

bool StrategyLoader::loadFromBacktestResult(const AppData::BacktestResult &result)
{
    if (result.strategyName.isEmpty() || result.strategyClass.isEmpty()) {
        emit errorOccurred("Invalid backtest result: missing strategy name or class");
        return false;
    }

    // 创建策略实例
    auto strategy = createStrategy(result.strategyClass);
    if (!strategy) {
        emit errorOccurred(QString("Failed to create strategy: %1").arg(result.strategyClass));
        return false;
    }

    // 从回测结果构建配置
    QVariantMap config;
    config["initialCapital"] = result.initialCapital;
    config["symbols"] = result.symbols.count() > 0 ? result.symbols.first() : "";
    config["parameters"] = result.parameters;

    // 初始化策略
    if (!strategy->initialize(config)) {
        emit errorOccurred(QString("Failed to initialize strategy: %1").arg(result.strategyName));
        return false;
    }

    // 保存策略和配置
    m_strategies[result.strategyName] = strategy;
    m_strategyConfigs[result.strategyName] = config;
    emit strategyLoaded(result.strategyName);

    return true;
}

std::shared_ptr<Strategy> StrategyLoader::getStrategy(const QString &strategyName)
{
    return m_strategies.value(strategyName, nullptr);
}

QMap<QString, std::shared_ptr<Strategy>> StrategyLoader::getAllStrategies()
{
    return m_strategies;
}

void StrategyLoader::clearStrategies()
{
    m_strategies.clear();
    m_strategyConfigs.clear();
}

std::shared_ptr<Strategy> StrategyLoader::createStrategy(const QString &className)
{
    // 尝试从插件加载策略
    QString pluginPath = QString("./plugins/%1.dll").arg(className);
    QPluginLoader loader(pluginPath);
    QObject *plugin = loader.instance();
    
    // if (plugin) {
    //     Strategy *strategy = qobject_cast<Strategy*>(plugin);
    //     if (strategy) {
    //         return std::shared_ptr<Strategy>(strategy, [loader](Strategy*) mutable {
    //             loader.unload();
    //         });
    //     }
    //     loader.unload();
    // }

    // 内置策略创建
    // if (className == "MovingAverageStrategy") {
    //     return std::make_shared<MovingAverageStrategy>();
    // } else if (className == "RSIStrategy") {
    //     return std::make_shared<RSIStrategy>();
    // } else if (className == "MACDStrategy") {
    //     return std::make_shared<MACDStrategy>();
    // } else if (className == "BollingerBandsStrategy") {
    //     return std::make_shared<BollingerBandsStrategy>();
    // }

    return nullptr;
}
