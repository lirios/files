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

#ifndef IOREQUEST_H
#define IOREQUEST_H

#include "diriteminfo.h"

#include <QHash>
#include <QDir>

class IORequest : public QObject
{
    Q_OBJECT
public:   
    explicit IORequest();
    virtual ~IORequest();
    
public:
    enum RequestType
    {
        DirList,
        DirListExternalFSChanges
    };
    virtual void run() = 0;
    RequestType  type() const;
    
private:
    // hide this because IORequest should *NOT* be parented directly
    using QObject::setParent;

protected:
    RequestType  m_type;
};



class IORequestLoader : public IORequest
{
 Q_OBJECT
public:
    enum LoaderType
    {
        NormalLoader,
        TrashLoader
    };

    IORequestLoader( const QString &pathName,
                     QDir::Filter filter,
                     bool isRecursive
                   );
    IORequestLoader( const QString& trashRootDir,
                     const QString &pathName,
                     QDir::Filter filter,
                     bool isRecursive
                   );                   
    virtual ~IORequestLoader();
    DirItemInfoList     getContents();

signals:
    void itemsAdded(const DirItemInfoList &files);

private:
    DirItemInfoList getNormalContent();
    DirItemInfoList getTrashContent();
    DirItemInfoList add(const QString &pathName, QDir::Filter filter,
                        bool isRecursive, DirItemInfoList directoryContents);
protected:
    LoaderType    mLoaderType;
    QString       mPathName;
    QDir::Filter  mFilter;
    bool          mIsRecursive;
    QString       mTtrashRootDir;
};




class DirListWorker : public IORequestLoader
{
    Q_OBJECT
public:
    explicit DirListWorker(const QString &pathName, QDir::Filter filter, const bool isRecursive);
    explicit DirListWorker(const QString& trashRootDir, const QString &pathName, QDir::Filter filter, const bool isRecursive);
    virtual ~DirListWorker();
    void run();
protected:
signals:
    void workerFinished();

};




class TrashListWorker  : public DirListWorker
{
    Q_OBJECT
public:
    explicit TrashListWorker(const QString &trashRoot, const QString& path, QDir::Filter filter);
    virtual ~TrashListWorker();
};



class  ExternalFileSystemChangesWorker : public IORequestLoader
{
    Q_OBJECT
public:
    explicit ExternalFileSystemChangesWorker(const DirItemInfoList& content,
                                      const QString &pathName,
                                      QDir::Filter filter,
                                      const bool isRecursive);
    virtual ~ExternalFileSystemChangesWorker();
    void     run();

protected:
    int  compareItems(const DirItemInfoList& contentNew);

signals:
    void     removed(const DirItemInfo&);
    void     changed(const DirItemInfo&);
    void     added(const   DirItemInfo& );
    void     finished(int);
private:
    QHash<QString, DirItemInfo>    m_curContent;   //!< using hash because the vector can be in any order
};



class ExternalFileSystemTrashChangesWorker : public ExternalFileSystemChangesWorker
{
 Q_OBJECT

public:
  ExternalFileSystemTrashChangesWorker(const QStringList& pathNames,
                                       const DirItemInfoList& list,
                                       QDir::Filter filter);
  virtual ~ExternalFileSystemTrashChangesWorker();
  void run();
private:
  QStringList    m_pathList;
};



#endif // IOREQUEST_H
