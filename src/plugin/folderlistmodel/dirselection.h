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
 * File: dirselection.h
 * Date: 29/01/2014
 */

#ifndef DIRSELECTION_H
#define DIRSELECTION_H

#include "diriteminfo.h"

#include <QObject>
#include <QStringList>


class DirItemAbstractListModel;

class DirSelection : public QObject
{
    Q_OBJECT
public:
    explicit DirSelection(DirItemAbstractListModel *parent,  DirItemInfoList *listItems);
    explicit DirSelection(QObject *parent = 0);

public slots:
        void        selectRange(int indexClicked);
        void        selectAll();
        void        clear();      
        void        toggleIndex(int index);     
        void        setIndex(int index, bool selected);
        void        setMultiSelection(bool enable);

public:
        Q_ENUMS(Mode)
        enum Mode
        {
            Single,
            Multi
        };
        Q_PROPERTY(int counter   READ counter   NOTIFY selectionChanged)
        Q_PROPERTY(Mode mode  READ mode WRITE setMode NOTIFY modeChanged)
        Q_INVOKABLE  QStringList selectedNames()      const;
        Q_INVOKABLE  void        setMode(Mode m);
        Q_INVOKABLE  QStringList selectedAbsFilePaths()  const;   //full path
        Q_INVOKABLE  QList<int>  selectedIndexes()    const;
        int                      counter()            const;
        Mode                     mode()               const;

public:
       /*!
        *   It allows to pass Control Modifiers directly to perform the  most common selection behaviour.
        *
        *   Usage Example:
        *        \li  1 When selecting an item with Shit key pressed it selects the rage calling \ref selectRange()
        *        \li  2 When selecting an item with Crtl key pressed it temporarily forces Multi Selection Mode
        *             calling \ref toggleIndex() instead of \ref setIndex();
        *
        * \param range when true it calls \ref selectRange() and does not consider the \a multiSelection parameter
        *
        * \param multiSelection when \a false it respects the current selection mode: calls \ref setIndex()
        *          for \ref Single selection mode or \ref toggleIndex() for \ref Multi selection mode.
        *          When \a true it calls \ref toggleIndex()
        *
        * QML example:
        *  \code
        *       property FolderListSelection selectionManager: pageModel.selectionObject()
        *       ...
        *
        *       MouseArea   {
        *           anchors.fill: parent
        *           onClicked: {
        *               selectionManager.select(model.index,
        *                                       (mouse.modifiers & Qt.ShiftModifier),
        *                                       (mouse.modifiers & Qt.ControlModifier) );
        *           }
        *       }
        *  \endcode
        *
        */
        Q_INVOKABLE  void        select(int index, bool range, bool multiSelection );

public:
        void        itemGoingToBeRemoved(const DirItemInfo& item);
        void        itemGoingToBeReplaced(const DirItemInfo& oldItemInfo, const DirItemInfo& newItemInfo);

private:      
        bool        priv_clear();
        void        notifyChanges();
        bool        priv_setIndex(int index, bool selected);

signals:
        void        selectionChanged(int);
        void        modeChanged(int);

private:
        int                        m_selectedCounter;
        DirItemAbstractListModel*  m_model;
        DirItemInfoList *          m_listItems;
        Mode                       m_mode;       
        int                        m_lastSelectedItem;
};

#endif // DIRSELECTION_H
