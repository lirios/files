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
 * File: fmutil.cpp
 * Date: 29/01/2014
 */

#include "fmutil.h"

#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

bool FMUtil::m_triedThemeName = false;

FMUtil::FMUtil()
{
}


/*!
 * \brief FMUtil::setThemeName() tries to set a theme name in order to get icons
 */
void FMUtil::setThemeName()
{
    QString name;
    //set saying we have tried to set ThemeName
    m_triedThemeName = true;
    QLatin1String  ubuntu_mobileTheme("ubuntu-mobile");
    QStringList paths(QIcon::themeSearchPaths());
#if defined(Q_OS_UNIX)
    if (paths.isEmpty())
    {
        paths.append(QLatin1String("/usr/share/icons"));
    }
#endif
    foreach (const QString&  dir, paths)
    {
        QDir D(dir);
        if (D.exists())
        {
#if DEBUG_MESSAGES
      qDebug() << Q_FUNC_INFO << "trying theme on Dir" << D.path();
#endif
           QFileInfoList inf =  D.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::System );
           int counter = inf.count();
           //specific names
           while (counter--)
           {
               if (inf.at(counter).fileName() == ubuntu_mobileTheme)
               {
                   if (testThemeName(ubuntu_mobileTheme))
                   {
                       return;
                   }
                   else
                   {
                       inf.removeAt(counter);
                   }
               }
           }
           //try symlinks
           counter = inf.count();
           while (counter--)
           {
               if (inf.at(counter).isSymLink())
               {
                   if (testThemeName(inf.at(counter).fileName()))
                   {
                       return;
                   }
                   else
                   {
                       inf.removeAt(counter);
                   }
               }
           }
           //try common directories
           counter = inf.count();
           while (counter--)
           {
               if (testThemeName(inf.at(counter).fileName()))
               {
                   return;
               }
           }
        }
    }
    name.clear();
    QIcon::setThemeName(name);
}


bool FMUtil::testThemeName(const QString& themeName)
{
    QMimeDatabase mimeBase;
    QStringList mimesToTest = QStringList()
                             << "text/plain"
                             << "inode/directory"
                             << "application/pdf"
                             << "application/postscript"
                             << "application/x-gzip";

    QIcon::setThemeName(themeName);
    bool hasTheme = true;
    int counter = mimesToTest.count();
    while(hasTheme  && counter--)
    {
        QMimeType mimetype = mimeBase.mimeTypeForName(mimesToTest.at(counter));
        hasTheme = QIcon::hasThemeIcon( mimetype.iconName() ) ||
                   QIcon::hasThemeIcon( mimetype.genericIconName() ) ;
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "trying theme name" << themeName << "ret=" << hasTheme;
#endif
    return hasTheme;
}
