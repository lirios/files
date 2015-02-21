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
 * File: actionprogress.cpp
 * Date: 30/12/2014
 */

#include "actionprogress.h"
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

ActionProgress::ActionProgress(QWidget *parent) :QDialog(parent)
{
    m_pbar = new QProgressBar(this);
    m_pbar->setMaximum(100);
    m_pbar->setMinimum(0);
    QPushButton *btn = new QPushButton("Cancel", this);
    connect(btn, SIGNAL(clicked()), this, SIGNAL(cancel()));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_pbar);
    layout->addWidget(btn);
}


void ActionProgress::setValue(int v )
{
    m_pbar->setValue(v);
}

void ActionProgress::reset()
{
   m_pbar->reset();
}
