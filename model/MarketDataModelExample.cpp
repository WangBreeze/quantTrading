#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "MarketDataModel.h"

// 这是一个示例代码，展示如何在 C++ 中注册 MarketDataModel 并将其传递给 QML
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // 创建 QML 引擎
    QQmlApplicationEngine engine;
    
    // 创建 MarketDataModel 实例
    MarketDataModel *marketDataModel = new MarketDataModel(&app);
    
    // 将模型注册到 QML 上下文
    engine.rootContext()->setContextProperty("marketDataModel", marketDataModel);
    
    // 加载主 QML 文件
    engine.load(QUrl(QStringLiteral("qrc:/resource/qml/window/MarketDataWindow.qml")));
    
    // 检查是否成功加载 QML
    if (engine.rootObjects().isEmpty())
        return -1;
    
    // 获取根对象（MarketDataWindow）
    QObject *rootObject = engine.rootObjects().first();
    
    // 调用 QML 中的 setModel 方法，传递 C++ 模型
    QMetaObject::invokeMethod(rootObject, "setModel",
                             Q_ARG(QVariant, QVariant::fromValue(marketDataModel)));
    
    // 添加一些测试数据
    QMetaObject::invokeMethod(rootObject, "addTestData");
    
    return app.exec();
}

// 在实际应用中，你可能需要在主应用程序中集成这些代码，而不是创建一个独立的示例程序
// 以下是在现有应用程序中集成 MarketDataModel 的示例代码：

/*
// 在主窗口类的头文件中声明 MarketDataModel
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    MarketDataModel *m_marketDataModel;
    // 其他成员...
};

// 在主窗口类的实现文件中初始化 MarketDataModel
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_marketDataModel(new MarketDataModel(this))
{
    // 设置 QML 引擎
    QQmlApplicationEngine *engine = new QQmlApplicationEngine(this);
    
    // 将模型注册到 QML 上下文
    engine->rootContext()->setContextProperty("marketDataModel", m_marketDataModel);
    
    // 加载 QML 文件
    engine->load(QUrl(QStringLiteral("qrc:/resource/qml/window/MarketDataPage.qml")));
    
    // 获取根对象
    QObject *rootObject = engine->rootObjects().first();
    
    // 创建 QQuickWidget 来显示 QML
    QQuickWidget *quickWidget = new QQuickWidget(engine, this);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setSource(QUrl(QStringLiteral("qrc:/resource/qml/window/MarketDataPage.qml")));
    
    // 设置为中心部件
    setCentralWidget(quickWidget);
    
    // 调用 QML 中的 setModel 方法
    QMetaObject::invokeMethod(quickWidget->rootObject(), "setModel",
                             Q_ARG(QVariant, QVariant::fromValue(m_marketDataModel)));
    
    // 添加一些测试数据或从其他数据源加载数据
    // ...
}
*/