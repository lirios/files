/**************************************************************************
 *
 * Copyright 2013 Canonical Ltd.
 * Copyright 2013 Carlos J Mazieri <carlos.mazieri@gmail.com>
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
 *
 * Filename: terminalfolderapp.h
 * Created : 29 Jul 2013
 **/


#ifndef TERMINALFOLDERAPP_H
#define TERMINALFOLDERAPP_H

#include <QObject>
#include <QProcess>


/*!
 *   Since  KDE and GNOME calling "x-terminal-emulator" do not give the real process id, closing a TerminalFolderApp
 *   is disabled by default.
 *
 *   \ref TerminalFolderApp::closeTerminal()  works for:
 *   \li  Ubuntu Touch
 *   \li  Meego, tested on Nemo Mobile and Nokia N9
 */
#ifndef   ENABLE_CLOSING
# define  ENABLE_CLOSING 0
#endif

/*!
 * \brief The TerminalFolderApp class opens a suitable Terminal application with a working directory
 *
 *  It tries to find and open a terminal application in the sequence:
 *  \li  "meego-terminal"          for Nemo Mobile and Nokia N9
 *  \li  "ubuntu-terminal-app"     for Ubuntu Touch
 *  \li  "x-terminal-emulator"     for any Desktop such as KDE and Gnome
 *  \li  TERM environment variable if defined
 */
class TerminalFolderApp : public QObject
{    
    Q_OBJECT
public:
    explicit TerminalFolderApp(QObject *parent = 0);
    ~TerminalFolderApp(); 

public slots:
    bool openTerminal(const QString& currentDir);

private:
    void   findTerminalApp();
    void   findDesktopParameter();


#if ENABLE_CLOSING
signals:
    void   openCounterChanged(int openedTerminals);
public slots:
    bool   closeTerminal(int index);
private:    
    bool   killPid(Q_PID pid);   
#endif

#if REGRESSION_TEST_FOLDERLISTMODEL
public:
#else
private:
#endif
    QList<Q_PID>   m_openPids;
    QString        m_terminalApp;
    QStringList    m_params;        //!< used to pass workdir parameter
};
#endif // TERMINALFOLDERAPP_H
