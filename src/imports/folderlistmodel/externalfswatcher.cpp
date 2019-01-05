/**************************************************************************
 *
 * Copyright 2013 Canonical Ltd.
 * Copyright 2013 Carlos J Mazieri <carlos.mazieri@gmail.com>
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
 * File: externalfswatcher.cpp
 * Date: 9/14/2013
 */

#include "externalfswatcher.h"

#include <QTimer>
#include <QDateTime>
#include <QDebug>

#if DEBUG_EXT_FS_WATCHER
# define DEBUG_FSWATCHER()    \
    qDebug() << "[extFsWatcher]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") \
             << Q_FUNC_INFO << "m_setPath:" << m_setPaths \
             << "m_changedPath:" << m_changedPath        \
             << "m_waitingEmit:" << m_waitingEmitCounter
# define DEBUG_FSWATCHER_MSG(msg) DEBUG_FSWATCHER() << msg
#else
# define DEBUG_FSWATCHER()  /**/
# define DEBUG_FSWATCHER_MSG(msg) /* msg */
#endif //


ExternalFSWatcher::ExternalFSWatcher(QObject *parent) :
    QFileSystemWatcher(parent)
  , m_waitingEmitCounter(0)
  , m_msWaitTime(DEFAULT_NOTICATION_PERIOD)
  , m_lastChangedIndex(-1)
{
    connect(this,   SIGNAL(directoryChanged(QString)),
            this,   SLOT(slotDirChanged(QString)));
}


void ExternalFSWatcher::setCurrentPath(const QString &curPath)
{
    if (!curPath.isEmpty() && (m_setPaths.count() != 1 || m_setPaths.at(0) != curPath))
    {
       setCurrentPaths(QStringList(curPath));
    }
}


void ExternalFSWatcher::setCurrentPaths(const QStringList &paths)
{    
    if (paths.count() > 0)
    {
        QStringList myPaths(paths);
        ::qSort(myPaths);
        m_setPaths = myPaths;
    }
    else
    {
        m_setPaths = paths;
    }
    clearPaths();
    //cleaning m_changedPath avoids any notification for a change
    // already scheduled to happen in slotFireChanges()
    m_changedPath.clear();    
    QFileSystemWatcher::addPaths(m_setPaths);
    DEBUG_FSWATCHER();
}


void ExternalFSWatcher::clearPaths()
{
    QStringList existentPaths = QFileSystemWatcher::directories();
    if (existentPaths.count() > 0)
    {
        QFileSystemWatcher::removePaths(existentPaths);
    }
}


/*!
 * \brief ExternalFSWatcher::slotDirChanged() schedules a Disk change to be notified
 *
 *  Once path that belongs to \a m_setPaths is modified in the Disk it becomes the \a m_changedPath and
 *  its change is scheculed to notified later. This path is taken out from QFileSystemWatcher to avoid
 *  lots of continuous notifications from QFileSystemWatcher when having hevy disk io.
 *
 * \param dir directory changed in the File System
 */
void ExternalFSWatcher::slotDirChanged(const QString &dir)
{
    DEBUG_FSWATCHER();
    int index = m_setPaths.indexOf(dir);
    if (index != -1  && (m_waitingEmitCounter == 0 || dir != m_changedPath))
    {
        m_lastChangedIndex = index;
        //changed path is taken from the QFileSystemWatcher and it becomes the current changed
        //in this case there will not be slotDirChanged() for this path until slotFireChanges()
        //restores the path in the QFileSystemWatcher
        removePath(m_setPaths.at(m_lastChangedIndex));
        ++m_waitingEmitCounter;
        m_changedPath = dir;
        QTimer::singleShot(m_msWaitTime, this, SLOT(slotFireChanges()));       
    }
}


/*!
 * \brief ExternalFSWatcher::slotFireChanges() emits \ref pathModified() only when it is sure
 *  that the LAST current path was changed.
 *
 *  The notification will be sent out only for the LAST modified path (if more than one) from the \a m_setPaths
 *
 *  \sa \ref ExternalFSWatcher class
 */
void ExternalFSWatcher::slotFireChanges()
{
   DEBUG_FSWATCHER();
   if ( --m_waitingEmitCounter == 0 ) //no more changes queued (it is the LAST), time to notify
   {
       //the notification will not fired if either setCurrentPath() or setCurrentPaths()
       //was called after the change in the disk be noticed
       if (m_lastChangedIndex != -1 &&
           m_lastChangedIndex < m_setPaths.count() &&
           m_setPaths.at(m_lastChangedIndex) == m_changedPath)
       {          
           emit pathModified(m_changedPath);
           DEBUG_FSWATCHER_MSG("emit pathModified()");
       }
       //restore the original m_setPaths list in QFileSystemWatcher anyway
       //it does not matter if the notification was fired or not.
       clearPaths();
       QFileSystemWatcher::addPaths(m_setPaths);
   }
}



void ExternalFSWatcher::setIntervalToNotifyChanges(int ms)
{
    m_msWaitTime = ms;
}


int ExternalFSWatcher::getIntervalToNotifyChanges() const
{
    return m_msWaitTime;
}
