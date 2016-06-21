#include <QGuiApplication>
#include <QCommandLineParser>

#include "scene.h"
#include "simwindow.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("glviewer");
    QGuiApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Firemix Scene Viewer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("scene", "Path to scene file");
    QCommandLineOption portOption = QCommandLineOption(QStringList {"p", "UDP Port"});
    parser.addOption(portOption);

    parser.process(app);

    QStringList args = parser.positionalArguments();

    if (args.length() < 1) {
        qCritical("You need to specify a scene file!");
        exit(1);
    }

    QString scenePath = args[0];
    quint16 udpPort = parser.value(portOption).toInt();

    Scene scene(scenePath);

    SimWindow window(udpPort);
    window.resize(QSize(800, 600));
    window.show();

    return app.exec();
}
