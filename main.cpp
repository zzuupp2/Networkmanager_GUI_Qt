#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QLocale>
#include <QTranslator>

#include <src/wrapper/nm_wrapper.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Networkmanager_GUI_Qt_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QQmlApplicationEngine engine;
    qRegisterMetaType<Net::DeviceInfo>();
    Net::NetworkManagerWrapper nm;
    engine.rootContext()->setContextProperty("nm", &nm);

    engine.loadFromModule("Networkmanager_GUI_Qt", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
