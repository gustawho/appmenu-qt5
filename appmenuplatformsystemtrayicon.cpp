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

#include <QtCore/qcoreapplication.h>
#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusconnectioninterface.h>
#include <QtDBus/qdbusinterface.h>
#include <QX11Info>
#include "appmenuplatformmenu.h"
#include "appmenuplatformsystemtrayicon.h"
#include "iconcache.h"

static const char *SNW_INTERFACE = "org.kde.StatusNotifierWatcher";
static const char *SNW_SERVICE   = "org.kde.StatusNotifierWatcher";
static const char *SNW_PATH      = "/StatusNotifierWatcher";

static const char *NOTIFICATION_INTERFACE = "org.freedesktop.Notifications";
static const char *NOTIFICATION_SERVICE   = "org.freedesktop.Notifications";
static const char *NOTIFICATION_PATH      = "/org/freedesktop/Notifications";

static const QString KDEItemFormat = QStringLiteral("org.kde.StatusNotifierItem-%1-%2");
static int instanceCount = 0;

static IconCache iconCache;

AppMenuPlatformSystemTrayIcon::AppMenuPlatformSystemTrayIcon():
    m_serviceName(KDEItemFormat.arg(QCoreApplication::applicationPid()).arg(++instanceCount)),
    m_objectPath("/StatusNotifierItem"),
    m_connection(QDBusConnection::connectToBus(QDBusConnection::SessionBus, m_serviceName)),
    m_sniAdaptor(new StatusNotifierItemAdaptor(this)),
    m_dbusMenuExporter(Q_NULLPTR)
{
    registerMetaTypes();
    m_connection.registerService(m_serviceName);
    m_connection.registerObject(m_objectPath, this, QDBusConnection::ExportAdaptors);
    QDBusInterface snw(SNW_SERVICE, SNW_PATH, SNW_INTERFACE);
    snw.asyncCall("RegisterStatusNotifierItem", m_serviceName);
}

AppMenuPlatformSystemTrayIcon::~AppMenuPlatformSystemTrayIcon()
{
    m_connection.unregisterObject(m_objectPath, QDBusConnection::UnregisterTree);
    m_connection.unregisterService(m_serviceName);
    delete m_sniAdaptor;
}

// QPlatformSystemTrayIcon implementation

void AppMenuPlatformSystemTrayIcon::init()
{
    m_status = QStringLiteral("Active");
    emit NewStatus(m_status);
}

void AppMenuPlatformSystemTrayIcon::cleanup()
{
    m_status = QStringLiteral("Passive");
    emit NewStatus(m_status);
}

void AppMenuPlatformSystemTrayIcon::updateIcon(const QIcon &icon)
{
    m_icon = icon;
    emit NewIcon();
    emit NewToolTip(); // ToolTip contains the icon
}

void AppMenuPlatformSystemTrayIcon::updateToolTip(const QString &tooltip)
{
    m_tooltip = tooltip;
    emit NewToolTip();
}

void AppMenuPlatformSystemTrayIcon::updateMenu(QPlatformMenu *menu)
{
    QMenu *qMenu = qobject_cast<AppMenuPlatformMenu *>(menu)->m_menu;
    QString menuObjectPath = m_objectPath + QStringLiteral("/menu");
    m_dbusMenuExporter = new DBusMenuExporter(menuObjectPath, qMenu, m_connection);
}

QRect AppMenuPlatformSystemTrayIcon::geometry() const
{
    return QRect();
}

void AppMenuPlatformSystemTrayIcon::showMessage(const QString &title, const QString &msg,
                                                const QIcon &icon, MessageIcon iconType, int msecs)
{
    QString iconString = icon.name();
    if (iconString.isEmpty()) {
        switch (iconType) {
        case NoIcon:
            break;
        case Information:
            iconString = "dialog-information";
            break;
        case Warning:
            iconString = "dialog-warning";
            break;
        case Critical:
            iconString = "dialog-error";
            break;
        }
    }

    QDBusInterface iface(NOTIFICATION_SERVICE, NOTIFICATION_PATH, NOTIFICATION_INTERFACE);
    iface.asyncCall("Notify",
                    id(),
                    quint32(0),    // replaces_id
                    iconString,
                    title,
                    msg,
                    QStringList(), // actions
                    QVariantMap(), // hints
                    msecs);        // timeout
}

bool AppMenuPlatformSystemTrayIcon::isSystemTrayAvailable() const
{
    const QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    if (!interface->isServiceRegistered(SNW_SERVICE).value()) {
        return false;
    }
    QDBusInterface snw(SNW_SERVICE, SNW_PATH, SNW_INTERFACE);
    QVariant value = snw.property("IsStatusNotifierHostRegistered");
    if (!value.canConvert<bool>()) {
        return false;
    }
    return value.toBool();
}

bool AppMenuPlatformSystemTrayIcon::supportsMessages() const
{
    const QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    return interface->isServiceRegistered(NOTIFICATION_SERVICE).value();
}

QPlatformMenu *AppMenuPlatformSystemTrayIcon::createMenu() const
{
    return new AppMenuPlatformMenu();
}

// Properties

QString AppMenuPlatformSystemTrayIcon::category() const
{
    return QStringLiteral("ApplicationStatus");
}

QString AppMenuPlatformSystemTrayIcon::id() const
{
    return QCoreApplication::applicationFilePath().section('/', -1);
}

QString AppMenuPlatformSystemTrayIcon::title() const
{
    QString title = QCoreApplication::applicationName();
    return title.isEmpty() ? id() : title;
}

QString AppMenuPlatformSystemTrayIcon::status() const
{
    return m_status;
}

QString AppMenuPlatformSystemTrayIcon::iconThemePath() const
{
    return iconCache.themePath(m_icon);
}

QString AppMenuPlatformSystemTrayIcon::iconName() const
{
    if (m_icon.isNull()) {
        return QString();
    }
    QString name = m_icon.name();
    if (!name.isEmpty()) {
        return name;
    }

    return iconCache.nameForIcon(m_icon);
}

DBusToolTip AppMenuPlatformSystemTrayIcon::toolTip() const
{
    DBusToolTip tip;
    tip.iconName = iconName();
    tip.title = m_tooltip;
    return tip;
}

QDBusObjectPath AppMenuPlatformSystemTrayIcon::menu() const
{
    return m_dbusMenuExporter
        ? QDBusObjectPath(m_objectPath + QStringLiteral("/menu"))
        : QDBusObjectPath("/invalid");
}

// StatusNotifierItem implementation

void AppMenuPlatformSystemTrayIcon::ContextMenu(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void AppMenuPlatformSystemTrayIcon::Activate(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    // Workarounds LP: #627195
    if (QX11Info::isPlatformX11() &&
        QString::fromUtf8(getenv("XDG_CURRENT_DESKTOP")).split(':').contains("Unity")) {
        QX11Info::setAppUserTime(0);
    }
    emit activated(Trigger);
}

void AppMenuPlatformSystemTrayIcon::SecondaryActivate(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    emit activated(MiddleClick);
}

void AppMenuPlatformSystemTrayIcon::Scroll(int delta, const QString& orientation)
{
    Q_UNUSED(delta);
    Q_UNUSED(orientation);
}
