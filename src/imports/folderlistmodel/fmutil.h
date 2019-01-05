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
 * File: fmutil.h
 * Date: 29/01/2014
 */

#ifndef FMUTIL_H
#define FMUTIL_H

#include <QStringList>
#include <QMimeType>
#include <QMimeDatabase>

/*!
 * \brief The FMUtil class  provides some utitlities
 */
class FMUtil
{
public:
    static void           setThemeName();
    static inline bool    hasTriedThemeName()  { return m_triedThemeName; }

private:
    FMUtil();
    static bool           testThemeName(const QString& themeName);

private:
    static bool           m_triedThemeName;
};

#endif // FMUTIL_H
