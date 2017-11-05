/*
 * Copyright 2011 Canonical Ltd.
 * Copyright 2014 Dmitry Shachnev <mitya57@ubuntu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Based on sni-qt code by Aurelien Gateau <aurelien.gateau@canonical.com>.
 */

#ifndef ICONCACHE_H
#define ICONCACHE_H

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qtemporarydir.h>
#include <QtGui/qicon.h>

/**
 * This class will save pixmaps from icons in a temporary dir on the disk,
 * making it possible to pass filenames for icons without names.
 */
class IconCache : public QObject
{
    Q_OBJECT
public:
    IconCache(QObject* parent=0);
    ~IconCache();

    static const int MaxIconCount;

    QString themePath(const QIcon &icon = QIcon());
    QString nameForIcon(const QIcon &icon);

private:
    QTemporaryDir *m_temporaryDir;
    mutable QList<qint64> m_cacheKeys;

    void cacheIcon(qint64 key, const QIcon &);
    void trimCache();
};

#endif /* ICONCACHE_H */
