/**************************************************************************
 *
 * Copyright 2014 Canonical Ltd.
 * Copyright 2014 Carlos J Mazieri <carlos.mazieri@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * File: qtrashutilinfo.cpp
 * Date: 16/03/2014
 */

#include "qtrashutilinfo.h"

#include <QDir>
#include <QSettings>
#include <QDateTime>

QLatin1String filesDirString("files");
QLatin1String infoDirString("info");

void QTrashUtilInfo::clear()
{
    trashRoot.clear();
    filesDir.clear();
    absFile.clear();
    infoDir.clear();
    absInfo.clear();
    valid = false;
}


QString QTrashUtilInfo::filesTrashDir(const QString &trashDir)
{
   QString filesDir(trashDir + QDir::separator() + filesDirString);
   return filesDir;
}


QString QTrashUtilInfo::infoTrashDir(const QString &trashDir)
{
   QString infoDir(trashDir + QDir::separator() + infoDirString);
   return infoDir;
}


void QTrashUtilInfo::setInfoFromTrashItem(const QString &absTrashItem)
{
    valid = false;
    QFileInfo item(absTrashItem);
    if (item.absolutePath().endsWith(filesDirString))
    {
        QFileInfo filesUnderRoot(item.absolutePath());
        QTrashUtilInfo::setInfo(filesUnderRoot.absolutePath(), absTrashItem);
    }
    else
    {
        clear();
    }
}


void QTrashUtilInfo::setInfo(const QString& trashRootDir, const QString& filename)
{
    valid = !trashRootDir.isEmpty();
    if (valid)
    {
        QFileInfo f(filename);
        trashRoot  = trashRootDir;
        filesDir   = filesTrashDir(trashRootDir);
        absFile    = filesDir + QDir::separator() + f.fileName();
        infoDir    = infoTrashDir(trashRootDir) ;
        absInfo    = infoDir + QDir::separator() + f.fileName() +
                     QLatin1String(".trashinfo");
    }
    else
    {
        clear();
    }
}


bool QTrashUtilInfo::isValid()
{
    return valid;
}


bool QTrashUtilInfo::existsFile()
{
    return QFileInfo(absFile).exists();
}


bool QTrashUtilInfo::existsInfoFile()
{
    return QFileInfo(absInfo).exists();
}


QString QTrashUtilInfo::getOriginalPathName()
{
    QString path;
    if (isValid())
    {
        QSettings inff(absInfo, QSettings::IniFormat);
        inff.beginGroup(QLatin1String("Trash Info"));
        QFileInfo f (inff.value(QLatin1String("Path")).toString());
        //Path contains the full pathname
        path = f.absoluteFilePath();
    }
    return path;
}


bool QTrashUtilInfo::createTrashInfoFile(const QString& orignalPathname)
{
    bool ret = isValid();
    if (ret)
    {
        QByteArray content("[Trash Info]\nPath=");
        content += orignalPathname + QLatin1Char('\n');
        content += "DeletionDate=";
        content += QDateTime::currentDateTime().toString(Qt::ISODate) + QLatin1Char('\n');
        QFile f(absInfo);
        ret = f.open(QFile::WriteOnly | QFile::Truncate) &&
              f.write(content) == content.size();
        f.close();
    }
    return ret;
}



bool QTrashUtilInfo::removeTrashInfoFile()
{
    QFile infoFile(absInfo);
    bool ret = false;
    if (valid && infoFile.exists())
    {
        ret = infoFile.remove();
    }
    return ret;
}
