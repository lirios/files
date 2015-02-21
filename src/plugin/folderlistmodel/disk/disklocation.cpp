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
 * File: disklocation.cpp
 * Date: 08/03/2014
 */

#include "disklocation.h"
#include "iorequest.h"
#include "ioworkerthread.h"
#include "externalfswatcher.h"

#include <QDebug>

#if defined(DEBUG_EXT_FS_WATCHER)
# define DEBUG_WATCHER()  qDebug() << "[extFsWatcher]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") \
                                   << Q_FUNC_INFO << this
#else
# define DEBUG_WATCHER() /**/
#endif

DiskLocation::DiskLocation(int type, QObject *parent)
   : Location(type, parent)
   , m_extWatcher(0)
{
}


DiskLocation::~ DiskLocation()
{
    stopExternalFsWatcher();
}


void DiskLocation::fetchItems(QDir::Filter dirFilter, bool recursive)
{
    DirListWorker *dlw  = new DirListWorker(m_info->absoluteFilePath(), dirFilter, recursive);
    connect(dlw,  SIGNAL(itemsAdded(DirItemInfoList)),
            this, SIGNAL(itemsAdded(DirItemInfoList)));
    connect(dlw,  SIGNAL(workerFinished()),
            this, SLOT(onItemsFetched()));
    workerThread()->addRequest(dlw);
}


bool DiskLocation::becomeParent()
{
    bool ret = false;
    if (m_info && !m_info->isRoot())
    {
        DirItemInfo *other = new DirItemInfo(m_info->absolutePath());
        if (other->isValid())
        {
            delete m_info;
            m_info = other;
            ret = true;
        }
        else
        {
            delete other;
        }
    }
    return ret;
}


void DiskLocation::refreshInfo()
{
    if (m_info)
    {
        DirItemInfo *item = new DirItemInfo(m_info->absoluteFilePath());
        delete m_info;
        m_info = item;
    }
}


/*!
 * \brief DiskLocation::stopExternalFsWatcher() stops the External File System Watcher
 */
void DiskLocation::stopExternalFsWatcher()
{
        if (m_extWatcher)
        {
            DEBUG_WATCHER();
            delete m_extWatcher;
            m_extWatcher = 0;
        }
}


/*!
 * \brief DiskLocation::startExternalFsWatcher() starts the External File System Watcher
 */
void DiskLocation::startExternalFsWatcher()
{
    if (m_extWatcher == 0)
    {
        DEBUG_WATCHER();
        m_extWatcher = new ExternalFSWatcher(this);
        m_extWatcher->setIntervalToNotifyChanges(EX_FS_WATCHER_TIMER_INTERVAL);

        connect(m_extWatcher, SIGNAL(pathModified(QString)),
                this,         SIGNAL(extWatcherPathChanged(QString)));       
       if (m_info)
       {               //setCurrentPath() checks for empty paths
           m_extWatcher->setCurrentPath(m_info->absoluteFilePath());
       }     
    }
}


void DiskLocation::onItemsFetched()
{
    if (m_extWatcher)
    {
         m_extWatcher->setCurrentPath(m_info->absoluteFilePath());
    }
    emit itemsFetched();
}


void DiskLocation::startWorking()
{
    if (m_usingExternalWatcher)
    {
        startExternalFsWatcher();
    }
}


void DiskLocation::stopWorking()
{
    stopExternalFsWatcher();
}


void DiskLocation::fetchExternalChanges(const QString &path,
                                        const DirItemInfoList &list,
                                        QDir::Filter dirFilter)
{
     ExternalFileSystemChangesWorker *extFsWorker =
          new ExternalFileSystemChangesWorker(list,
                                              path,
                                              dirFilter, false);
     addExternalFsWorkerRequest(extFsWorker);


}

void DiskLocation::addExternalFsWorkerRequest(ExternalFileSystemChangesWorker *extFsWorker)
{
    connect(extFsWorker,    SIGNAL(added(DirItemInfo)),
            this,           SIGNAL(extWatcherItemAdded(DirItemInfo)));

    connect(extFsWorker,    SIGNAL(removed(DirItemInfo)),
            this,           SIGNAL(extWatcherItemRemoved(DirItemInfo)));

    connect(extFsWorker,    SIGNAL(changed(DirItemInfo)),
            this,           SIGNAL(extWatcherItemChanged(DirItemInfo)));

    connect(extFsWorker,    SIGNAL(finished(int)),
            this,           SIGNAL(extWatcherChangesFetched(int)));

        workerThread()->addRequest(extFsWorker);
}


ExternalFSWatcher  * DiskLocation::getExternalFSWatcher() const
{   
    return m_extWatcher;
}


void DiskLocation::setUsingExternalWatcher(bool use)
{
    Location::setUsingExternalWatcher(use);
    if (m_usingExternalWatcher)
    {
        startExternalFsWatcher();
    }
    else
    {
        stopExternalFsWatcher();
    }
}


DirItemInfo * DiskLocation::validateUrlPath(const QString& uPath)
{
    QString myPath(uPath);
    QFileInfo tmpUrl(uPath);
    if (tmpUrl.isRelative() && m_info)
    {
        tmpUrl.setFile(m_info->absoluteFilePath(), uPath);
        myPath  =  tmpUrl.absoluteFilePath();
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "path:" << myPath;
#endif
    DirItemInfo * item = new DirItemInfo(myPath);
    if (!item->isValid() || !item->exists() || !item->isContentReadable())
    {
        delete item;
        item = 0;
    }
    return item;
}
