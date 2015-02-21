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
 * File: dirselection.cpp
 * Date: 29/01/2014
 */

#include "dirselection.h"
#include "diritemabstractlistmodel.h"
#include <QTimer>
#include <QDebug>


#define  VALID_INDEX(index)   (index >= 0 && index < m_model->rowCount())

DirSelection::DirSelection(QObject *parent) :  QObject(parent)
{
}

DirSelection::DirSelection(DirItemAbstractListModel *parent, DirItemInfoList *listItems) :
    QObject(parent)
   ,m_selectedCounter(0)
   ,m_model(parent)
   ,m_listItems(listItems)
   ,m_mode(Single)  
   ,m_lastSelectedItem(-1)
{
}



QStringList DirSelection::selectedAbsFilePaths() const
{
    QStringList ret;
    int counter = m_model->rowCount();
    for(int index = 0 ; index < counter; ++index)
    {
        if (m_listItems->at(index).isSelected())
        {
            ret.append(m_listItems->at(index).absoluteFilePath());
        }
    }
    return ret;
}

QStringList DirSelection::selectedNames() const
{
    QStringList ret;
    int counter = m_model->rowCount();
    for(int index = 0 ; index < counter; ++index)
    {
        if (m_listItems->at(index).isSelected())
        {
            ret.append(m_listItems->at(index).fileName());
        }
    }
    return ret;
}



QList<int>  DirSelection::selectedIndexes()    const
{
    QList<int> ret;
    int counter = m_model->rowCount();
    for(int index = 0 ; index < counter; ++index)
    {
        if (m_listItems->at(index).isSelected())
        {
            ret.append(index);
        }
    }
    return ret;
}


void DirSelection::clear()
{   
    if (priv_clear())
    {
        notifyChanges();
    }
}


bool DirSelection::priv_clear()
{
    bool notify = m_selectedCounter != 0;
    if (notify)
    {
        int counter = m_model->rowCount();
        DirItemInfo *data =  m_listItems->data();
        while (m_selectedCounter > 0  && counter-- )
        {
            if ( data[counter].setSelection(false) )
            {
                --m_selectedCounter;
                m_model->notifyItemChanged(counter);              
            }
        }
    }
    //force it to zero, works when cleaning the buffer first
    m_selectedCounter  = 0;
    m_lastSelectedItem = -1;
    return notify;
}


void DirSelection::selectAll()
{
    int counter = m_model->rowCount();
    bool notify = m_selectedCounter != counter;
    if (notify)
    {
        DirItemInfo *data =  m_listItems->data();
        while ( counter-- )
        {
            if ( data[counter].setSelection(true) )
            {
                ++m_selectedCounter;
                m_model->notifyItemChanged(counter);              
            }
        }
        notifyChanges();
    }
}


int DirSelection::counter() const
{
    return m_selectedCounter;
}


DirSelection::Mode DirSelection::mode() const
{
    return m_mode;
}


void DirSelection::itemGoingToBeRemoved(const DirItemInfo &item)
{
    if (m_selectedCounter > 0 && item.isSelected())
    {      
        --m_selectedCounter;
        notifyChanges();
    }
    // item is going to be removed, no QAbstractItemModel::dataChanged() signal is necessary to refresh views
}


void DirSelection::setIndex(int index, bool selected)
{
     if (VALID_INDEX(index))
     {
         int old_selectedCounter = m_selectedCounter;
         if (selected && m_mode == Single && m_selectedCounter > 0)
         {
             priv_clear();
         }       
         if (    priv_setIndex(index, selected)
              || old_selectedCounter != m_selectedCounter
            )
         {
             notifyChanges();
         }
     }
}


void DirSelection::toggleIndex(int index)
{
    if (VALID_INDEX(index))
    {
        setIndex(index, !m_listItems->at(index).isSelected());
    }
}


void DirSelection::setMode(Mode m)
{
    if (m != m_mode)
    {
        m_mode = m;
        emit modeChanged(m_mode);
    }
}


void DirSelection::notifyChanges()
{
    emit selectionChanged(m_selectedCounter);    
}


/*!
 * \brief DirSelection::itemGoingToBeReplaced() it is supposed to control selection writable and readabble states
 *
 *     So far it does nothing
 *
 * \param oldItemInfo
 * \param newItemInfo
 */
void DirSelection::itemGoingToBeReplaced(const DirItemInfo &oldItemInfo,
                                         const DirItemInfo &newItemInfo)
{
    if (oldItemInfo.isSelected())
    {
       // we may add selection writable state in the future
        Q_UNUSED(newItemInfo);
    }   
}


void DirSelection::selectRange(int indexClicked)
{
    bool changed = false;
    if (   VALID_INDEX(indexClicked)
        && m_selectedCounter > 0
        && indexClicked != m_lastSelectedItem
        && VALID_INDEX(m_lastSelectedItem)
        && !m_listItems->at(indexClicked).isSelected()
       )
    {
        //go from indexClicked to  m_lastSelectedItem
        int  increment = indexClicked > m_lastSelectedItem?  -1 : 1;
        int  nextItem  = indexClicked;
        int  saved_lastSelectedItem = m_lastSelectedItem;
        while (priv_setIndex(nextItem, true) && nextItem != saved_lastSelectedItem)
        {
            nextItem  += increment;
            changed    = true;
        }
    }
    if (changed)
    {
        notifyChanges();
    }
}


bool DirSelection::priv_setIndex(int index, bool selected)
{
    DirItemInfo *data  = m_listItems->data();
    bool changed = false;
    if ((changed = data[index].setSelection(selected)))
    {
        m_model->notifyItemChanged(index);
        if (selected)
        {
            ++m_selectedCounter;         
            m_lastSelectedItem = index;
        }
        else
        {
            --m_selectedCounter;          
        }
    }
    return changed;
}


void DirSelection::select(int index, bool range, bool multiSelection )
{
    if (range && VALID_INDEX(m_lastSelectedItem))
    {
        selectRange(index);
    }
    else
    {
        if (multiSelection || m_mode == Multi)
        {
            Mode saveMode = m_mode;
            //set Multi selection do not  call clear()
            m_mode = Multi;
            toggleIndex(index);
            m_mode = saveMode;
        }
        else
        {
            setIndex(index, true);
        }
    }
}


void DirSelection::setMultiSelection(bool enable)
{
    Mode m = enable ? Multi : Single;
    setMode(m);
}
