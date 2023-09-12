#include "cpanelmainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QStringList argList = QCoreApplication::arguments();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CPanel_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    CPanelMainWindow w;

    if(argList.contains("--window"))
        w.show();
    else if (argList.contains("--maximized"))
        w.showMaximized();
    else if (argList.contains("--help"))
    {
        std::cout << "Parameters:\n\t"
                     "--fullscreen:\tOpen Screen in mode FullScreen\n\t"
                     "--maximized:\tOpen Screen in mode Maximized"
                     "" << std::endl;
        exit(0);
    }
    else
        w.showFullScreen();

    return a.exec();
}
