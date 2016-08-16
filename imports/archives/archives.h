/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author : Niklas Wenzel <nikwen.developer@gmail.com>
 */

#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <QObject>
#include <QProcess>

class Archives : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void extractZip(const QString path, const QString destination);
    Q_INVOKABLE void extractTar(const QString path, const QString destination);
    Q_INVOKABLE void cancelArchiveExtraction();

signals:
    void finished(bool success, int errorCode);
    void killProcess();

private slots:
    void _onError(QProcess::ProcessError error);
    void _onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void extractArchive(const QString program, const QStringList arguments);

    QProcess* _process = nullptr;
};


#endif // ARCHIVES_H
