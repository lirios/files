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
 * File: diritemabstractlistmodel.h
 * Date: 30/01/2014
 */

#ifndef DIRITEMABSTRACTLISTMODEL_H
#define DIRITEMABSTRACTLISTMODEL_H

#include <QObject>

#include <QAbstractListModel>


class DirItemInfo;
class DirItemModel;

class DirItemAbstractListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    virtual int                 getIndex(const QString& name) = 0;
    virtual void                notifyItemChanged(int index)  = 0;
protected:
    explicit DirItemAbstractListModel(QObject *parent = 0) :
        QAbstractListModel(parent)
    {
    }
};

#endif // DIRITEMABSTRACTLISTMODEL_H
