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
 * Filename: terminalfolderapp.cpp
 * Created : 29 Jul 2013
 **/


#include "terminalfolderapp.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN)
   // defined WIN Process
#else // Q_OS_UNIX
# include <sys/types.h>
# include <signal.h>
# include <unistd.h>
#endif

#define DEBUG_MESSAGES 1

TerminalFolderApp::TerminalFolderApp(QObject *parent) :
    QObject(parent)
{
    findTerminalApp();
}

TerminalFolderApp::~TerminalFolderApp()
{
}


//=======================================================================================================================
/*!
 * \brief TerminalFolderApp::openTerminal() tries to open a terminal
 *
 *  It tries to open the terminal application in the sequence:
 *  \li  "meego-terminal"          for Nemo Mobile and Nokia N9
 *  \li  "ubuntu-terminal-app"     for Ubuntu Touch
 *  \li  "x-terminal-emulator"     for any Desktop such as KDE and Gnome
 *  \li  TERM environment variable if defined
 *
 * \param currentDir
 * \return true if the terminal could be opened
 */
bool TerminalFolderApp::openTerminal(const QString& currentDir)
{
    QFileInfo f(currentDir);
    bool launched = false;
    bool ok = false;
    Q_PID pid = 0;
    if (!m_terminalApp.isEmpty() &&
         f.isDir() &&
         f.isReadable() &&
         f.isExecutable())
    {
       launched = true;
       switch(m_params.count())
      {
         case 2:  m_params.append(currentDir);  break;
         case 3:  m_params[2] = currentDir;
        default: break;
      }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "starting" << m_terminalApp
             << m_params << "at" << currentDir;
#endif
      ok = QProcess::startDetached(m_terminalApp,
                                 m_params, currentDir, &pid);
      if (ok)
      {
#if ENABLE_CLOSING        
          emit openCounterChanged(m_openPids.count() + 1);
#endif
          m_openPids.append(pid);
      }
    }//end found a terminal application and currentDir is good
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "launched" << launched << "ok" << ok << "returning pid" << pid;
#endif   

    return ok;
}



//=======================================================================================================================
/*!
 * \brief TerminalFolderApp::findTerminalApp() finds a suitable terminal application, called only in the creator
 */
void TerminalFolderApp::findTerminalApp()
{
#if defined(Q_OS_MSDOS) || defined(Q_OS_WIN)
    m_terminalApp = "cmd.exe";
#else // Q_OS_UNIX
    char terminalEnvVar[128];
    terminalEnvVar[0] = 0;
    QByteArray term = qgetenv("TERM");
    if (term.size() > 0)
    {
        ::qstrcpy(terminalEnvVar, term.constData());
    }

    // Ubuntu Touch
    // appid://com.ubuntu.terminal/terminal/current-user-version"


    const char *desktop_terminal_emulator = "x-terminal-emulator";
    const char * apps [] =
    {
        "meego-terminal"          // Nemo Mobile and Nokia N9
       ,"ubuntu-terminal-app"     // Ubuntu Touch
       ,desktop_terminal_emulator // Desktop (any) KDE/GNOME
       ,terminalEnvVar            // TERM environment variable
    };

    QByteArray usr_bin("/usr/bin");
    QByteArray  path = qgetenv("PATH");
    if (path.size() == 0 ) {
        path = usr_bin;
    }
    QList<QByteArray>  dirs(path.split(':'));
    if (!dirs.contains(usr_bin)) {
        dirs.append(usr_bin);
    }
    bool found = false;
    for (int counter = 0; !found && counter < dirs.count(); ++counter)
    {
        for(size_t counter2=0 ; !found && counter2 < sizeof(apps)/sizeof(apps[0]); ++counter2)
        {
            if (apps[counter2][0])
            {
                QString terminalPath = dirs[counter];
                terminalPath += QDir::separator();
                terminalPath += apps[counter2];
                QFileInfo terminal(terminalPath);
                if (terminal.exists() && terminal.isExecutable())
                {
                    m_terminalApp = terminalPath;
                    found = true;
                    if (terminalPath.endsWith(QLatin1String(desktop_terminal_emulator)))
                    {
                        findDesktopParameter();
                    }
                }
            }
        }
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << m_terminalApp;
#endif
#endif
}


//=======================================================================================================================
/*!
 * \brief TerminalFolderApp::findDesktopParameter() sets the working directory parameter
 *
 *  Options:
 * \li if konsole --nofork --workdir <dir>
 * \li if gnome-terminal "--disable-factory --working-dir <dir>
 */
void TerminalFolderApp::findDesktopParameter()
{
    QFileInfo app(m_terminalApp);
    int counter = 10;
    while (counter-- && app.isSymLink())
    {
        app.setFile(app.symLinkTarget());
#if DEBUG_MESSAGES
        qDebug() << Q_FUNC_INFO << "testing" << app.fileName();
#endif
    }
    if (app.absoluteFilePath().endsWith(QLatin1String("konsole")))
    {        
        m_params.append(QLatin1String("--nofork"));
        m_params.append(QLatin1String("--workdir"));
        if (app.isAbsolute())
        {
            m_terminalApp = app.absoluteFilePath();
        }
    }
    else
    if (app.absoluteFilePath().endsWith(QLatin1String("gnome-terminal")))
    {
        if (app.isAbsolute())
        {
            QFileInfo gnomeTerm(app.absolutePath(),QLatin1String("gnome-terminal"));
            if (gnomeTerm.isExecutable() && gnomeTerm.isFile())
            {
                m_terminalApp = gnomeTerm.absoluteFilePath();
            }
        }
        m_params.append(QLatin1String("--disable-factory"));
        m_params.append(QLatin1String("--working-dir"));
    }
}


#if ENABLE_CLOSING

//=======================================================================================================================
/*!
 * \brief TerminalFolderApp::closeTerminal() tries to close the terminal app
 * \param index index of opened terminals, if more than one.
 * \return true could close the terminal
 */
bool TerminalFolderApp::closeTerminal(int index)
{
    Q_PID pid   = 0;
    bool closed = false;
    int counter = m_openPids.count();
    int before = counter;
    while (counter--)
    {
        // check if any terminal that was closed by the user
        pid = m_openPids.at(counter);
        int test = ::kill(pid, 0);
#if DEBUG_MESSAGES
        qDebug() << Q_FUNC_INFO << "pid" << pid
                 << "ret from kill" << test;
#endif
        if ( test != 0 )
        {
            m_openPids.takeAt(counter);
        }
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "active terminals" << m_openPids.count();
#endif
    if (index >= 0 && m_openPids.count() > 0)
    {
        if (index >= m_openPids.count())
        {
            index = m_openPids.count() -1;
        }
        pid    = m_openPids.takeAt(index);
        if (!(closed = killPid(pid)))
        {
            m_openPids.append(pid);
        }
    }
    if (before != m_openPids.count())
    {
       emit openCounterChanged(m_openPids.count());
    }
    return closed;
}



//=======================================================================================================================
/*!
     kill a process.

     Tries to kill using SIGTERM which is the default signal to finish a process.
     If the process  does not finish then sends a SIGKILL which is equivalment to "kill -9" command

     \return TRUE the process does not exist anymore (it was successfully killed).
             FALSE could not kill the process
 */
//=======================================================================================================================
bool   TerminalFolderApp::killPid(Q_PID pid)
{
    bool processIsNotRunning = true;
    if (::kill(pid, SIGTERM) == 0)
    {
        int counter = 15;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        while(counter-- && ::kill(pid, 0) == 0)
        {
            ::usleep(50);
            switch(counter)
            {              
               case 13:       ::kill(pid, SIGKILL); break;
               default:       break;
            }
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        if (::kill(pid, 0) == 0)
        {
            processIsNotRunning = false;
        }
    }
#if DEBUG_MESSAGES
    qDebug() << Q_FUNC_INFO << "pid" << pid << "ret" << processIsNotRunning;
#endif
    return processIsNotRunning;
}


#endif // ENABLE_CLOSING
