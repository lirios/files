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
 * File: placesmodel.cpp
 * Date: 21/04/2014
 */

#include "placesmodel.h"
#include "locationurl.h"
#include <QDir>
#include <QIcon>
#include <QStandardPaths>


PlacesModel::PlacesModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QLatin1String doc("Documents");
    QLatin1String dow("Downloads");

    addPlace("Home", ":/resources/resources/home-page.png", QStringList(QDir::homePath()));

    addPlace(doc, ":/resources/resources/document_folder.png",
                QStringList() << QStandardPaths::displayName(QStandardPaths::DocumentsLocation)
                              << QDir::homePath() + QDir::separator() + doc
            );

     addPlace(dow, ":/resources/resources/downloads_folder.png",
                QStringList() << QStandardPaths::displayName(QStandardPaths::DownloadLocation)
                              << QDir::homePath() + QDir::separator() + dow
            );

    addPlace("Temp", ":/resources/resources/temp_folder.png", QStringList(QDir::tempPath()));

    addPlace("Root", ":/resources/resources/red_folder.png", QStringList(QDir::rootPath()));

    addPlace("Trash", ":/resources/resources/recyclebin_full.png", QStringList(LocationUrl::TrashRootURL));
}


QVariant PlacesModel::data(const QModelIndex &index, int role) const
{
   if ( role == Qt::DisplayRole)
   {
      return m_places.at(index.row()).name;
   }
   if (role == Qt::DecorationRole)
   {
      return m_places.at(index.row()).icon;
   }
   return QVariant();
}


QString PlacesModel::pathFrom(int row) const
{
    return m_places.at(row).urlPath;
}


void PlacesModel::addPlace(const QString &n, const QString &i, const QStringList &u)
{
    Place place(n,i);
    if (u.count() == 1)
    {
        place.urlPath  = u.at(0);
        m_places.append(place);
        return;
    }

    for (int counter = 0 ; counter < u.count();  ++counter)
    {
        if (QFileInfo(u.at(counter)).exists())
        {
           place.urlPath  = u.at(counter);
           m_places.append(place);
           return;
        }
    }
}
