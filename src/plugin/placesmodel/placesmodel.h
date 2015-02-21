/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Author : David Planella <david.planella@ubuntu.com>
 *          Arto Jalkanen <arto.jalkanen@gmail.com>
 */

#ifndef PLACESMODEL_H
#define PLACESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QStandardPaths>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QSet>

#include "qmtabparser.h"

class PlacesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString locationHome READ locationHome CONSTANT)
    Q_PROPERTY(QString locationDocuments READ locationDocuments CONSTANT)
    Q_PROPERTY(QString locationDownloads READ locationDownloads CONSTANT)
    Q_PROPERTY(QString locationMusic READ locationMusic CONSTANT)
    Q_PROPERTY(QString locationPictures READ locationPictures CONSTANT)
    Q_PROPERTY(QString locationVideos READ locationVideos CONSTANT)

public:
    explicit PlacesModel(QObject *parent = 0);
    ~PlacesModel();
    QString locationHome() const;
    QString locationDocuments() const;
    QString locationDownloads() const;
    QString locationMusic() const;
    QString locationPictures() const;
    QString locationVideos() const;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void userMountAdded(const QString &path);
    void userMountRemoved(const QString &paht);

public slots:
    void addLocation(const QString &location);
    void removeItem(int indexToRemove);
    inline bool isUserMountDirectory(const QString location) {
        return m_userMounts.contains(location);
    }

private slots:
    void mtabChanged(const QString &path);
    void rescanMtab();

private:
    void initNewUserMountsWatcher();
    // Returns true if location was not known before, and false if it was known
    bool addLocationWithoutStoring(const QString &location);
    // Returns true if location was not known before, and false if it was known
    void removeItemWithoutStoring(int itemToRemove);

    QMtabParser m_mtabParser;
    QStringList m_runtimeLocations;
    QString m_userMountLocation;
    bool isMtabEntryUserMount(const QMtabEntry &entry) const;
    bool isSubDirectory(const QString &dir, const QString &path) const;
    QString standardLocation(QStandardPaths::StandardLocation location) const;
    QStringList m_locations;
    QSettings *m_settings;
    QFileSystemWatcher *m_newUserMountsWatcher;
    QSet<QString> m_userMounts;
};

#endif // PLACESMODEL_H



