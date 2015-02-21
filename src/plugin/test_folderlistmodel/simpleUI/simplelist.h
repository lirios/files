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
 * File: simplelist.h
 * Date: 3/9/2013
 */

#ifndef SIMPLELIST_H
#define SIMPLELIST_H

#include <QMainWindow>
#include <QModelIndex>

class DirModel;
class QProgressBar;
class DirSelection;
class PlacesModel;
class ActionProgress;

namespace Ui {
  class SimpleList;
}


class SimpleList : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SimpleList(QWidget *parent = 0);
    ~SimpleList();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void  allowSelectedActions(int selectedCounter);
    void  allowTrashActions(bool enable);
    void  do_connections();

private:
    Ui::SimpleList *ui;
    DirModel       *m_model;   
    ActionProgress * m_pbar;
    DirSelection  * m_selection;
    bool            m_holdingCtrlKey;
    bool            m_holdingShiftKey;
    Qt::MouseButton m_button;
    PlacesModel   * m_placesModel;

private slots:   
    void   onNewDir();
    void   onRename();   
    void   onRowClicked(QModelIndex);
    void   onOpenItem(QModelIndex index);
    void   onSetSort(int col, Qt::SortOrder order);
    void   onProgress(int, int,int);
    void   onClipboardChanged();
    void   onError(QString title, QString message);
    void   onPathChanged(QString path);
    void   onPathChoosedFromList(int);
    void   onPathComboEdited();
    void   onSelectionChanged(int);
    void   onPlacesClicked(QModelIndex);
    void   onOpenTerminal();
    void   onCancelAction();
    void   onStatusChanged();
};

#endif // SIMPLELIST_H
