#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "timing.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Timing timer_motor;
    engine.rootContext()->setContextProperty("Check_Time", &timer_motor);

    const QUrl url(u"qrc:/new_pid/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
