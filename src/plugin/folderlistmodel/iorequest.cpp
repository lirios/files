/*
 * Copyright (C) 2012 Robin Burchell <robin+nemo@viroteck.net>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "iorequest.h"
#include "qtrashutilinfo.h"
#include "diriteminfo.h"
#include "trashiteminfo.h"

#include <QDirIterator>
#include <QDebug>

#ifdef DEBUG_MESSAGES
#include <QDateTime>
#include <QThread>
#endif

IORequest::IORequest() : QObject(), m_type(DirList)
{
}

IORequest::~IORequest()
{

}

IORequest::RequestType IORequest::type() const
{
    return m_type;
}

//----------------------------------------------------------------------------------
IORequestLoader::IORequestLoader(const QString &pathName,
                                 QDir::Filter filter,
                                 bool isRecursive)
      : IORequest()
      , mLoaderType(NormalLoader)
      , mPathName(pathName)
      , mFilter(filter)
      , mIsRecursive(isRecursive)
{
}

IORequestLoader::IORequestLoader(const QString& trashRootDir,
                                 const QString &pathName,
                                 QDir::Filter filter,
                                 bool isRecursive)
      : IORequest()
      , mLoaderType(TrashLoader)
      , mPathName(pathName)
      , mFilter(filter)
      , mIsRecursive(isRecursive)
      , mTtrashRootDir(trashRootDir)
{

}

IORequestLoader::~IORequestLoader()
{

}

DirItemInfoList  IORequestLoader::getContents()
{
   return mLoaderType == NormalLoader ?
                getNormalContent() :
                getTrashContent();
}

DirItemInfoList  IORequestLoader::getNormalContent()
{
#if DEBUG_EXT_FS_WATCHER
    qDebug() << "[exfsWatcher]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
             << Q_FUNC_INFO;
#endif
    DirItemInfoList directoryContents;
    directoryContents = add(mPathName, mFilter, mIsRecursive, directoryContents);
    return directoryContents;
}

DirItemInfoList IORequestLoader::add(const QString &pathName,
                                      QDir::Filter filter,
                                      bool isRecursive,
                                      DirItemInfoList directoryContents)
{
    QDir tmpDir = QDir(pathName, QString(), QDir::NoSort, filter);
    QDirIterator it(tmpDir);
    while (it.hasNext())
    {
        it.next();
        if(it.fileInfo().isDir() && isRecursive) {
            directoryContents = add(it.fileInfo().filePath(),
                                    filter, isRecursive, directoryContents);
        } else {
            directoryContents.append(DirItemInfo(it.fileInfo()));
        }
    }
    return directoryContents;
}

DirItemInfoList  IORequestLoader::getTrashContent()
{
   DirItemInfoList directoryContents;
   QTrashUtilInfo trashInfo;
   QDir tmpDir = QDir(mPathName, QString(), QDir::NoSort, mFilter);
   bool isTopLevel = QFileInfo(mPathName).absolutePath() == mTtrashRootDir;
   QDirIterator it(tmpDir);
   while (it.hasNext())
   {
       it.next();
       trashInfo.setInfo(mTtrashRootDir, it.fileInfo().absoluteFilePath());
       if (!isTopLevel || (isTopLevel && trashInfo.existsInfoFile() && trashInfo.existsFile()) )
       {
          //TODO read the trashinfo file and set it into  a display field
          //     the display field can be a string the usally points to absoluteFilePath()
          //     it would be used only in the DirModel::data()
           TrashItemInfo item(QTrashUtilInfo::filesTrashDir(mTtrashRootDir),
                              it.fileInfo().absoluteFilePath());
           directoryContents.append(item);
       }
   }
   return directoryContents;
}


//-----------------------------------------------------------------------------------------------
DirListWorker::DirListWorker(const QString &pathName, QDir::Filter filter, const bool isRecursive)
    : IORequestLoader(pathName, filter, isRecursive)
{

}


DirListWorker::DirListWorker(const QString& trashRootDir, const QString &pathName, QDir::Filter filter, const bool isRecursive)
    : IORequestLoader(trashRootDir, pathName, filter, isRecursive)
{

}

DirListWorker::~DirListWorker()
{

}

void DirListWorker::run()
{
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "Running on: " << QThread::currentThreadId();
#endif

    DirItemInfoList directoryContents = getContents();

    // last batch
    emit itemsAdded(directoryContents);
    emit workerFinished();
}




//-------------------------------------------------------------------------------------
TrashListWorker::TrashListWorker(const QString& trashRoot, const QString &path, QDir::Filter filter)
  : DirListWorker(trashRoot, path, filter, false)
{
    mLoaderType = TrashLoader;
}

TrashListWorker::~TrashListWorker()
{

}

//---------------------------------------------------------------------------------------------------------
ExternalFileSystemChangesWorker::ExternalFileSystemChangesWorker(const DirItemInfoList &content,
                                                   const QString &pathName,
                                                   QDir::Filter filter,
                                                   const bool isRecursive)
    : IORequestLoader(pathName, filter, isRecursive)

{
    m_type        = DirListExternalFSChanges;
    int counter = content.count();
    while (counter--)
    {
        m_curContent.insert( content.at(counter).absoluteFilePath(), content.at(counter) );
    }
}


ExternalFileSystemChangesWorker::~ExternalFileSystemChangesWorker()
{

}

int ExternalFileSystemChangesWorker::compareItems(const DirItemInfoList& contentNew)
{
    int   addedCounter=0;
    int   removedCounter=0;
#if DEBUG_EXT_FS_WATCHER
        qDebug() << "[exfsWatcher]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                 << Q_FUNC_INFO
                 << "m_curContent.count():"      << m_curContent.count()
                 << "contentNew.count():" << contentNew.count();
#endif
    int counter = contentNew.count();
    if (counter > 0)
    {
        int tmpCounter = counter;
        while (tmpCounter--)
        {
            const DirItemInfo& originalItem = contentNew.at(tmpCounter);
            const DirItemInfo  existItem    = m_curContent.value(originalItem.absoluteFilePath());
            if ( existItem.exists() )
            {
                //it may have changed
                if (   originalItem.size()         != existItem.size()
                       || originalItem.lastModified() != existItem.lastModified()
                       || originalItem.permissions()  != existItem.permissions()
                       )
                {
                    emit changed(originalItem);
                }
                //remove this item
                m_curContent.remove(originalItem.absoluteFilePath());
            }
            else // originalItem was added
            {
                emit added(originalItem);
                ++addedCounter;
            }
        }

        QHash<QString, DirItemInfo>::iterator  i = m_curContent.begin();
        for ( ;  i != m_curContent.end();  ++removedCounter, ++i )
        {
            emit removed(i.value());
        }
    }
#if DEBUG_EXT_FS_WATCHER
        qDebug() << "[exfsWatcher]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                 << Q_FUNC_INFO
                 << "addedCounter:"   << addedCounter
                 << "removedCounter:" << removedCounter;
#endif

   return counter;
}

void ExternalFileSystemChangesWorker::run()
{
    DirItemInfoList directoryContents = getContents();    
    int remainingitemsCounter = compareItems(directoryContents);
    emit finished(remainingitemsCounter);
}


//---------------------------------------------------------------------
ExternalFileSystemTrashChangesWorker::ExternalFileSystemTrashChangesWorker(const QStringList &pathNames,
                                                                           const DirItemInfoList &list,
                                                                           QDir::Filter filter)
    :  ExternalFileSystemChangesWorker(list, pathNames.at(0), filter, false)
    ,  m_pathList(pathNames)
{
    mLoaderType = TrashLoader;
}

ExternalFileSystemTrashChangesWorker::~ExternalFileSystemTrashChangesWorker()
{

}

void ExternalFileSystemTrashChangesWorker::run()
{
    DirItemInfoList directoryContents;
    for(int counter = 0; counter < m_pathList.count(); counter++)
    {
        mPathName = QTrashUtilInfo::filesTrashDir(m_pathList.at(counter));
        directoryContents += getContents();
    }
    int remainingitemsCounter = compareItems(directoryContents);
    emit finished(remainingitemsCounter);
}
