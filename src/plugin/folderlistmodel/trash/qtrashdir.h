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
 * File: qtrashcan.h
 * Date: 06/02/2014
 */

#ifndef QTRASHDIR_H
#define QTRASHDIR_H

#include <QStringList>
#include <QDateTime>
struct QTrashUtilInfo;

/*!
 * \brief The QTrashDir class is a Qt implementation of the Trash Directories specification
 *
 *  It creates/gets only the Trash Directories
 *  freedesktop.org specification \link http://http://standards.freedesktop.org/trash-spec/trashspec-0.8.html
 *
 *  1. Trash Dir  is a user folder (not symlink) available only to the user, it  contains:
 *       \li \a files folder \see filesTrashDir()
 *       \li \a info  folder \see infoTrashDir()
 * \code
 *   drwx------  2 devubuntu dev 4096 Fev  6 11:48 files
 *   drwx------  2 devubuntu dev 4096 Fev  6 11:48 info
 * \endcode
 *
 *  2. Files under home moved to Trash go to Home Trash Dir, that can be:
 *
 *     2.1 $XDG_DATA_HOME/Trash      (when exists and it is writable)
 *     2.2 $HOME/.local/share/Trash  (created when it does not exist)
 *
 *  3. Other files (not under user home) moved to Trash, must try to use/create
 *     Trash Dir from the its topdir (mount point), in the following order:
 *
 *     3.1. $topdir/.Trash/$uid  (when  $topdir/.Trash exists, is writable and has the stick bit set)
 *     3.2. $topdir/Trash-$uid   (when $topdir is writable)
 *
 *  \ref homeTrash()       returns the current Home Trash Dir for the user either 2.1 or 2.2
 *  \ref allTrashes()      returns all existent Trash Dir for the user 2.1, 2.2, 3.1 and 3.2
 *  \ref suitableTrash()   returns a Trash Dir for a file/dir in order to send it the Trash
 *
 *  \note
 *       Trash Location can be considered a set of available Trashes Dir to the user,
 *       a File Browser must consider all them, every time a File Browser points to Trsh Location
 *       \ref allTrashes() must be called to get the current mounted file systems and then browse any
 *        trashDir/files and check trashDir/info.
 *
 */

class QTrashDir
{   
public:
    explicit QTrashDir();

    /*!
     * \brief homeTrash() gets/crates the curret Home Trash Dir
     * \return the Trash Dir, it can be empty when it cannot be created
     */
    QString       homeTrash() const;

    /*!
     * \brief allTrashes() makes a list of all Trashes avaialable to the user
     *
     * An application which intends to list all files in the Trash Can must fetch all files
     * in the "files" directory for all items in the Trash Dir list.
     *
     * \note  It creates only Home Trash Dir when they do not exist
     *
     * \return a list of all Trashes for the current user.
     */
    QStringList   allTrashes() const;

    /*!
     * \brief suitableTrash() gets/creates a Trash Dir for a such file/dir
     * \param fullPathName  a file or dir intended to be sent to Trash
     * \return the Trash Dir, it can be empty when it cannot be created
     */
    QString       suitableTrash(const QString &fullPathName) const;

    bool          suitableTrash(const QString &fullPathName, QTrashUtilInfo& fullInfo) const;


private:          
    bool          validate(const QString& trashDir, bool create=false) const;
    bool          isMountPointSharedWithStickBit(const QString& mountPoint) const;
    bool          checkUserDirPermissions(const QString& dir) const;
    bool          createUserDir(const QString& dir) const;
    QStringList   mountedPoints() const;
    QString       getMountPoint(const QString& fileOrDir) const;
    QString       getSuitableTopTrashDir(const QString& mountPoint) const;
    QString       getSharedTopTrashDir(const QString& mountPoint) const;
    QString       getSingleTopTrashDir(const QString& mountPoint, bool create = false) const;

private:
    uint          m_userId;
#if defined(REGRESSION_TEST_FOLDERLISTMODEL) //used in Unit/Regression tests
   friend class TestDirModel;
#endif
};

#endif // QTRASHDIR_H
