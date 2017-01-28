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
 * File: clipboard.h
 * Date: 1/22/2014
 */

#ifndef  CLIPBOARD_H
#define  CLIPBOARD_H

#include <QMimeData>
#include <QUrl>
#include <QStringList>

class DirModelMimeData;

enum ClipboardOperation
{
    NoClipboard, ClipboardCopy, ClipboardCut
};



/*!
 * \brief The Clipboard class handles global clipboard storage
 */
class Clipboard : public QObject
{
   Q_OBJECT
public:
   explicit Clipboard(QObject *parent = 0);
    ~Clipboard();
    QStringList  paste(ClipboardOperation& operation);
    int          clipboardLocalUrlsCounter();
    inline bool  hasClipboardModifiedByOtherApplication() const {return m_clipboardModifiedByOther;}

public slots:
   void         cut(const QStringList&  names, const QString &path);
   void         copy(const QStringList& names, const QString &path);
   void         clear();

signals:
   void         clipboardChanged();

private slots:
   void         onClipboardChanged ();

private:
   void         storeOnClipboard(const QStringList &names,
                                 ClipboardOperation op,
                                 const QString &curPath);
private:
   DirModelMimeData  *     m_mimeData;
   bool                    m_clipboardModifiedByOther;
};



/*!
 * \brief The DirModelMimeData class is the storage on Clipboard
 */
class DirModelMimeData : public QMimeData
{
public:
    explicit DirModelMimeData();
    ~DirModelMimeData();
    virtual QStringList     formats() const    { return m_formats; }
    virtual bool            hasFormat ( const QString & mimeType ) const;

public:
    enum ClipBoardDataOwner
    {
        Nobody,    // might have failed
        Application,
        MySelf
    };

    ClipBoardDataOwner      setIntoClipboard(const QStringList& files,
                                             const QString &path,
                                             ClipboardOperation operation);
    const QMimeData *       clipboardMimeData();
    QStringList             localUrls(ClipboardOperation& operation);

private:
    static QList<QUrl>      gnomeUrls(const QMimeData *mime, ClipboardOperation& operation);
    ClipboardOperation      clipBoardOperation();
    bool                    fillClipboard(const QStringList& files, const QString &path, ClipboardOperation operation);
    QStringList             makeFullPath(const QStringList& files, const QString &path);
    bool                    testClipboardContent(const QStringList& files, const QString &path);

private:
    QStringList              m_formats;
    const QMimeData *        m_appMime;
    QByteArray               m_gnomeData;
    QList<QUrl>              m_urls;
    static DirModelMimeData* m_globalMimeData; //!< some mobile devices do not use X, they may not have clipboard
    static   int             m_instances;
};



#endif //CLIPBOARD_H

