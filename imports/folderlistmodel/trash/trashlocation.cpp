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
 * File: trashlocation.cpp
 * Date: 08/03/2014
 */

#include "iorequest.h"
#include "ioworkerthread.h"
#include "trashlocation.h"
#include "trashiteminfo.h"
#include "qtrashutilinfo.h"
#include "locationurl.h"
#include "externalfswatcher.h"

#include <QDebug>

TrashLocation::TrashLocation(int type, QObject *parent) :
    DiskLocation(type, parent)
   ,QTrashDir()

{
}

TrashLocation::~TrashLocation()
{

}

bool TrashLocation::becomeParent()
{

    bool ret = false;
    TrashItemInfo *trashInfo = static_cast<TrashItemInfo*> (m_info);
    if (trashInfo && !trashInfo->isRoot())
    {
        QString trashDir = trashInfo->getTrashDir();
        if (!trashDir.isEmpty())
        {
            TrashItemInfo *other = new TrashItemInfo(trashDir, trashInfo->absolutePath());
            if (other->isValid() && other->isDir())
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
    }
    return ret;
}


DirItemInfo * TrashLocation::validateUrlPath(const QString& urlPath)
{
     TrashItemInfo *item  =  0;
     QString  myPath(urlPath);

    //first void any relative path when is root
    if (m_info  && m_info->isRoot() && myPath.startsWith(QLatin1String("..")))
    {
        return item;
    }

    int firstSlash       = -1;

    // handle relative paths to the current
    if (!myPath.startsWith(LocationUrl::TrashRootURL) && m_info)
    {
        QFileInfo f;
        f.setFile(m_info->absoluteFilePath(), myPath);
        if (f.exists() && f.isDir())
        {
            TrashItemInfo *trashItem = static_cast<TrashItemInfo*> (m_info);
            item = new TrashItemInfo(trashItem->getTrashDir(), f.canonicalFilePath());
#if DEBUG_MESSAGES
            qDebug() << Q_FUNC_INFO << "cur path:" << m_info->absoluteFilePath()
                     << " new path:"  << item->absoluteFilePath()
                     << "url:"   << item->urlPath();
#endif
        }
        else
        {
           myPath = LocationUrl::TrashRootURL + urlPath;
        }
    }
    else
    {
        item  = new TrashItemInfo(myPath);
        if (!item->isRoot())
        {
            delete item;
            item = 0;
        }
    }

    //Not Relative, handle absolute path but it is not root
    if (item == 0)
    {       
        QString absTrashItem;
        QString trashItemFromRoot = myPath.mid(LocationUrl::TrashRootURL.size());
        foreach(const QString& trashRoot, allTrashes())
        {
            //this is the full path of the item, it does not mean it is a Trash top level item
            //example: trash:///Dir1/Dir2/Dir3  may be in /home/user/.local/share/Trash//Dir1/Dir2/Dir3
            absTrashItem = QTrashUtilInfo::filesTrashDir(trashRoot) + QDir::separator() + trashItemFromRoot;
            const QFileInfo info(absTrashItem);
            if (info.exists())
            {
                //check top level trash item
                firstSlash = trashItemFromRoot.indexOf(QDir::separator());
                QString toplevelDir = firstSlash != -1 ? trashItemFromRoot.left(firstSlash)
                                                       : trashItemFromRoot;

                QTrashUtilInfo topLevelTrashDirInfo;
                topLevelTrashDirInfo.setInfo(trashRoot, toplevelDir);
#if DEBUG_MESSAGES
                    qDebug() << Q_FUNC_INFO
                             <<  topLevelTrashDirInfo.absFile << "exists" << topLevelTrashDirInfo.existsFile()
                             <<  topLevelTrashDirInfo.absInfo << "exists" << topLevelTrashDirInfo.existsInfoFile();
#endif
                //check if a .trashinfo file for toplevel dir exists
                if (topLevelTrashDirInfo.existsInfoFile())
                {
                    item = new TrashItemInfo(QTrashUtilInfo::filesTrashDir(trashRoot), absTrashItem);
                    break;
                }
            }
        }
    }

    if (item)
    {
        if (!item->isValid() || !item->isContentReadable())
        {
            delete item;
            item = 0;
        }
        else
        {
            if (firstSlash != -1)
            {
                // TODO get the trashinfo information and carry into the item
            }
        }
    }
    return item;
}




void TrashLocation::refreshInfo()
{
    if (m_info && !m_info->isRoot())
    {
        TrashItemInfo *trashItem = static_cast<TrashItemInfo*>(m_info);
        TrashItemInfo *item = new TrashItemInfo(trashItem->getTrashDir(), trashItem->absoluteFilePath());
        delete m_info;
        m_info = item;
    }
}


void TrashLocation::startExternalFsWatcher()
{
    //TODO implement a Watcher for this
    //modify the existent watcher to work having  a list of paths
    if (m_usingExternalWatcher && m_extWatcher == 0 && isRoot())
    {     
        m_extWatcher = new ExternalFSWatcher(this);
        m_extWatcher->setIntervalToNotifyChanges(EX_FS_WATCHER_TIMER_INTERVAL);      
        m_extWatcher->setCurrentPaths(m_currentPaths);

        connect(m_extWatcher, SIGNAL(pathModified(QString)),
                this,         SIGNAL(extWatcherPathChanged(QString)));
    }
}


void TrashLocation::fetchItems(QDir::Filter dirFilter, bool recursive)
{
    Q_UNUSED(recursive)
    if (!m_info->isRoot()) //any item under the logical trash folder
    {
        //sub items inside Trash do not need external watcher, they will never be updated
        stopExternalFsWatcher();
        TrashItemInfo *trashItem = static_cast<TrashItemInfo*> (m_info);
        TrashListWorker *dlw  = new TrashListWorker(trashItem->getRootTrashDir(),
                                                    trashItem->absoluteFilePath(),
                                                    dirFilter);
       addTrashFetchRequest(dlw);
    }
    else    
    {
        m_currentPaths = allTrashes();
        startExternalFsWatcher();

        //the trash a is logical folder, its content can be composed by more than one physical folder
        foreach (const QString& trashRootDir, m_currentPaths)
        {
            TrashListWorker *dlw  = new TrashListWorker(trashRootDir,
                                                        QTrashUtilInfo::filesTrashDir(trashRootDir),
                                                        dirFilter);
            addTrashFetchRequest(dlw);
         }
    }
}


void TrashLocation::addTrashFetchRequest(TrashListWorker *workerObject)
{
    connect(workerObject,  SIGNAL(itemsAdded(DirItemInfoList)),
            this,          SIGNAL(itemsAdded(DirItemInfoList)));
    //it differs from DiskLocation
    connect(workerObject,  SIGNAL(workerFinished()),
            this,          SIGNAL(itemsFetched()));
    workerThread()->addRequest(workerObject);
}



void TrashLocation::fetchExternalChanges(const QString& urlPath,
                                         const DirItemInfoList& list,
                                         QDir::Filter dirFilter)
{
    Q_UNUSED(urlPath);
    if (m_extWatcher)
    {
       ExternalFileSystemTrashChangesWorker * extFsWorker =
        new ExternalFileSystemTrashChangesWorker( m_extWatcher->pathsWatched(),
                                                  list,
                                                  dirFilter
                                                );
       addExternalFsWorkerRequest(extFsWorker);
    }
}


void TrashLocation::startWorking()
{
   // do nothing, the startExternalFsWatcher() is called in fetchItems()
}


ActionPaths
TrashLocation::getRestorePairPaths(const DirItemInfo& item)  const
{
    const TrashItemInfo* ptrash = static_cast<const TrashItemInfo*> (&item);
    QTrashUtilInfo trashInfo;

    trashInfo.setInfo(ptrash->getRootTrashDir(), ptrash->absoluteFilePath());

    ActionPaths ret(ptrash->absoluteFilePath());
    ret.setTargetFullName(trashInfo.getOriginalPathName());

    return ret;
}


ActionPaths
TrashLocation::getMovePairPaths(const DirItemInfo &item) const
{
    ActionPaths ret(item.absoluteFilePath());

    QTrashUtilInfo trashInfo;
    trashInfo.setInfo(suitableTrash(item.absoluteFilePath()), item.absoluteFilePath());

    ret.setTargetFullName( trashInfo.absFile );
    return ret;
}
