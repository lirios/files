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
 * File: tempfiles.h
 * Date: 4/2/2013
 */

#ifndef TEMPFILES_H
#define TEMPFILES_H

#include <QStringList>

/*!
 * \brief The TempFiles class easily creates some temporary files
 *
 *  The current path starts with QDir::tempPath()
 *
 *  By calling \ref addSubDirLevel() a directory is created under current path
 *
 *  The \ref create() is used to create many files under current path
 *
 * removeAll() removes all files but not any directory created by addSubDirLevel()
 */

class TempFiles
{
public:
    TempFiles();
    bool addSubDirLevel(const QString&dir);
    bool create(int counter =1);
    bool create(const QString& name, int counter = 1);
    bool touch(int counter =1);
    bool touch(const QString& name, int counter = 1);
    QString lastFileCreated();
    QString lastNameCreated();
    QStringList createdList() const   { return m_filesCreated; }
    int      created()        const   { return m_filesCreated.count();}
    int      howManyExist();
    void removeAll();
    QStringList createdNames();
    QString   lastPath()       const { return m_dir; }
private:
    bool    createPrivate(const QString& name, int counter, bool putContent) ;
private:
    QString       m_dir;
    QStringList   m_filesCreated;
    QByteArray    m_content;
};


/*!
 * \brief The DeepDir class creates a directory tree under QDir::tempPath()
 *
 * The constructor receives the name of the first directory and the level of the tree
 * each subdirectory will have two files
 *
 * By calling \ref remove() the whole tree is removed.
 */
class DeepDir
{
public:
    DeepDir(const QString& rootDir, int level);
    ~ DeepDir()
    {
        remove();
    }
    bool remove();
    QString path()       const  { return root;}
    QString firstLevel() const  { return firstDirLevel; }
    QString lastLevel()  const  { return lastDirLevel; }
    int     filesCreated() const { return totalFiles;}
    int     itemsCreated() const { return totalItems;}
private:
    QString  root;
    QString  firstDirLevel;
    QString  lastDirLevel;
    int      totalFiles;
    int      totalItems;
};

#endif // TEMPFILES_H
