/*
 * Files app - File manager for Papyros
 * Copyright (C) 2015 Michael Spencer <sonrisesoftware@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QLibrary>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QLibraryInfo>

#include <QtQuickControls2/QQuickStyle>

#include <QDebug>

#include "config.h"

int main(int argc, char *argv[])
{
    // HiDPI support
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // Setup application
    QGuiApplication app(argc, argv);
    app.setApplicationName(QLatin1String("Files"));
    app.setApplicationVersion(QLatin1String(FILES_VERSION_STRING));
    app.setOrganizationDomain(QLatin1String("papyros.io"));
    app.setOrganizationName(QLatin1String("Papyros"));
    app.setDesktopFileName(QLatin1String("io.papyros.Files.desktop"));

    // Set default style
    if (QQuickStyle::name().isEmpty())
        QQuickStyle::setStyle(QLatin1String("Material"));

#if 0
    QString locale = QLocale::system().name();

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator papyrosFilesTranslator;
    papyrosFilesTranslator.load(locale, DATA_INSTALL_DIR "/translations");
    app.installTranslator(&papyrosFilesTranslator);
#endif

    // Setup QML engine and show the main window
    QQmlApplicationEngine engine(QUrl(QLatin1String("qrc:/qml/main.qml")));

    return app.exec();
}
