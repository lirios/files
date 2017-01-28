/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author : Arto Jalkanen <ajalkane@gmail.com>
 */

#ifndef PAMAUTHENTICATION_H
#define PAMAUTHENTICATION_H

#include <QObject>
#include <QAbstractListModel>
#include <QStandardPaths>
#include <QSettings>

// Forward declarations
struct pam_handle;
struct pam_message;
struct pam_response;

/**
 * QML plugin to query if authentication is required and do authentication.
 *
 * At the moment the plugin is expected to be used as follows:
 *
 * - Ask if authentication is required (requireAuthentication() function). This will
 *   return true if Ubuntu Touch user has selected some unlocking security (PIN code or password).
 * - If so, the application should provide a dialog to ask for the PIN code/password and
 *   call validatePasswordToken(provided password) function.
 * - If validatePasswordToken() function returns true for given token, then user was authenticated,
 *   otherwise failed to authenticate.
 *
 * Caveats:
 *
 * - If user failed to authenticate, it doesn't necessarily mean password was invalid. Failure
 *   can also be because of account being locked or password being expired or because of some other
 *   reason in the underlying pam library. The exact reason is not currently available to QML, as practically
 *   for normal users the only reason is password mismatch. If the user has done advanced modifications
 *   the exact reason for failure can be seen in application's logs.
 * - Currently the application using this module will have to provide the dialogs asking for
 *   for PIN/password and an error dialog. This will most likely change in future, but there's no
 *   concrete plans yet. Discussion at https://code.launchpad.net/~ubuntu-filemanager-dev/ubuntu-filemanager-app/require-screenlock-password/+merge/230058
 *   can be seen for some background information.
 *
 * Simplified skeleton QML code example that asks for authentication if required at start-up of application (specific dialogs omitted):
 *
 * import com.ubuntu.PamAuthentication 0.1
 *
 * MainView {
 *   PamAuthentication {
 *     id: pamAuthentication
 *     serviceName: "applicationName"
 *   }
 *
 *   Component.onCompleted: {
 *     if (pamAuthentication.requireAuthentication()) {
 *        var passwordToken = queryPasswordInputDialog()
 *        if (!pamAuthentication.validatePasswordToken(passwordToken) {
 *           // Successs...
 *        } else  {
 *           showAuthenticationFailedDialog()
 *        }
 *     }
 *   }
 * }
 *
 */
class PamAuthentication : public QObject
{
    Q_OBJECT

public:
    explicit PamAuthentication(QObject *parent = 0);
    ~PamAuthentication();

    /**
     * \brief returns true if current user has password or PIN code set for unlocking lockscreen
     */
    Q_INVOKABLE bool requireAuthentication();

    /**
     * \brief returns true if current user can be authenticated with given password token (be it PIN code or password)
     */
    Q_INVOKABLE bool validatePasswordToken(const QString &token);

    /**
     * \brief Should be set to the application's name in lowercase. For example, for FileManager it would be "filemanager".
     */
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY serviceNameChanged)
    inline const QString &serviceName() const {
        return m_serviceName;
    }

    void setServiceName(const QString &serviceName);
signals:
    void serviceNameChanged();

public slots:
private:
    static int ConversationFunction(int num_msg,
                                    const pam_message** msg,
                                    pam_response** resp,
                                    void* appdata_ptr);

    bool initPam(pam_handle **pamHandle);
    int validateAccount(pam_handle *pamHandle);

    QString m_passwordToken;
    QString m_serviceName;
    QString m_userLogin;
};

#endif // PAMAUTHENTICATION_H



