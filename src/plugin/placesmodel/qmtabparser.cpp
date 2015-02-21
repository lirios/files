/*
 * Copyright (C) 2015 Canonical Ltd
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
 *          Carlos J Mazieri <carlos.mazieri@gmail.com>
 */

#include <qmtabparser.h>

#include <mntent.h>

#include <QFileInfo>
#include <QStringList>

class MtabFileGuard {
    FILE *mtabFile;

public:
    MtabFileGuard(FILE *f) {
        mtabFile = f;
    }
    ~MtabFileGuard() {
        endmntent(mtabFile);
    }
};

QMtabParser::QMtabParser(const QString& path, QObject *parent)
    : QObject(parent) {
    m_path = path.isEmpty() ? _PATH_MOUNTED : path;
}

QMtabParser::~QMtabParser() {}

QList<QMtabEntry>
QMtabParser::parseEntries() {
    QList<QMtabEntry> entries;

    FILE *f = setmntent(m_path.toLocal8Bit().data(), "r");
    if (f == 0) {
        return entries;
    }

    MtabFileGuard guard(f);

    struct mntent entStorage;
    char buffer[1024];
    while (mntent *ent = getmntent_r(f, &entStorage, buffer, 1024)) {
        QMtabEntry entry;
        entry.fsName = ent->mnt_fsname;
        entry.dir = ent->mnt_dir;
        entry.type = ent->mnt_type;
        entry.opts = ent->mnt_opts;
        entry.freq = ent->mnt_freq;
        entry.passno = ent->mnt_passno;
        entries << entry;
    }

    return entries;
}
