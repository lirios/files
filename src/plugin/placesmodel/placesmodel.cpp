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
 *          Arto Jalkanen <ajalkane@gmail.com>
 */

#include "placesmodel.h"
#include <QDebug>
#include <QDir>
#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>

PlacesModel::PlacesModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_userMountLocation = "/media/" + qgetenv("USER");
    // For example /run/user/1000
    m_runtimeLocations = QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);

    QStringList defaultLocations;
    // Set the storage location to a path that works well
    // with app isolation
    QString settingsLocation =
            QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + "/" + QCoreApplication::applicationName() + "/" + "places.conf";
    m_settings = new QSettings(settingsLocation, QSettings::IniFormat, this);

    // Prepopulate the model with the user locations
    // for the first time it's used
    defaultLocations.append(locationHome());
    defaultLocations.append(locationDocuments());
    defaultLocations.append(locationDownloads());
    defaultLocations.append(locationMusic());
    defaultLocations.append(locationPictures());
    defaultLocations.append(locationVideos());
    // Add root also
    defaultLocations.append("/");

    if (!m_settings->contains("storedLocations")) {
        m_locations.append(defaultLocations);
    } else {
        m_locations = m_settings->value("storedLocations").toStringList();
    }

    foreach (const QString &location, m_locations) {
        qDebug() << "Location: " << location;
    }

    initNewUserMountsWatcher();
    rescanMtab();
}

PlacesModel::~PlacesModel() {

}

void
PlacesModel::initNewUserMountsWatcher() {
    m_newUserMountsWatcher = new QFileSystemWatcher(this);

    qDebug() << Q_FUNC_INFO << "Start watching mtab file for new mounts" << m_mtabParser.path();

    m_newUserMountsWatcher->addPath(m_mtabParser.path());

    connect(m_newUserMountsWatcher, &QFileSystemWatcher::fileChanged, this, &PlacesModel::mtabChanged);
}

void
PlacesModel::mtabChanged(const QString &path) {
    qDebug() << Q_FUNC_INFO << "file changed in " << path;
    rescanMtab();
    // Since old mtab file is replaced with new contents, must readd filesystem watcher
    m_newUserMountsWatcher->removePath(path);
    m_newUserMountsWatcher->addPath(path);
}

void
PlacesModel::rescanMtab() {
    const QString& path = m_mtabParser.path();
    qDebug() << Q_FUNC_INFO << "rescanning mtab" << path;

    QList<QMtabEntry> entries = m_mtabParser.parseEntries();

    QSet<QString> userMounts;

    foreach (QMtabEntry e, entries) {
        qDebug() << Q_FUNC_INFO << "Considering" << "fsName:" <<  e.fsName << "dir:" << e.dir << "type:" << e.type;
        if (isMtabEntryUserMount(e)) {
            qDebug() << Q_FUNC_INFO << "Adding as userMount directory dir" << e.dir;
            userMounts << e.dir;
        }
    }

    QSet<QString> addedMounts = QSet<QString>(userMounts).subtract(m_userMounts);
    QSet<QString> removedMounts = QSet<QString>(m_userMounts).subtract(userMounts);

    m_userMounts = userMounts;

    foreach (QString addedMount, addedMounts) {
        qDebug() << Q_FUNC_INFO << "user mount added: " << addedMount;
        addLocationWithoutStoring(addedMount);
        emit userMountAdded(addedMount);
    }

    foreach (QString removedMount, removedMounts) {
        qDebug() << Q_FUNC_INFO << "user mount removed: " << removedMount;
        int index = m_locations.indexOf(removedMount);
        if (index > -1) {
            removeItemWithoutStoring(index);
        }
        emit userMountRemoved(removedMount);
    }
}

bool PlacesModel::isMtabEntryUserMount(const QMtabEntry &e) const {
    if (e.fsName == "none") {
        qDebug() << Q_FUNC_INFO << "Ignoring mounts with filesystem name 'none'";
        return false;
    }
    if (isSubDirectory(m_userMountLocation, e.dir)) {
        qDebug() << Q_FUNC_INFO << "Is user mount location";
        return true;
    }
    foreach (const QString &runtimeLocation, m_runtimeLocations) {
        if (isSubDirectory(runtimeLocation, e.dir)) {
            qDebug() << Q_FUNC_INFO << "Is user mount location";
            return true;
        }
    }

    return false;
}

bool PlacesModel::isSubDirectory(const QString &dir, const QString &path) const {
    QFileInfo dirFi = QFileInfo(dir);
    QFileInfo pathFi = QFileInfo(path);

    QString absDir = dirFi.absolutePath();
    QString absPath = pathFi.absolutePath();

    return absPath.startsWith(QString(absDir + "/"));
}

QString PlacesModel::standardLocation(QStandardPaths::StandardLocation location) const
{
    QStringList locations = QStandardPaths::standardLocations(location);
    QString standardLocation = "";

    foreach (const QString &location, locations) {
        // We always return the first location or an empty string
        // The frontend should check out that it exists
        if (QDir(location).exists()) {
            standardLocation = location;
            break;
        }
    }

    return standardLocation;
}

QString PlacesModel::locationHome() const
{
    return standardLocation(QStandardPaths::HomeLocation);
}

QString PlacesModel::locationDocuments() const
{
    return standardLocation(QStandardPaths::DocumentsLocation);
}

QString PlacesModel::locationDownloads() const
{
    return standardLocation(QStandardPaths::DownloadLocation);
}

QString PlacesModel::locationMusic() const
{
    return standardLocation(QStandardPaths::MusicLocation);
}

QString PlacesModel::locationPictures() const
{
    return standardLocation(QStandardPaths::PicturesLocation);
}

QString PlacesModel::locationVideos() const
{
    return standardLocation(QStandardPaths::MoviesLocation);
}

int PlacesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_locations.count();
}

QVariant PlacesModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)

    return m_locations.at(index.row());
}

QHash<int, QByteArray> PlacesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::UserRole, "path");

    return roles;
}

void PlacesModel::removeItem(int indexToRemove)
{
    removeItemWithoutStoring(indexToRemove);

    // Remove the location permanently
    m_settings->setValue("storedLocations", m_locations);
}

void PlacesModel::removeItemWithoutStoring(int indexToRemove)
{

    // Tell Qt that we're going to be changing the model
    // There's no tree-parent, first new item will be at
    // indexToRemove, and the last one too
    beginRemoveRows(QModelIndex(), indexToRemove, indexToRemove);

    // Remove the actual location
    m_locations.removeAt(indexToRemove);

    // Tell Qt we're done with modifying the model so that
    // it can update the UI and everything else to reflect
    // the new state
    endRemoveRows();
}

void PlacesModel::addLocation(const QString &location)
{
    if (addLocationWithoutStoring(location)) {
        // Store the location permanently
        m_settings->setValue("storedLocations", m_locations);
    }
}

bool PlacesModel::addLocationWithoutStoring(const QString &location)
{
    // Do not allow for duplicates
    if (!m_locations.contains(location)) {
        // Tell Qt that we're going to be changing the model
        // There's no tree-parent, first new item will be at
        // m_locations.count(), and the last one too
        beginInsertRows(QModelIndex(), m_locations.count(), m_locations.count());

        // Append the actual location
        m_locations.append(location);


        // Tell Qt we're done with modifying the model so that
        // it can update the UI and everything else to reflect
        // the new state
        endInsertRows();
        return true;
    }
    return false;
}
