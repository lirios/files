/**************************************************************************
 *
 * Copyright 2014 Canonical Ltd.
 * Copyright 2014 Carlos J Mazieri <carlos.mazieri@gmail.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * File: clipboard.cpp
 * Date: 1/22/2014
 */

#include "clipboard.h"

#include <QClipboard>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

static  QLatin1String GNOME_COPIED_MIME_TYPE  ("x-special/gnome-copied-files");
static  QLatin1String KDE_CUT_MIME_TYPE       ("application/x-kde-cutselection");


int DirModelMimeData::m_instances = 0;
DirModelMimeData*  DirModelMimeData::m_globalMimeData = 0;


bool DirModelMimeData::hasFormat ( const QString & mimeType ) const
{
   bool ret = false;
   if (  mimeType == KDE_CUT_MIME_TYPE  )
   {
      ret = true;
   }
   else
   {
      ret = m_formats.contains(mimeType);
   }
   return ret;
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::DirModelMimeData
 */
DirModelMimeData::DirModelMimeData() :
    QMimeData()
  , m_appMime(0)
{
    m_formats.append("text/uri-list");
    m_formats.append(GNOME_COPIED_MIME_TYPE);
    m_formats.append("text/plain");
    m_formats.append("COMPOUND_TEXT");
    m_formats.append("TARGETS");
    m_formats.append("MULTIPLE");
    m_formats.append("TIMESTAMP");
    m_formats.append("SAVE_TARGETS");

    ++m_instances;
#if DEBUG_MESSAGES
     qDebug() << Q_FUNC_INFO << this << "instances" << m_instances;
#endif
}




DirModelMimeData::~DirModelMimeData()
{
    --m_instances;
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << this  << "instances" << m_instances
             << "m_globalMimeData" << m_globalMimeData;
#endif
    if (m_instances == 1 && m_globalMimeData)
    {
        DirModelMimeData * tmp = m_globalMimeData;
        m_globalMimeData = 0;
        delete tmp;
    }
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::gnomeUrls
 * \param mime
 * \param operation
 * \return
 */
QList<QUrl>
DirModelMimeData::gnomeUrls(const QMimeData * mime,
                            ClipboardOperation& operation)
{
    QList<QUrl>  urls;
    if (mime->hasFormat(GNOME_COPIED_MIME_TYPE))
    {
       QByteArray  bytes = mime->data(GNOME_COPIED_MIME_TYPE);
       QList<QString>  d = QString(bytes).split(QLatin1String("\n"),
                                                QString::SkipEmptyParts);
       operation = ClipboardCopy;
       if (d.count() > 0)
       {
           if (d.at(0).trimmed().startsWith(QLatin1String("cut")))
           {
               operation = ClipboardCut;
           }
           for (int counter= 1; counter < d.count(); counter++)
           {
               urls.append(d.at(counter).trimmed());
           }
       }
    }
    return urls;
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::clipBoardOperation()
 * \param mime
 * \return
 */
ClipboardOperation DirModelMimeData::clipBoardOperation()
{
    ClipboardOperation op = ClipboardCopy;
    m_appMime = clipboardMimeData();
    if (m_appMime)
    {
       //first check for GNOME clipboard format, op comes with Copy/Cut
        if (gnomeUrls(m_appMime, op).count() == 0)
        { // there is no gnome format, tries KDE format
            QStringList formats = m_appMime->formats();
            int f = formats.count();
            while(f--)
            {
                const QString &mi = formats.at(f);
                if(mi.startsWith(QLatin1String("application/x-kde")) )
                {
                    if (mi.contains(QLatin1String("cut")))
                    {
                        op = ClipboardCut;
                        break;
                    }
                }
            }
        }
    }
    return op;
}


//===============================================================================================
/*!
 * \brief DirModelMimeData::setIntoClipboard
 *
 *  Try to put data in the global cliboard
 *
 *  \note:
 *       On mobile devices clipboard might not work, in this case a local Clipboard is simulated
 *
 * \param files
 * \param path
 * \param isCut
 * \return who is owner of clipboard data
 */
DirModelMimeData::ClipBoardDataOwner
DirModelMimeData::setIntoClipboard(const QStringList &files, const QString& path, ClipboardOperation operation)
{
    static bool firstTime = true;
    DirModelMimeData::ClipBoardDataOwner  ret = Nobody;
    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard)
    {
        ret = Application;
        DirModelMimeData *mime = m_globalMimeData ? m_globalMimeData
                                                  : new DirModelMimeData();
        if (mime->fillClipboard(files, path, operation))
        {
            clipboard->setMimeData(mime);
            //it looks like some mobile devices does not have X or Clipboard does work for other reason
            //in this case we simulate our own clipboard, the QClipboard::dataChanged() signal is also
            //checked in \ref Clipboard::storeOnClipboard()
            if (firstTime)
            {
                firstTime = false;
                if (!m_globalMimeData && !testClipboardContent(files, path))
                {
                    qWarning() << "QClipboard does not work,  using own QMimeData storage";
                    m_globalMimeData = mime;
                }
            }
#if DEBUG_MESSAGES
            qDebug() << Q_FUNC_INFO << "mime" << mime
                     << "own Clipboard Mime Data" << m_globalMimeData;
#endif
        }
        else
            if (m_globalMimeData != mime)
            {
                delete mime;
            }
        //check if it is necessary to send notification about Clipboard changed
        if (m_globalMimeData)
        {
            ret = MySelf;
        }
    }
    return ret;
}



bool DirModelMimeData::fillClipboard(const QStringList& files, const QString &path, ClipboardOperation operation)
{
    int index = m_formats.indexOf(KDE_CUT_MIME_TYPE);
    if (index != -1 && operation != ClipboardCut)
    {
        m_formats.removeAt(index);
    }
    else
    if (operation == ClipboardCut)
    {
        m_formats.append(KDE_CUT_MIME_TYPE);
    }
    m_urls.clear();
    m_gnomeData.clear();
    m_gnomeData += operation == ClipboardCut ?
                                    QLatin1String("cut") :
                                    QLatin1String("copy");
    QStringList fullPaths = makeFullPath(files, path);
    for(int counter = 0; counter < fullPaths.count(); counter++)
    {
        QUrl item = QUrl::fromLocalFile(fullPaths.at((counter)));
        m_urls.append(item);
        m_gnomeData += QLatin1Char('\n') + item.toEncoded() ;
    }
    setData(GNOME_COPIED_MIME_TYPE, m_gnomeData);
    setUrls(m_urls);

    return true;
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::clipboardMimeData
 * \return
 */
const QMimeData *DirModelMimeData::clipboardMimeData()
{
    const QMimeData *ret = 0;
    QClipboard *clipboard = QApplication::clipboard();
    if (m_globalMimeData)
    {
        ret = m_globalMimeData;
    }
    else
    if (clipboard)
    {
        ret = clipboard->mimeData();
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "clipboard" << clipboard
                << "m_ownClipboardMimeData" << m_globalMimeData
                << "clipboard->mimeData()" << ret;
#endif
    return ret;
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::localUrls
 * \return
 */
QStringList
DirModelMimeData::localUrls(ClipboardOperation& operation)
{
     m_appMime = clipboardMimeData();
     QStringList paths;
     //it may have external urls
     if (m_appMime)
     {
         QList<QUrl> urls;
         if (m_appMime->hasUrls())
         {
             urls =  m_appMime->urls();
             operation = clipBoardOperation();
         }
         else
         {
             urls = gnomeUrls(m_appMime, operation);
         }
         for (int counter=0; counter < urls.count(); counter++)
         {
             if (urls.at(counter).toString().startsWith(QLatin1String("file://")))
             {
                 paths.append(urls.at(counter).toLocalFile());
             }
         }
     }
#if DEBUG_MESSAGES
        qDebug() << Q_FUNC_INFO << paths;
#endif
     return paths;
}


//===============================================================================================
/*!
 * \brief DirModelMimeData::testClipboardContent() Gets the clipboard content and compare with data previously stored
 * \param files
 * \param path
 * \return true if clipboard has content and it matches data previously stored
 */
bool  DirModelMimeData::testClipboardContent(const QStringList &files, const QString &path)
{
    bool ret = false;
    ClipboardOperation tmpOperation;
    QStringList expectedList = makeFullPath(files,path);
    QStringList realList     = localUrls(tmpOperation);
    if (realList == expectedList)
    {
        ret = true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "FAILED, Clipboard does not work";
    }
    return ret;
}

//===============================================================================================
/*!
 * \brief DirModelMimeData::makeFullPath() Just creates a fulpath file list when they do exist
 * \param files
 * \param path
 * \return the list itself
 */
QStringList DirModelMimeData::makeFullPath(const QStringList& files, const QString &path)
{
    QStringList fullPathnameList;
    QFileInfo fi;
    for(int counter = 0; counter < files.count(); counter++)
    {
        const QString& item = files.at(counter);
        fi.setFile(item);
        if (!fi.isAbsolute())
        {
            fi.setFile(path + QDir::separator() + item);
        }
        if (fi.exists())
        {
            fullPathnameList.append(fi.absoluteFilePath());
        }
    }
    return fullPathnameList;
}


//===========================================================================
//
//===========================================================================
Clipboard::Clipboard(QObject *parent):
    QObject(parent)
  , m_mimeData ( new DirModelMimeData() )
  , m_clipboardModifiedByOther(false)
{
    QClipboard *clipboard = QApplication::clipboard();

    connect(clipboard, SIGNAL(dataChanged()), this,    SIGNAL(clipboardChanged()));
    connect(clipboard, SIGNAL(dataChanged()), this,    SLOT(onClipboardChanged()));
}


Clipboard::~Clipboard()
{
    delete m_mimeData;
}

//================================================================================
/*!
 * \brief Clipboard::clipboardHasChanged() used to identify if the clipboard changed during a Cut operation
 *
 *  \sa \ref endCurrentAction()
 */
void Clipboard::onClipboardChanged()
{
    m_clipboardModifiedByOther = true;
}


//==================================================================
/*!
 * \brief Clipboard::storeOnClipboard() store data on Clipboard
 * \param pathnames files list
 * \param op \ref ClipboardOperation as  \ref ClipboardCopy or  \ref ClipboardCut
 *
 *  Stores data on clipboard by calling \ref DirModelMimeData::setIntoClipboard() which uses Qt class QClipboard
 *  It is expected that QClipboard class emits the dataChanged() signal when a new content is set into it,
 *  if it does we caught that signal in \ref clipboardHasChanged() which sets \ref m_clipboardModifiedByOther to true.
 */
void  Clipboard::storeOnClipboard(const QStringList &names, ClipboardOperation op, const QString& curPath)
{
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << names << "ClipboardOperation" << op;
#endif
     DirModelMimeData::ClipBoardDataOwner owner =
         m_mimeData->setIntoClipboard(names, curPath, op);
     if (owner == DirModelMimeData::MySelf || !m_clipboardModifiedByOther)
     {
         emit clipboardChanged();
     }
     m_clipboardModifiedByOther = false;
}

//===============================================================================================
/*!
 * \brief Clipboard::copy
 * \param pathnames
 */
void Clipboard::copy(const QStringList &names, const QString& path)
{
    storeOnClipboard(names, ClipboardCopy, path);
}

//===============================================================================================
/*!
 * \brief Clipboard::cut
 * \param pathnames
 */
void Clipboard::cut(const QStringList &names, const QString &path)
{
    storeOnClipboard(names, ClipboardCut, path);
}


//=======================================================
/*!
 * \brief Clipboard::clipboardLocalUrlsCounter
 * \return
 */
int Clipboard::clipboardLocalUrlsCounter()
{
    ClipboardOperation operation;
    return m_mimeData->localUrls(operation).count();
}


//=======================================================
/*!
 * \brief Clipboard::paste
 * \param operation
 * \return
 */
QStringList Clipboard::paste(ClipboardOperation &operation)
{
    QStringList items = m_mimeData->localUrls(operation);
    if (operation == ClipboardCut)
    {
        //this must still be false when cut finishes to change the clipboard to the target
        m_clipboardModifiedByOther = false;
    }
    return items;
}

/*!
 * \brief Clears clipboard entries
 */
void Clipboard::clear()
{
    qDebug() << Q_FUNC_INFO << "Clearing clipboard";
    storeOnClipboard(QStringList(), ClipboardCopy, "");
}
