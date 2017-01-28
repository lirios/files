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
 * File: trashiteminfo.h
 * Date: 05/03/2014
 */

#ifndef TRASHITEMINFO_H
#define TRASHITEMINFO_H

#include "diriteminfo.h"


/*!
 * \brief The TrashItemInfo class provides a QFileInfo like information for files in Trash
 *
 *  Basically it differs from DirItemInfo in the field \a d_ptr->_normalizedPath, it must store the
 *  url like trash:///Item, while the field d_ptr->_path stores the current path in the file system as usual.
 *
 *  So suppose a Item in the trash:
 *  \li \ref absoluteFilePath()  returns like /home/user/.local/share/Trash/files/Item
 *  \li \ref urlPath()           returns      trash:///Item
 *  \li \ref getTrashDir()  does a right-to-left comparing in order to find out the Trash Dir, in this case /home/user/.local/share/Trash/files
 *
 *  The constructor \ref TrashItemInfo(const QString& urlPath) is used only to store the logical root trash folder trash:///
 */
class TrashItemInfo : public DirItemInfo
{
public:
    TrashItemInfo(const QString& urlPath);
    TrashItemInfo(const QString& trashPath, const QString& urlPath);   
    TrashItemInfo(const TrashItemInfo &other);
public:
    virtual TrashItemInfo& operator=(const DirItemInfo &other);
    virtual TrashItemInfo& operator=(const TrashItemInfo &other);
public:
    QString getTrashDir() const;
    QString getRootTrashDir() const;
private:
    void    setRoot();
    void    init(const QString& trashPath);
};

#endif // TRASHITEMINFO_H
