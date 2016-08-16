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
 * File: simpleslots.cpp
 * Date: 21/04/2014
 */


#include "simplelist.h"
#include "ui_simplelist.h"
#include "dirmodel.h"
#include "dirselection.h"
#include "placesmodel.h"
#include "terminalfolderapp.h"
#include "actionprogress.h"

#include <QDir>
#include <QMetaType>
#include <QHeaderView>
#include <QDebug>
#include <QProgressBar>
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QMouseEvent>


void SimpleList::onRowClicked(QModelIndex index)
{
    if (index.isValid())
    {
        m_selection->select(index.row(), m_holdingShiftKey, m_holdingCtrlKey);
    }
}


void SimpleList::onNewDir()
{

}

void SimpleList::onRename()
{

}



void SimpleList::onSetSort(int col, Qt::SortOrder order)
{
    if (col == 0 || col == 2)
    {
        if (col == 0)
        {
            m_model->setSortBy(DirModel::SortByName);
        }
        else
        {
            m_model->setSortBy(DirModel::SortByDate);
        }
        DirModel::SortOrder o = (DirModel::SortOrder)order;
        m_model->setSortOrder(o);
    }
}


void SimpleList::onClipboardChanged()
{
    int clipboardCounter = m_model->getClipboardUrlsCounter();
    int rows             = m_model->rowCount();
    statusBar()->showMessage(QString("Total Items = %1 Clipboard Items = %2")
                             .arg(rows)
                             .arg(clipboardCounter)
                             );
    ui->actionPaste->setEnabled(clipboardCounter > 0);
}


void SimpleList::onStatusChanged()
{
    if (!m_model->awaitingResults())
    {
        onClipboardChanged();
    }
}


void SimpleList::onProgress(int cur, int total, int percent)
{
    QString p;
    m_pbar->setValue(percent);
    if (cur == 0 && percent == 0)
    {
        m_pbar->reset();
        m_pbar->show();
    }
    else
        if (percent == 100)
        {
            QTimer::singleShot(200, m_pbar, SLOT(hide()));
        }
    p.sprintf("progress(cur=%d, total=%d, percent=%d)", cur,total,percent);
    qDebug() << p;
}


void SimpleList::onError(QString title, QString message)
{
    if (m_pbar)
    {
        m_pbar->hide();
    }
    QMessageBox::critical(this, title, message);
}


void SimpleList::onOpenItem(QModelIndex index)
{
    if (index.isValid())
    {

        if (!m_model->openIndex(index.row()))
        {
            QModelIndex idx = m_model->index(index.row(), 0);
            QString item = m_model->data(idx).toString();
            onError("Could not open item index", item);
        }
    }
}


void SimpleList::onPathChanged(QString path)
{
    int index = ui->comboBoxPath->findText(path);
    if (index == -1)
    {
        ui->comboBoxPath->insertItem(0, path);
        ui->comboBoxPath->setCurrentIndex(0);
    }
    else
    {
        ui->comboBoxPath->setCurrentIndex(index);
    }
    this->setWindowTitle(path);
    allowTrashActions(false);
}


void SimpleList::onPathChoosedFromList(int row)
{
    m_model->setPath(ui->comboBoxPath->itemText(row));
}


void SimpleList::onPathComboEdited()
{
    m_model->openPath(ui->comboBoxPath->lineEdit()->text());
}


void SimpleList::onSelectionChanged(int itemsCounter)
{
   allowSelectedActions(itemsCounter);
}


void SimpleList::onPlacesClicked(QModelIndex index)
{
    m_model->setPath(m_placesModel->pathFrom(index.row()));
}


void SimpleList::onOpenTerminal()
{
    QString curPath = m_model->path();
    if (!curPath.isEmpty() && !curPath.startsWith("trash:/"))
    {
        TerminalFolderApp terminal;
        terminal.openTerminal(curPath);
    }
}


void SimpleList::onCancelAction()
{
    m_model->cancelAction();
    m_pbar->hide();
}
