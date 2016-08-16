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
 * File: trashiteminfo.cpp
 * Date: 05/03/2014
 */

#include "trashiteminfo.h"
#include "locationurl.h"


TrashItemInfo::TrashItemInfo(const QString &urlPath)
    : DirItemInfo()  
{  
   d_ptr->_path            = urlPath;
   d_ptr->_isLocal         = true;
   d_ptr->_normalizedPath  = urlPath;
   if (urlPath == LocationUrl::TrashRootURL)
   {
      setRoot();
   }
}


TrashItemInfo::TrashItemInfo(const TrashItemInfo &other)
       : DirItemInfo(other)       
{
}


TrashItemInfo::TrashItemInfo(const QString& trashPath, const QString &urlPath)
   : DirItemInfo(urlPath)  
{
    init(trashPath);
}


void TrashItemInfo::setRoot()
{
    d_ptr->_isValid      = true;
    d_ptr->_isRoot       = true;
    d_ptr->_isDir        = true;
    d_ptr->_isReadable   = true;
    d_ptr->_isExecutable = true;
    d_ptr->_exists       = true;
    d_ptr->_fileName.clear();
}


void TrashItemInfo::init(const QString& trashPath)
{
    if (trashPath == absoluteFilePath())
    {
        d_ptr->_path = trashPath;
        setRoot();
    }
    else
    {
        if (!d_ptr->_path.startsWith(trashPath))
        {
            d_ptr->_isValid = false;
        }
    }
    QString abs(d_ptr->_path);
    d_ptr->_normalizedPath = abs.replace(0,trashPath.length()+1, LocationUrl::TrashRootURL);
}


TrashItemInfo& TrashItemInfo::operator=(const DirItemInfo &other)
{
    DirItemInfo::operator  = (other);
    //the following code is disabled because TrashItemInfo does not define any data member
    //but it is kept for the case any specific data member be necessary in the future
    //and it is also kept to warn that doing so would cause a bug
#if 0
    const TrashItemInfo *isTrash = dynamic_cast<const TrashItemInfo*> (&other);   
    if (isTrash)
    {
      //copy data specific to this class here,
      //do not unnecessarily copy data that is handled by parent's assignment operator
    }
#endif
    return *this;
}


TrashItemInfo& TrashItemInfo::operator=(const TrashItemInfo &other)
{
    DirItemInfo::operator = (other);   
    return *this;
}


/*!
 * \brief TrashItemInfo::getTrashDir()
 *
 *  Lets suppose a directory in the trash named DIR:
 *    absFilePath()        = /home/user/.local/share/Trash/files/DIR
 *    normalizedFilePath() =                            trash:///DIR
 *
 *    The trash dir is /home/user/.local/share/Trash/files
 *
 * \return The trash dir
 */
QString TrashItemInfo::getTrashDir() const
{
    QString trashDir;
    QString norm(urlPath());
    if (    norm.length() > LocationUrl::TrashRootURL.length()
         && norm.startsWith(LocationUrl::TrashRootURL)
       )
    {
        QStringRef  trashItemRef(norm.midRef(LocationUrl::TrashRootURL.length()));
        QString abs(absoluteFilePath());
        int length = abs.lastIndexOf(trashItemRef);
        if (length > 0)
        {
            trashDir = abs.left(length-1);
        }
    }
    return trashDir;
}


QString TrashItemInfo::getRootTrashDir() const
{
    QString ret = getTrashDir();
    if (!isRoot())
    {
        ret = QFileInfo(ret).absolutePath();
    }
    return ret;
}
