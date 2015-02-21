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
 * File: locationsfactory.cpp
 * Date: 05/03/2014
 */

#include "diriteminfo.h"
#include "locationsfactory.h"
#include "location.h"
#include "locationurl.h"
#include "disklocation.h"
#include "trashlocation.h"
#include "trashiteminfo.h"

#include <QDir>
#include <QDebug>



LocationsFactory::LocationsFactory(QObject *parent)
 : QObject(parent)
 , m_curLoc(0)
 , m_lastValidFileInfo(0)
{
   m_locations.append(new DiskLocation(LocalDisk));
   m_locations.append(new TrashLocation(TrashDisk));
}


LocationsFactory::~LocationsFactory()
{
    ::qDeleteAll(m_locations);
    m_locations.clear();
    if (m_lastValidFileInfo)
    {
        delete m_lastValidFileInfo;
    }
}


/*!
 * \brief LocationsFactory::parse() identifies what main location that path/url refers to
 * \param path it is supposed to be always a full path like: file:///myDir /myDir trash:/// trash:///myDir
 * \return
 */

Location * LocationsFactory::parse(const QString& uPath)
{
    int index = -1;
    int type  = -1;
    Location * location = 0;   
    if ( (index = uPath.indexOf(QChar(':'))) != -1 )
    {
#if defined(Q_OS_WIN)
#else
#if defined(Q_OS_UNIX)
        if (uPath.startsWith(LocationUrl::TrashRootURL.midRef(0,6)))
        {
            type = TrashDisk;
            m_tmpPath  = LocationUrl::TrashRootURL + stringAfterSlashes(uPath, index+1);
        }
        else
#endif //Q_OS_UNIX
#endif //Q_OS_UNIX
        if (uPath.startsWith(LocationUrl::DiskRootURL.midRef(0,5)))
        {
            type = LocalDisk;
            m_tmpPath  = QDir::rootPath() + stringAfterSlashes(uPath, index+1);
        }
    }
    else
    {
        m_tmpPath = stringAfterSlashes(uPath, -1);
        type    = LocalDisk;
        if (!m_tmpPath.startsWith(QDir::rootPath()) && m_curLoc)
        {
            //it can be any, check current location
            type = m_curLoc->type();
        }
    }
    if (!m_tmpPath.isEmpty() && type != -1)
    {
        location = m_locations.at(type);       
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "input path:" << uPath  << "location result:" << location;
#endif
    return location;
}


Location * LocationsFactory::setNewPath(const QString& uPath)
{
    storeValidFileInfo(0);
    Location *location = parse(uPath);
    if (location)
    {
        DirItemInfo *item = location->validateUrlPath(m_tmpPath);
        if (item)
        {
            //isContentReadable() must already carry execution permission
            if (item->isValid() && item->isDir() && item->isContentReadable())
            {
                location->setInfoItem(item);
                if (location != m_curLoc)
                {
                    if (m_curLoc)
                    {
                        m_curLoc->stopWorking();
                    }
                    emit locationChanged(m_curLoc, location);
                    location->startWorking();
                    m_curLoc = location;
                }
            }
            else
            {
                storeValidFileInfo(item);
                location = 0;
            }
        }
        else
        {   // not valid
            location = 0;
        }
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "input path:" << uPath  << "location result:" << location;
#endif
    return location;
}


QString  LocationsFactory::stringAfterSlashes(const QString &url, int firstSlashIndex) const
{
    QString ret;
    if (firstSlashIndex >=0)
    {
        while ( firstSlashIndex < url.length() && url.at(firstSlashIndex) == QDir::separator())
        {
            ++firstSlashIndex;
        }
        if (firstSlashIndex < url.length())
        {
            ret = url.mid(firstSlashIndex);
        }
    }
    else
    {
        ret = url;
        firstSlashIndex = 0;
    }
    //replace any double slashes by just one
    for(int charCounter = ret.size() -1; charCounter > 0; --charCounter)
    {
        if (ret.at(charCounter) == QDir::separator() &&
                ret.at(charCounter-1) == QDir::separator())
        {
            ret.remove(charCounter,1);
        }
    }
    return ret;
}


void LocationsFactory::storeValidFileInfo(DirItemInfo *item)
{
    if (m_lastValidFileInfo)
    {
        delete m_lastValidFileInfo;
    }
    m_lastValidFileInfo = item;
}
