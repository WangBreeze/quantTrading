#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);

    qDebug() << "Style :" << QStyleFactory::keys();
    // 然后设置样式或加载 UI：
    app.setStyle("Windows");
    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/resource/qml/main.qml"));
    // QObject::connect(
    //     &engine,
    //     &QQmlApplicationEngine::objectCreated,
    //     &app,
    //     [url](QObject *obj, const QUrl &objUrl) {
    //         if (!obj && url == objUrl)
    //             QCoreApplication::exit(-1);
    //     },
    //     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
