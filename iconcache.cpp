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

#include <sys/types.h>
#include <utime.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include "iconcache.h"

const int IconCache::MaxIconCount = 20;

IconCache::IconCache(QObject *parent):
    QObject(parent),
    m_temporaryDir(Q_NULLPTR)
{
}

IconCache::~IconCache()
{
    if (m_temporaryDir) {
        delete m_temporaryDir;
    }
}

QString IconCache::themePath(const QIcon &icon)
{
    if (!m_temporaryDir) {
        QString dir;
        QString runtimeDir = QString::fromUtf8(getenv("XDG_RUNTIME_DIR"));

        if (!runtimeDir.isEmpty()) {
            dir = runtimeDir;
            QDir d(dir);
            if (!d.exists()) {
                d.mkpath(".");
            }
        } else if (!getenv("SNAP")) {
            dir = QDir::tempPath();
        } else {
            // Try to get the .cache from $XDG_CACHE_HOME, if it's not set,
            // it has to be in ~/.cache as per XDG standard
            dir = QString::fromUtf8(getenv("XDG_CACHE_HOME"));
            if (dir.isEmpty()) {
                dir = QDir::cleanPath(QDir::homePath() + QStringLiteral("/.cache"));
            }

            QDir d(dir);
            if (!d.exists()) {
                d.mkpath(".");
            }
        }

        QString path = dir + QStringLiteral("/qt-tray-iconcache-XXXXXX");
        m_temporaryDir = new QTemporaryDir(path);
    }

    if (!icon.isNull() && !icon.name().isEmpty() && QIcon::hasThemeIcon(icon.name())) {
        QString dataHome = QString::fromUtf8(getenv("XDG_DATA_HOME"));

        if (dataHome.isEmpty()) {
            dataHome = QDir::homePath() + "/.local/share";
        }

        return QDir::cleanPath(dataHome + "/icons");
    }

    return m_temporaryDir->path();
}

QString IconCache::nameForIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        return QString();
    }

    qint64 key = icon.cacheKey();
    QList<qint64>::iterator it = qFind(m_cacheKeys.begin(), m_cacheKeys.end(), key);
    if (it == m_cacheKeys.end()) {
        cacheIcon(key, icon);
        trimCache();
    } else {
        // Place key at the end of list as it is the most recently accessed
        m_cacheKeys.erase(it);
        m_cacheKeys.append(key);
    }

    return QString::number(key);
}

void IconCache::trimCache()
{
    QDir dir(themePath() + "/hicolor");
    dir.setFilter(QDir::Dirs);

    while (m_cacheKeys.count() > MaxIconCount) {
        qint64 cacheKey = m_cacheKeys.takeFirst();

        Q_FOREACH(const QString &sizeDir, dir.entryList()) {
            QString iconSubPath = QString("%1/apps/%2.png").arg(sizeDir).arg(cacheKey);
            if (dir.exists(iconSubPath)) {
                dir.remove(iconSubPath);
            }
        }
    }
}

void touch(const QString &_name, const QDateTime &time)
{
    QByteArray name = QFile::encodeName(_name);
    struct utimbuf buf;
    buf.actime = time.toTime_t();
    buf.modtime = buf.actime;
    int ret = utime(name.data(), &buf);
    if (ret) {
        qCritical("Failed to touch %s: %s", name.data(), strerror(errno));
    }
}

void IconCache::cacheIcon(qint64 key, const QIcon &icon)
{
    QList<QSize> sizes; //= icon.availableSizes();
    if (sizes.isEmpty()) {
        // sizes can be empty if icon is an SVG. In this case generate images for a few sizes
        sizes << QSize(16, 16) << QSize(22, 22) << QSize(32, 32) << QSize(48, 48);
    }

    QDir dir(themePath());
    Q_FOREACH(const QSize &size, sizes) {
        QPixmap pix = icon.pixmap(size);
        QString dirName = QString("hicolor/%1x%1/apps").arg(size.width());
        if (!dir.exists(dirName)) {
            if (!dir.mkpath(dirName)) {
                qWarning("Could not create '%s' dir in '%s'",
                    qPrintable(themePath()), qPrintable(dirName));
                continue;
            }
        }
        QString pixPath = QString("%1/%2/%3.png")
            .arg(themePath()).arg(dirName).arg(key);
        if (!pix.save(pixPath, "png")) {
            qWarning("Could not save icon as '%s'", qPrintable(pixPath));
        }
    }

    m_cacheKeys << key;

    // Touch the theme path: GTK icon loading system checks the mtime of the
    // dir to decide whether it should look for new icons in the theme dir.
    //
    // Note: We do not use QDateTime::currentDateTime() as the new time because
    // if the icon is updated in less than one second, GTK won't notice it.
    // See https://bugs.launchpad.net/sni-qt/+bug/812884
    QFileInfo info(themePath());
    QDateTime mtime = info.lastModified();
    touch(themePath(), mtime.addSecs(1));
}
