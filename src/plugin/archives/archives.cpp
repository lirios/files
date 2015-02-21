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

#include "archives.h"
#include "QDebug"

void Archives::extractZip(const QString path, const QString destination)
{
    QString program = "unzip"; // This programm is available in the images as it is one of the dependencies of the ubuntu-download-manager package.
    QStringList arguments;
    arguments << path << "-d" << destination;

    extractArchive(program, arguments);
}

void Archives::extractTar(const QString path, const QString destination)
{
    QString program = "tar";
    QStringList arguments;
    arguments << "xf" << path << "-C" << destination;

    extractArchive(program, arguments);
}

void Archives::extractArchive(const QString program, const QStringList arguments)
{
    if (_process != nullptr && _process->state() == QProcess::ProcessState::Running) {
        return; // Do not allow two extractions running in parallel. Due to the way this is used in QML parallelization is not needed.
    }

    _process = new QProcess(this);

    // Connect to internal slots in order to have one unified onFinished slot handling both events for QML.
    connect(_process,
            static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>
            (&QProcess::finished), this, &Archives::_onFinished);
    connect(_process,
            static_cast<void(QProcess::*)(QProcess::ProcessError)>
            (&QProcess::error), this, &Archives::_onError);
    connect(this, &Archives::killProcess,
            _process, &QProcess::kill);

    _process->start(program, arguments);
}

void Archives::cancelArchiveExtraction()
{
    qDebug() << "Cancelling archive extraction";
    emit killProcess();
}

void Archives::_onError(QProcess::ProcessError error)
{
    qDebug() << "Extraction failed (1) with the following error:" << _process->readAllStandardError();
    emit finished(false, error);
}

void Archives::_onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ((exitStatus == QProcess::NormalExit || exitCode == 0) && _process->readAllStandardError().trimmed().isEmpty()) {
        emit finished(true, -1);
    } else {
        qDebug() << "Extraction failed (2) with the following error:" << _process->readAllStandardError();
        emit finished(false, -1);
    }
}
