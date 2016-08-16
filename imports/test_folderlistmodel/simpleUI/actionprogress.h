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
 * File: actionprogress.h
 * Date: 30/12/2014
 */

#ifndef ACTIONPROGRESS_H
#define ACTIONPROGRESS_H

#include <QDialog>

class QProgressBar;

class ActionProgress : public QDialog
{
    Q_OBJECT
public:
   ActionProgress(QWidget *parent = 0);
signals:
       void    cancel();
public slots:
   void setValue(int v);
   void reset() ;
private:
   QProgressBar * m_pbar;
};

#endif // ACTIONPROGRESS_H
