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
 * File: simplelist.cpp
 * Date: 3/9/2013
 */

#include "simplelist.h"
#include "ui_simplelist.h"
#include "dirmodel.h"
#include "dirselection.h"
#include "placesmodel.h"
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

SimpleList::SimpleList(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SimpleList),  
    m_pbar( new ActionProgress(this) ),
    m_selection(0),
    m_holdingCtrlKey(false),
    m_holdingShiftKey(false),
    m_button(Qt::NoButton),
    m_placesModel(new PlacesModel(this))
{
    //prepare UI
    ui->setupUi(this);
    ui->tableViewFM->horizontalHeader()->setSortIndicator(0,Qt::AscendingOrder);
    resize(1200,600);

    ui->listViewPlaces->setModel(m_placesModel);
    ui->listViewPlaces->setSpacing(ui->listViewPlaces->spacing() + 7);

    //create the model
    m_model = new DirModel(this);
    DirModel::registerMetaTypes();
    ui->tableViewFM->setModel(m_model);

    //enable External Disk Watcher
    m_model->setEnabledExternalFSWatcher(true);
    ui->checkBoxExtFsWatcher->setChecked(true);

    //get selection object
    m_selection = m_model->selectionObject();

    //selection is handled in the model, disable it in the view
    ui->tableViewFM->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableViewFM->viewport()->installEventFilter(this);

    //get default values from model
    ui->checkBoxShowDirs->setChecked(m_model->showDirectories());
    ui->checkBoxShowHidden->setChecked(m_model->getShowHiddenFiles());   
    ui->checkBoxShowMediaInfo->setChecked(m_model->readsMediaMetadata());
    ui->checkBoxMultiSelection->setChecked(m_selection->mode() == DirSelection::Multi);

    //start clibpboard message
    onClipboardChanged();

    //start with actions disabled,
    //onSelectionChanged() does a basic actions allowable handling
    onSelectionChanged(0);

    //connect everything
    do_connections();

    //start browsing home
    m_model->goHome(); 
}


SimpleList::~SimpleList()
{
    delete ui;
}


bool SimpleList::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->tableViewFM->viewport() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        m_holdingShiftKey = mouseEvent->modifiers() & Qt::ShiftModifier;;
        m_holdingCtrlKey  = mouseEvent->modifiers() & Qt::ControlModifier;;
        m_button          = mouseEvent->button();
    }
    return QMainWindow::eventFilter(obj, event);
}


void SimpleList::do_connections()
{
    connect(ui->tableViewFM, SIGNAL(clicked(QModelIndex)),
            this,          SLOT(onRowClicked(QModelIndex)));

    connect(ui->tableViewFM, SIGNAL(doubleClicked(QModelIndex)),
            this,          SLOT(onOpenItem(QModelIndex)));

    connect(m_model, SIGNAL(pathChanged(QString)),
            this,    SLOT(onPathChanged(QString)));

    connect(ui->toolButtonUp,     SIGNAL(clicked()),    m_model, SLOT(cdUp()));
    connect(ui->toolButtonBack,   SIGNAL(clicked()),    m_model, SLOT(goBack()));
    connect(ui->actionCopy,       SIGNAL(triggered()),  m_model, SLOT(copySelection()));
    connect(ui->actionCut,        SIGNAL(triggered()),  m_model, SLOT(cutSelection()));
    connect(ui->actionDelete,     SIGNAL(triggered()),  m_model, SLOT(removeSelection()));
    connect(ui->actionPaste,      SIGNAL(triggered()),  m_model, SLOT(paste()));
    connect(ui->actionMoveToTrash,SIGNAL(triggered()),  m_model, SLOT(moveSelectionToTrash()));
    connect(ui->actionTerminnal,  SIGNAL(triggered()),  this,    SLOT(onOpenTerminal()));

    connect(ui->actionRestoreFromTrash, SIGNAL(triggered()),
            m_model,              SLOT(restoreSelectionFromTrash()));

    connect(ui->checkBoxShowDirs,     SIGNAL(clicked(bool)), m_model, SLOT(setShowDirectories(bool)));
    connect(ui->checkBoxShowHidden,   SIGNAL(clicked(bool)), m_model, SLOT(setShowHiddenFiles(bool)));
    connect(ui->checkBoxExtFsWatcher, SIGNAL(clicked(bool)), m_model, SLOT(setEnabledExternalFSWatcher(bool)));
    connect(ui->checkBoxShowMediaInfo, SIGNAL(clicked(bool)),m_model, SLOT(setReadsMediaMetadata(bool)));
    connect(ui->checkBoxMultiSelection,SIGNAL(clicked(bool)), m_selection, SLOT(setMultiSelection(bool)));

    connect(ui->comboBoxPath,     SIGNAL(activated(int)),
            this, SLOT(onPathChoosedFromList(int)));

    connect(ui->comboBoxPath->lineEdit(),    SIGNAL(returnPressed()),
            this,    SLOT(onPathComboEdited()));

    connect(ui->tableViewFM->horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            this,                              SLOT(onSetSort(int,Qt::SortOrder)));

    connect(m_model, SIGNAL(progress(int,int,int)),
            this,    SLOT(onProgress(int,int,int)));

    connect(m_model, SIGNAL(clipboardChanged()),
            this,    SLOT(onClipboardChanged()));

    connect(m_model, SIGNAL(error(QString,QString)),
            this,    SLOT(onError(QString,QString)));

    connect(m_model, SIGNAL(awaitingResultsChanged()),
            this,    SLOT(onStatusChanged()));

    connect(m_selection,   SIGNAL(selectionChanged(int)),
            this,          SLOT(onSelectionChanged(int)));

    connect(ui->listViewPlaces,  SIGNAL(clicked(QModelIndex)),
            this,                SLOT(onPlacesClicked(QModelIndex)));

    connect(m_pbar,  SIGNAL(cancel()), this, SLOT(onCancelAction()));
}

//===================================================================

/*
 *  Simple Allowable methods, they cover not everything
 *
 *  allowSelectedActions() and allowTrashActions()
 */

void SimpleList::allowSelectedActions(int selectedCounter)
{
    bool enableActions  = selectedCounter > 0;
    ui->actionRename->setEnabled(selectedCounter == 1);
    ui->actionDelete->setEnabled(enableActions);
    ui->actionCut->setEnabled(enableActions);
    ui->actionCopy->setEnabled(enableActions);
    allowTrashActions(enableActions);
}


void SimpleList::allowTrashActions(bool enable)
{
    if (m_model->path().startsWith("trash:/"))
    {
       ui->actionEmptyTrash->setEnabled(true);
       ui->actionRestoreFromTrash->setEnabled(enable);
       ui->actionMoveToTrash->setEnabled(false);
       ui->actionNewFolder->setEnabled(false);
       ui->actionTerminnal->setEnabled(false);
    }
    else
    {
       ui->actionEmptyTrash->setEnabled(false);
       ui->actionRestoreFromTrash->setEnabled(false);
       ui->actionMoveToTrash->setEnabled(enable);
       ui->actionNewFolder->setEnabled(true);
       ui->actionTerminnal->setEnabled(true);
    }
}
