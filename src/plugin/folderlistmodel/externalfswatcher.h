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
 * File: externalfswatcher.h
 * Date: 9/14/2013
 */

#ifndef EXTERNALFSWATCHER_H
#define EXTERNALFSWATCHER_H

#include <QFileSystemWatcher>
#include <QStringList>

#define DEFAULT_NOTICATION_PERIOD  500


/*!
 * \brief The ExternalFSWatcher class watches for external changes in Disk emitting pathModified() signal.
 *
 *  The path(s) being watched  is/are set by using the slot \ref  setCurrentPath() or \ref setCurrentPaths()
 *
 *  The idea of this class is to minimize notifications as the current path in the File Manager can change quickly.
 *  A notification will occur if it was requested for a path and this path is still the current at the moment
 *  of the notification.
 *
 *  Once it detects a Disk change it will wait \ref getIntervalToNotifyChanges() milliseconds to notify that change.
 *  During the time it waits:
 *    \li  the notified path will NOT be watched until pathModified() is emitted
 *    \li  another call to \ref setCurrentPath() or \ref setCurrentPaths() invalidades the current change,
 *         that mean the signal pathModified() will NOT be emitted.
 *
 * \note When more than one path is being watched by using \ref setCurrentPaths() and changes happen in
 *       more than one path before the getIntervalToNotifyChanges() expires, only the LAST path modified
 *       will be notified as changed. It may possible that it goes to a loop if all the paths were modified,
 *       but the loop finishes when the last one from the list is modified.
 */
class ExternalFSWatcher : public QFileSystemWatcher
{
    Q_OBJECT
public:
    explicit ExternalFSWatcher(QObject *parent = 0);
    int      getIntervalToNotifyChanges() const;

    inline const QStringList& pathsWatched() const { return m_setPaths;}

signals:
     void      pathModified(const QString& path);

public slots:
     void      setCurrentPath(const QString& curPath);
     void      setCurrentPaths(const QStringList& paths);
     void      setIntervalToNotifyChanges(int ms);     

private slots:
     void      slotDirChanged(const QString&);
     void      slotFireChanges();

private:
     void      clearPaths();

private:
     QStringList m_setPaths;
     QString     m_changedPath;
     unsigned    m_waitingEmitCounter;
     int         m_msWaitTime;
     int         m_lastChangedIndex;
};

#endif // EXTERNALFSWATCHER_H
