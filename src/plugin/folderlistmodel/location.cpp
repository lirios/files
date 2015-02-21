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
 * File: location.cpp
 * Date: 08/03/2014
 */
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
 * File: locations.cpp
 * Date: 04/03/2014
 */

#include "location.h"
#include "ioworkerthread.h"

Q_GLOBAL_STATIC(IOWorkerThread, ioWorkerThread)


Location::Location(int type, QObject *parent)
    : QObject(parent)
    , m_info(0)
    , m_type(type)
    , m_usingExternalWatcher(false)
{

}

Location::~Location()
{
    if (m_info)
    {
        delete m_info;
        m_info = 0;
    }
}


bool Location::isRoot() const
{
     return m_info ? m_info->isRoot() : false;
}


bool Location::isWritable() const
{
    return m_info->isWritable();
}


bool Location::isReadable() const
{
    return m_info ? m_info->isContentReadable() : false;
}

void Location::setInfoItem(const DirItemInfo &itemInfo)
{
    setInfoItem (new DirItemInfo(itemInfo));
}

void Location::setInfoItem(DirItemInfo *itemInfo)
{
    if (m_info)
    {
        delete m_info;
    }
    m_info = itemInfo;
}


QString Location::urlPath() const
{
    return m_info ? m_info->urlPath(): QString();
}


void Location::startWorking()
{

}

void Location::stopWorking()
{

}

bool Location::becomeParent()
{
    return false;
}

IOWorkerThread * Location::workerThread() const
{
    return ioWorkerThread();
}


//providing an empty method
void Location::fetchExternalChanges(const QString &path,
                                    const DirItemInfoList &list,
                                    QDir::Filter dirFilter)
{
    Q_UNUSED(path);
    Q_UNUSED(list);
    Q_UNUSED(dirFilter);
}


void Location::setUsingExternalWatcher(bool use)
{
    m_usingExternalWatcher = use;
}
