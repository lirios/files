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
 * File: diriteminfo.h
 * Date: 30/01/2014
 */

#ifndef DIRITEMINFO_H
#define DIRITEMINFO_H

#include <QtGlobal>
#include <QVector>
#include <QFileInfo>
#include <QSharedData>
#include <QDateTime>
#include <QDir>
#include <QMimeType>
#include <QMimeDatabase>

class DirItemInfoPrivate;


/*!
 * \brief The DirItemInfo class
 *
 *  It intends to provide the same information as QFileInfo for Local Files:
 *     * selection state
 *     * any information about the item type, if it is Local/Remove
 *     *
 */
class DirItemInfo
{
public:
     DirItemInfo();
     DirItemInfo(const QString& filePath);
     DirItemInfo(const DirItemInfo& other);
     DirItemInfo(const QFileInfo& fi);


     virtual ~DirItemInfo();

public:
    bool              isSelected() const;
    bool              setSelection(bool selected);
    virtual bool      isValid() const;

    /*!
     * \brief isLocal()
     * \return true if the file is the disk: valid for Trash and any mounted FS
     */
    virtual bool      isLocal() const;

    /*!
     * \brief isRemote()
     * \return true if the file is in any remote host, mounted File Sharing is considered as Local
     */
    virtual bool      isRemote() const;

    QFileInfo diskFileInfo() const;

    inline void swap(DirItemInfo &other)
    { qSwap(d_ptr, other.d_ptr); }

    virtual inline DirItemInfo& operator=(const DirItemInfo &other)
    {  swap(*(const_cast<DirItemInfo*>(&other))); return *this; }

    virtual bool      exists()   const;
    virtual QString   filePath() const;
    virtual QString   fileName() const;
    virtual QString   path() const;
    virtual QString   absolutePath() const;
    virtual QString   absoluteFilePath() const;
    virtual QString   urlPath() const;
    virtual bool      isReadable() const;
    virtual bool      isContentReadable() const;
    virtual bool      isWritable() const;
    virtual bool      isExecutable() const;
    virtual bool      isRelative() const;
    virtual bool      isAbsolute() const;
    virtual bool      isFile() const;
    virtual bool      isDir() const;
    virtual bool      isSymLink() const;
    virtual bool      isRoot() const;
    virtual QFile::Permissions  permissions() const;
    virtual qint64    size() const;
    virtual QDateTime created() const;
    virtual QDateTime lastModified() const;
    virtual QDateTime lastRead() const;
    virtual QMimeType mimeType() const;

    virtual void      setFile(const QString &dir, const QString & file);

#if 0
    virtual QString   path() const;
    virtual QString   owner()   const;
    virtual uint      ownerId() const;
    virtual QString   group()   const;
    virtual uint      groupId() const;
    virtual bool      permission(QFile::Permissions permissions) const;
#endif

protected:
   QString           filePathFrom(const QString& path) const;

protected:
    QSharedDataPointer<DirItemInfoPrivate> d_ptr;
};

typedef QVector<DirItemInfo>   DirItemInfoList;

Q_DECLARE_SHARED(DirItemInfo)
Q_DECLARE_METATYPE(DirItemInfo)




class  DirItemInfoPrivate : public QSharedData
{
public:
    DirItemInfoPrivate();
    DirItemInfoPrivate(const DirItemInfoPrivate& other);
    DirItemInfoPrivate(const QFileInfo& fi);
    void setFileInfo(const QFileInfo&);

public:
    bool      _isValid     :1;
    bool      _isLocal     :1;
    bool      _isRemote    :1;
    bool      _isSelected  :1;
    bool      _isAbsolute  :1;
    bool      _exists      :1;
    bool      _isFile      :1;
    bool      _isDir       :1;
    bool      _isSymLink   :1;
    bool      _isRoot      :1;
    bool      _isReadable  :1;
    bool      _isWritable  :1;
    bool      _isExecutable:1;
    QFile::Permissions  _permissions;
    qint64    _size;
    QDateTime _created;
    QDateTime _lastModified;
    QDateTime _lastRead;
    QString   _path;
    QString   _fileName;
    QString   _normalizedPath;
    static QMimeDatabase mimeDatabase;
};



/*!
 * \brief The ActionPaths struct contains helper functions to do simple path handling
 *
 *  Paths stored here are supposed to NOT be relative.
 *
 *  It does not use any QFileInfo method, so it may work for any URL type
 */
struct ActionPaths
{
 public:
    ActionPaths() {}
    ActionPaths(const QString& source)
    {
        setSource(source);
    }
    inline void setSource(const QString& source)
    {        
        int pathLen = source.lastIndexOf(QDir::separator());
        if (pathLen != -1)
        {
           _source   = source;
           _sFile    = QStringRef(&_source, pathLen + 1,  _source.size() - pathLen - 1);
           _origPath = QStringRef(&_source, 0, pathLen);
        }
        else
        {
           //avoids possible memory corruption using relative paths, QStringRef would be empty/null
           //relative paths currently are not supported
            setSource(QString(".") + QDir::separator() + source);
        }
    }
    inline void setTargetPathOnly(const QString& path)
    {
        _targetPath = path;
        _target     = path +  QDir::separator();
        _target    += _sFile;
    }
    inline void setTargetFullName(const QString& fullPathname)
    {
        _target = fullPathname;
        int lastSeparator = _target.lastIndexOf(QDir::separator());
        if (lastSeparator > 0)
        {
            _targetPath  = _target.mid(0, lastSeparator);
        }
    }
    inline ActionPaths& operator=(const ActionPaths& other)
    {
        setSource(other._source);
        setTargetFullName(other._target);
        return *this;
    }
    inline bool              areEquals() const {return _source == _target;}
    inline bool              arePathsEquals() const
    {
        return QStringRef::compare(_origPath, _targetPath) == 0;
    }
    inline const QString&    source()    const {return _source;}
    inline const QString&    target()    const {return _target;}
    inline const QString&    targetPath()const {return _targetPath;}
    inline const QStringRef& file()      const {return _sFile;}
    inline int               baseOrigSize() const {return _origPath.size();}
    inline void  toggle()
    {
        QString savedSource(_source);
        setSource(_target);
        setTargetFullName(savedSource);
    }
 private:
    QString    _source;      //!<   source full pathname
    QString    _target;      //!<   target full pathname
    QString    _targetPath;  //!<   target path only
    QStringRef _sFile;       //!<   source file name only
    QStringRef _origPath;    //!<   source path only
};

Q_DECLARE_METATYPE(ActionPaths)

typedef QList<ActionPaths>       ActionPathList;

#endif // DIRITEMINFO_H
