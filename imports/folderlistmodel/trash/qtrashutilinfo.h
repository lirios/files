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
 * File: qtrashutilinfo.h
 * Date: 16/03/2014
 */

#ifndef QTRASHUTILINFO_H
#define QTRASHUTILINFO_H

#include <QString>


/*!
 * \brief The QTrashUtilInfo struct just provides helper functions and information using a file structure like
 *
 *  Trash/
 *        files/           info/
 *              item           item.trashinfo
 */
struct QTrashUtilInfo
{
public:
    /*!
     * \brief setInfo()          build the whole Trash information
     * \param trashRootDir       the root Trash Dir usually a folder "Trash"
     * \param filename           the item, it can be either a relative file name or a full path name
     */
    void setInfo(const QString& trashRootDir, const QString& filename);

    /*!
     * \brief setInfoFromTrashItem() build the whole Trash information from the absolute path name from a trash item
     * \param absTrashItem       the full path, something like <TrashRoot>/files/item
     *
     *  The item pointed by \a absTrashItem does not need to exist
     */
    void setInfoFromTrashItem(const QString& absTrashItem);
    void clear();
    bool existsInfoFile();
    bool existsFile();
    bool isValid();
    QString getOriginalPathName();
    bool createTrashInfoFile(const QString& orignalPathname);
    bool removeTrashInfoFile();

    /*!
     * \brief filesTrashDir() gets the "files" directory under Trash Dir
     * \param trashDir
     * \return trashDir/files
     */
    static QString       filesTrashDir(const QString& trashDir);

    /*!
     * \brief infoTrashDir() gets gets the "info" directory under Trash Dir
     * \param trashDir
     * \return trashDir/info
     */
    static QString       infoTrashDir(const QString& trashDir);

    QString  trashRoot; // root
    QString  filesDir;  // root/files
    QString  absFile;   // root/files/item
    QString  infoDir;   // root/info
    QString  absInfo;   // root/info/item.trashinfo
    bool     valid;
};

#endif // QTRASHUTILINFO_H
