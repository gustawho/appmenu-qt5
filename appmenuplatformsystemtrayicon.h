/*
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
 */

#ifndef APPMENUPLATFORMSYSTEMTRAYICON_H
#define APPMENUPLATFORMSYSTEMTRAYICON_H

#include <QtCore/qrect.h>
#include <QtCore/qstring.h>
#include <QtGui/qicon.h>
#include <qpa/qplatformsystemtrayicon.h>
#include <dbusmenuexporter.h>
#include "statusnotifieritem_adaptor.h"

class AppMenuPlatformSystemTrayIcon : public QPlatformSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(QString Category READ category)
    Q_PROPERTY(QString Id READ id)
    Q_PROPERTY(QString Title READ title)
    Q_PROPERTY(QString Status READ status)
    Q_PROPERTY(quint32 WindowId READ windowId)
    Q_PROPERTY(QString IconThemePath READ iconThemePath)
    Q_PROPERTY(QString IconName READ iconName)
    Q_PROPERTY(DBusImageList IconPixmap READ iconPixmap)
    Q_PROPERTY(QString OverlayIconName READ overlayIconName)
    Q_PROPERTY(DBusImageList OverlayIconPixmap READ overlayIconPixmap)
    Q_PROPERTY(QString AttentionIconName READ attentionIconName)
    Q_PROPERTY(DBusImageList AttentionIconPixmap READ attentionIconPixmap)
    Q_PROPERTY(QString AttentionMovieName READ attentionMovieName)
    Q_PROPERTY(DBusToolTip ToolTip READ toolTip)
    Q_PROPERTY(QDBusObjectPath Menu READ menu)

public:
    AppMenuPlatformSystemTrayIcon();
    virtual ~AppMenuPlatformSystemTrayIcon();

    // QPlatformSystemTrayIcon
    virtual void init();
    virtual void cleanup();
    virtual void updateIcon(const QIcon &icon);
    virtual void updateToolTip(const QString &tooltip);
    virtual void updateMenu(QPlatformMenu *menu);
    virtual QRect geometry() const;
    virtual void showMessage(const QString &title, const QString &msg,
                             const QIcon &icon, MessageIcon iconType, int msecs);
    virtual bool isSystemTrayAvailable() const;
    virtual bool supportsMessages() const;
    virtual QPlatformMenu *createMenu() const;

    // Properties
    QString category() const;
    QString id() const;
    QString title() const;
    QString status() const;
    quint32 windowId() const { return 0; }
    QString iconThemePath() const;
    QString iconName() const;
    DBusImageList iconPixmap() const { return DBusImageList(); }
    QString overlayIconName() const { return QString(); }
    DBusImageList overlayIconPixmap() const { return DBusImageList(); }
    QString attentionIconName() const { return QString(); }
    DBusImageList attentionIconPixmap() const { return DBusImageList(); }
    QString attentionMovieName() const { return QString(); }
    DBusToolTip toolTip() const;
    QDBusObjectPath menu() const;

    // StatusNotifierItem
    void ContextMenu(int x, int y);
    void Activate(int x, int y);
    void SecondaryActivate(int x, int y);
    void Scroll(int delta, const QString& orientation);

Q_SIGNALS:
    void NewStatus(const QString &status);
    void NewIcon();
    void NewToolTip();

private:
    QString m_serviceName;
    QString m_objectPath;
    QString m_status;
    QIcon m_icon;
    QString m_tooltip;

    QDBusConnection m_connection;
    StatusNotifierItemAdaptor *m_sniAdaptor;
    DBusMenuExporter *m_dbusMenuExporter;
};

#endif /* APPMENUPLATFORMSYSTEMTRAYICON_H */
