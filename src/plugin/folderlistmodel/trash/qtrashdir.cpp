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
 * File: QTrashDir.cpp
 * Date: 06/02/2014
 */

#include "qtrashdir.h"
#include "qtrashutilinfo.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>

#if defined(Q_OS_UNIX)
# include <sys/statvfs.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif


QTrashDir::QTrashDir() : m_userId(::getuid())
{

}


bool QTrashDir::validate(const QString &trashDir, bool create) const
{
   bool ret = false;
   QFileInfo info(trashDir);
   if (!info.exists() && create)
   {
       createUserDir(info.absoluteFilePath());
   }
   if (checkUserDirPermissions(trashDir))
   {
       QString  files(QTrashUtilInfo::filesTrashDir(trashDir));
       QString  info(QTrashUtilInfo::infoTrashDir(trashDir));
       if ( (checkUserDirPermissions(files) || (create && createUserDir(files)))  &&
            (checkUserDirPermissions(info)  || (create && createUserDir(info)))
          )
       {
         ret = true;
       }
   }
   return ret;
}


bool QTrashDir::checkUserDirPermissions(const QString &dir) const
{
    QFileInfo f(dir);
    bool ret = false;   
    if ( f.isDir() && !f.isSymLink() )
    {
       QFile::Permissions permissions = f.permissions();
       bool owner   = permissions & (QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
       bool group   = permissions & (QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup);
       bool others  = permissions & (QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
       if (owner && !group && !others)
       {
           ret = true;
       }
    }
    return ret;
}


bool QTrashDir::createUserDir(const QString &dir) const
{  
    QFileInfo d(dir);
    bool ret = false;
    if ((d.exists() && d.isDir()) || QDir().mkpath(dir))
    {

        ret = QFile(dir).setPermissions(QFile::ReadOwner  |
                                        QFile::WriteOwner |
                                        QFile::ExeOwner);
    }
    return ret;
}


QString QTrashDir::homeTrash() const
{
   QString homeTrash;
   // If $XDG_DATA_HOME is either not set or empty, a default equal to
   //$HOME/.local/share should be used.
   QString xdg_home =   QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
   if (!xdg_home.isEmpty())
   {
       QString tmp(xdg_home+ QDir::separator() + QLatin1String("Trash"));
       if (validate(tmp,true))
       {
          homeTrash = tmp;
       }
   }
   return homeTrash;
}


/*!
 * \brief QTrashDir::isSharedTopDirWithStickBit
 * \param mountPoint this is the root (mount point directory)
 * \return true it the mount point has a .Trash folder with stick bit set
 */
bool QTrashDir::isMountPointSharedWithStickBit(const QString &mountPoint) const
{
   bool ret = false;;
   QFileInfo trashDir(mountPoint + QDir::separator() + ".Trash");
#if DEBUG_MESSAGES
          qDebug() << Q_FUNC_INFO << trashDir.absoluteFilePath() ;
#endif
   if (trashDir.isDir() && !trashDir.isSymLink() && trashDir.isWritable())
   {
        //QFileInfo does not work for stick bit, using stat() instead
        struct stat  vfs;
        if (::stat(QFile::encodeName(trashDir.absoluteFilePath()).constData(), &vfs) == 0)
        {
          ret =  vfs.st_mode  & S_ISVTX;
#if DEBUG_MESSAGES
          qDebug() << Q_FUNC_INFO << trashDir.absoluteFilePath()
                   << QString::number(vfs.st_mode, 8)
                   << QString::number(vfs.st_mode, 16) << "ret" << ret;
#endif
       }
   }
   return ret;
}


QString QTrashDir::getMountPoint(const QString &fileOrDir) const
{
   unsigned long   fileOrDirFsId = 0xffff;
   unsigned long   parentFsId    = 0xffff;
   struct statvfs  vfs;
   QString         ret;
   QFileInfo       finfo(fileOrDir);

   if ( finfo.exists() &&
        ::statvfs( QFile::encodeName(finfo.canonicalFilePath()).constData(), &vfs) == 0 )
   {
       parentFsId =  fileOrDirFsId =  vfs.f_fsid;
       while (!finfo.isRoot() && fileOrDirFsId == parentFsId)
       {
           finfo.setFile(finfo.canonicalPath());
           if ( ::statvfs( QFile::encodeName(finfo.canonicalPath()).constData(), &vfs) == 0 )
           {
               parentFsId =  vfs.f_fsid;
           }
       }
       ret = finfo.canonicalFilePath();
   }
   return ret;
}




/*!
 * \brief QTrashDir::mountedPoints()
 * \return a list of mounted directories from /etc/mtab
 *
 * \note
 */
QStringList QTrashDir::mountedPoints() const
{
    QStringList mountPoints;
    QFile  mtab(QLatin1String("/etc/mtab"));
    if (mtab.open(QFile::ReadOnly))
    {
        QString line = mtab.readLine();
        while (!line.isEmpty())
        {
            QStringList items = line.split(QLatin1Char(' '));
            if (items.count() > 2)
            {
               if (items.first() != (QLatin1String("proc")) &&
                   items.first() != (QLatin1String("sysfs")) &&
                   items.first() != (QLatin1String("none")) &&
                   items.first() != (QLatin1String("udev")) &&
                   items.first() != (QLatin1String("devpts")) &&
                   items.first() != (QLatin1String("tmpfs")) &&
                   items.first() != (QLatin1String("systemd"))
                  )
               {
                   mountPoints.append(items.at(1));
               }
            }
            line = mtab.readLine();
        }
        mtab.close();
        ::qSort(mountPoints);
    }
#if DEBUG_MESSAGES
          qDebug() << Q_FUNC_INFO << "mount points:" << mountPoints;
#endif
    return mountPoints;
}


QStringList QTrashDir::allTrashes() const
{
    QStringList trashes;
    QString    trashDir(homeTrash());
    if (!trashDir.isEmpty())
    {
        trashes.append(trashDir);
    }
    QStringList mounted = mountedPoints();
    foreach(const QString& mp, mounted)
    {
       trashDir = getSharedTopTrashDir(mp);
       if (!trashDir.isEmpty())
       {
           trashes.append(trashDir);
       }
       trashDir = getSingleTopTrashDir(mp);
       if (!trashDir.isEmpty())
       {
           trashes.append(trashDir);
       }
    }
#if DEBUG_MESSAGES
          qDebug() << Q_FUNC_INFO << "trashes:" << trashes;
#endif
    return trashes;
}


QString QTrashDir::getSuitableTopTrashDir(const QString &mountPoint) const
{
    QString trashDir(getSharedTopTrashDir(mountPoint));
    //if previous shared mountPoint/Trash/$uid failed
    //try  mountPoint/Trash-$uid
    if (trashDir.isEmpty())
    {
        trashDir = getSingleTopTrashDir(mountPoint, true);
    }
    return trashDir;
}


QString QTrashDir::getSharedTopTrashDir(const QString &mountPoint) const
{
    QString trashDir;
    QString userTrash(mountPoint + QDir::separator() +
                      QLatin1Literal(".Trash"));
    if (isMountPointSharedWithStickBit(mountPoint))
    {
        QString userInsideSharedTrash(userTrash +
                                     QDir::separator() +
                                     QString::number(m_userId)
                                     );
        if (validate(userInsideSharedTrash, true))
        {
            trashDir = userInsideSharedTrash;
        }
    }
    return trashDir;
}


QString QTrashDir::getSingleTopTrashDir(const QString &mountPoint, bool create ) const
{
    QString trashDir;
    QString userTrash(mountPoint + QDir::separator() +
                      QLatin1Literal(".Trash"));
    userTrash += QLatin1Char('-') + QString::number(m_userId);
    if (validate(userTrash, create))
    {
           trashDir = userTrash;
    }
    return trashDir;
}


QString QTrashDir::suitableTrash(const QString &fullPathName) const
{
    QFileInfo fi(fullPathName);
    QString trashDir;
    QString homeTrashDir(homeTrash());
    if (fi.exists())
    {
         //first case file/dir is under other directory than Home
         if (!fi.canonicalPath().startsWith(QDir::homePath()))
         {
             QString topDir = getMountPoint(fi.canonicalFilePath());
             trashDir  = getSuitableTopTrashDir(topDir);
             //check if the file/dir intended to be moved into Trash does belong to Trash Dir itself
             if (!trashDir.isEmpty() &&
                  fi.canonicalFilePath().startsWith(trashDir)
                )
             {
                 trashDir.clear();
             }
         }
         //check AGAIN if the file/dir intended to be moved into Trash does belong to Trash Dir itself
         if ( trashDir.isEmpty() &&
              fi.canonicalFilePath() != QDir::homePath() &&
              !fi.canonicalFilePath().startsWith(homeTrashDir)
              )
         {
             trashDir = homeTrashDir;
         }
    }
    return trashDir;
}


bool QTrashDir::suitableTrash(const QString &fullPathName, QTrashUtilInfo &fullInfo) const
{
    fullInfo.setInfo(suitableTrash(fullPathName), fullPathName);
    return fullInfo.isValid();
}



