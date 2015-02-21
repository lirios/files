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
 * File: placesmodel.h
 * Date: 21/04/2014
 */

#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QAbstractListModel>
#include <QIcon>

class PlacesModel : public QAbstractListModel
{
    Q_OBJECT
public:   
    explicit PlacesModel(QObject *parent = 0);

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int      rowCount(const QModelIndex &) const
    { return m_places.count(); }

    QString pathFrom(int row) const;

private:
    void addPlace(const QString& n,
                  const QString& i,
                  const QStringList& u);

private:
    struct Place
    {
    public:
        Place(const QString&n, const QString& i):
           name(n),
           icon(i)
        {

        }
        QString name;
        QIcon   icon;
        QString urlPath;
    };

    QList<Place> m_places;
};

#endif // PLACESMODEL_H
