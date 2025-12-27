#include "backend/backend.h"
// #include "backend/config.h"
#include "QtAwesome.h"
#include "QtAwesomeQuickImageProvider.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>

int
main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    fa::QtAwesome *awesome = new fa::QtAwesome(qApp);
    awesome->initFontAwesome();

    // qmlRegisterSingletonInstance("App", 1, 0, "Config", new Config);

    Backend backend;

    QQmlApplicationEngine engine;
    engine.addImageProvider("fa", new QtAwesomeQuickImageProvider(awesome));
    engine.rootContext()->setContextProperty("backend", &backend);

    QObject::connect(
                     &engine,
                     &QQmlApplicationEngine::objectCreationFailed,
                     &app,
                     []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);

    engine.loadFromModule("rpi_dashboard", "Main");

    return app.exec();
}
