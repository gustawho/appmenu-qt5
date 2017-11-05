/*
 * Copyright 2013-2014 Canonical Ltd.
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
 * Author: Łukasz 'sil2100' Zemczak <lukasz.zemczak@canonical.com>
 */

#include "appmenuplatformmenubar.h"
#include "appmenuplatformmenuitem.h"
#include "appmenuplatformsystemtrayicon.h"
#include "registrar_interface.h"

// Ugly, but sadly we need to use private headers for desktop-theme related classes
#include <private/qgenericunixthemes_p.h>

#include <dbusmenuexporter.h>

#include <QWindow>
#include <QWidget>
#include <QString>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QDebug>
#include <QList>
#include <QVariant>
#include <QX11Info>

#undef signals // Needed to make sure we can include gtk.h
#include <gtk/gtk.h>
#include <X11/Xlib.h>


#define LOG qDebug() << "appmenu-qt:" << __FUNCTION__ << __LINE__
#define LOG_VAR(x) qDebug() << "appmenu-qt:" << __FUNCTION__ << __LINE__ << #x ":" << x
#define WARN qWarning() << "appmenu-qt:" << __FUNCTION__ << __LINE__

QT_BEGIN_NAMESPACE

static const char* REGISTRAR_SERVICE = "com.canonical.AppMenu.Registrar";
static const char* REGISTRAR_PATH    = "/com/canonical/AppMenu/Registrar";


bool useGlobalMenu() {
    bool haveAppMenu = QDBusConnection::sessionBus().interface()->isServiceRegistered(REGISTRAR_SERVICE).value();
    QByteArray menuProxy = qgetenv("UBUNTU_MENUPROXY");
    bool menuProxyIsGood = menuProxy.isNull() ||
                           (!menuProxy.isEmpty() && menuProxy.at(0) != '0');
    return haveAppMenu && menuProxyIsGood;
}

/*
 * The menubar adapter communicates over DBus with the menubar renderer.
 * It is responsible for registering windows to it and exposing their menubars
 * if they have one.
 */
class MenuBarAdapter
{
public:
    MenuBarAdapter(QMenuBar*, const QString&);
    ~MenuBarAdapter();

    bool registerWindow();
    void resetRegisteredWinId();

private:
    uint m_registeredWinId;
    DBusMenuExporter* m_exporter;
    QMenuBar* m_menuBar;
    QString m_objectPath;
};


QList<QMenuBar *> globalMenuBars;


///////////////////////////////////////////////////////////
AppMenuPlatformMenuBar::AppMenuPlatformMenuBar()
    : QPlatformMenuBar(),
      m_menubar(0),
      m_window(0),
      m_adapter(0)
{
}

AppMenuPlatformMenuBar::~AppMenuPlatformMenuBar()
{
}

void 
AppMenuPlatformMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
{
    Q_UNUSED(menu);
    Q_UNUSED(before);
    return;
}

void 
AppMenuPlatformMenuBar::removeMenu(QPlatformMenu *menu)
{
    Q_UNUSED(menu);
    return;
}

void 
AppMenuPlatformMenuBar::syncMenu(QPlatformMenu *menuItem)
{
    Q_UNUSED(menuItem);
    handleReparent(m_window);
    return;
}

void 
AppMenuPlatformMenuBar::handleReparent(QWindow *newParentWindow)
{
    if (!newParentWindow)
        return;

    static int menuBarId = 1;
    m_objectPath = QString(QLatin1String("/MenuBar/%1")).arg(menuBarId++);

    m_window = newParentWindow;
    QWidget *window = QWidget::find(m_window->winId());

    m_menubar = window->findChild<QMenuBar *>();
    if (!m_menubar) {
        // Something went wrong, this shouldn't have happened
        WARN << "The given QWindow has no QMenuBar assigned";
        return;
    }

    if (globalMenuBars.indexOf(m_menubar) != -1) {
        WARN << "The given QMenuBar is already registered by appmenu-qt5, skipping";
        m_menubar = 0;
        return;
    }

    delete m_adapter;
    m_adapter = new MenuBarAdapter(m_menubar, m_objectPath);
    if (m_adapter->registerWindow()) {
        globalMenuBars.push_back(m_menubar);
    }
}

QPlatformMenu *
AppMenuPlatformMenuBar::menuForTag(quintptr tag) const
{
    Q_UNUSED(tag);
    return NULL;
}


///////////////////////////////////////////////////////////
MenuBarAdapter::MenuBarAdapter(QMenuBar* _menuBar, const QString& _objectPath)
    : m_registeredWinId(0),
      m_exporter(0), 
      m_menuBar(_menuBar),
      m_objectPath(_objectPath)
{
}

MenuBarAdapter::~MenuBarAdapter()
{
    delete m_exporter;
    m_exporter = 0;
    globalMenuBars.removeAll(m_menuBar);
}

bool
MenuBarAdapter::registerWindow()
{
    static com::canonical::AppMenu::Registrar *registrar = 0;

    if (globalMenuBars.indexOf(m_menuBar) >= 0) {
        WARN << "Already present, error!";
        return false;
    }

    if (!m_menuBar->window()) {
        WARN << "No parent for this menubar";
        return false;
    }

    uint winId = m_menuBar->window()->winId();
    if (winId == m_registeredWinId)
        return true;

    if (!registrar)
        registrar = new com::canonical::AppMenu::Registrar(REGISTRAR_SERVICE, REGISTRAR_PATH, QDBusConnection::sessionBus(), 0);

    if (!registrar || !registrar->isValid())
        return false;

    Q_FOREACH(QAction *action, m_menuBar->actions()) {
                if (!action->isSeparator()) {
                    WARN << action->text();
                }
    }

    if (!m_exporter)
        m_exporter = new DBusMenuExporter(m_objectPath, (QMenu *)m_menuBar);

    m_registeredWinId = winId;

    registrar->RegisterWindow(winId, QDBusObjectPath(m_objectPath));

    return true;
}

void
MenuBarAdapter::resetRegisteredWinId()
{
    m_registeredWinId = 0;
}


///////////////////////////////////////////////////////////

/* Helper function, as copy-pasted from Qt 5.2.1 gtk2 platformthemeplugin */
static QString gtkSetting(const gchar *propertyName)
{
    if (!QX11Info::isPlatformX11()) {
        return QString();
    }

    GtkSettings *settings = gtk_settings_get_default();
    gchararray value;
    g_object_get(settings, propertyName, &value, NULL);
    QString str = QString::fromUtf8(value);
    g_free(value);
    return str;
}

/*
 * The GnomeAppMenuPlatformTheme is a platform theme providing the platform
 * menubar functionality with the Qt5 QGnomeTheme look
 */
class GnomeAppMenuPlatformTheme : public QGnomeTheme
{
public:
    GnomeAppMenuPlatformTheme();
    virtual QPlatformMenuItem* createPlatformMenuItem() const {
        return new AppMenuPlatformMenuItem();
    }
    virtual QPlatformMenu* createPlatformMenu() const { return 0; }
    virtual QPlatformMenuBar* createPlatformMenuBar() const;
    virtual QPlatformSystemTrayIcon* createPlatformSystemTrayIcon() const {
        return new AppMenuPlatformSystemTrayIcon();
    }

    virtual QVariant themeHint(QPlatformTheme::ThemeHint hint) const;
};


GnomeAppMenuPlatformTheme::GnomeAppMenuPlatformTheme()
    : QGnomeTheme()
{
    if (QX11Info::isPlatformX11()) {
        int (*oldErrorHandler)(Display *, XErrorEvent *) = XSetErrorHandler(NULL);
        gtk_init(0, 0);
        XSetErrorHandler(oldErrorHandler);
    }
}

QVariant GnomeAppMenuPlatformTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    if (!QX11Info::isPlatformX11()) {
        return QVariant(QVariant::String);
    }

    switch (hint) {
        case QPlatformTheme::SystemIconThemeName:
            return QVariant(gtkSetting("gtk-icon-theme-name"));
        case QPlatformTheme::SystemIconFallbackThemeName:
            return QVariant(gtkSetting("gtk-fallback-icon-theme"));
        default:
            return QGnomeTheme::themeHint(hint);
    }
}


QPlatformMenuBar *
GnomeAppMenuPlatformTheme::createPlatformMenuBar() const
{
    if (useGlobalMenu()) {
        return new AppMenuPlatformMenuBar();
    } else {
        return QGnomeTheme::createPlatformMenuBar();
    }
}


///////////////////////////////////////////////////////////

#ifndef QKDETHEME_STILL_PRIVATE
/*
 * The KdeAppMenuPlatformTheme is a platform theme providing the platform
 * menubar functionality with the Qt5 QKdeTheme look
 */
class KdeAppMenuPlatformTheme : public QKdeTheme
{
public:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    KdeAppMenuPlatformTheme(const QStringList &kdeDirs, int kdeVersion)
        : QKdeTheme(kdeDirs, kdeVersion)
    {
    }
#else
    KdeAppMenuPlatformTheme(const QString &kdeHome, int kdeVersion)
        : QKdeTheme(kdeHome, kdeVersion)
    {
    }
#endif
    virtual QPlatformMenuItem* createPlatformMenuItem() const {
        return new AppMenuPlatformMenuItem();
    }
    virtual QPlatformMenu* createPlatformMenu() const { return 0; }
    virtual QPlatformMenuBar* createPlatformMenuBar() const;
};


QPlatformMenuBar *
KdeAppMenuPlatformTheme::createPlatformMenuBar() const
{
    if (useGlobalMenu()) {
        return new AppMenuPlatformMenuBar();
    } else {
        return QKdeTheme::createPlatformMenuBar();
    }
}
#endif


///////////////////////////////////////////////////////////
const char *AppMenuPlatformThemePlugin::name = "appmenu-qt5";

AppMenuPlatformThemePlugin::AppMenuPlatformThemePlugin(QObject *parent)
{
    Q_UNUSED(parent);
}

QPlatformTheme *
AppMenuPlatformThemePlugin::create(const QString &key, const QStringList &paramList)
{
    if (key.compare(QLatin1String(AppMenuPlatformThemePlugin::name), Qt::CaseInsensitive))
        return 0;

#ifndef QKDETHEME_STILL_PRIVATE
    if (paramList.indexOf("kde") >= 0) {
        // This check is copy-pasted from the Qt5 source code
        // We need to determine the version number of KDE and the kde home dir
        const QByteArray kdeVersionBA = qgetenv("KDE_SESSION_VERSION");
        const int kdeVersion = kdeVersionBA.toInt();
        if (kdeVersion >= 4) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
            // Determine KDE prefixes in the following priority order:
            // - KDEHOME and KDEDIRS environment variables
            // - ~/.kde(<version>)
            // - read prefixes from /etc/kde<version>rc
            // - fallback to /etc/kde<version>

            QStringList kdeDirs;
            const QString kdeHomePathVar = QFile::decodeName(qgetenv("KDEHOME"));
            if (!kdeHomePathVar.isEmpty())
                kdeDirs += kdeHomePathVar;

            const QString kdeDirsVar = QFile::decodeName(qgetenv("KDEDIRS"));
            if (!kdeDirsVar.isEmpty())
                kdeDirs += kdeDirsVar.split(QLatin1Char(':'), QString::SkipEmptyParts);

            const QString kdeVersionHomePath = QDir::homePath() + QStringLiteral("/.kde") + QLatin1String(kdeVersionBA);
            if (QFileInfo(kdeVersionHomePath).isDir())
                kdeDirs += kdeVersionHomePath;

            const QString kdeHomePath = QDir::homePath() + QStringLiteral("/.kde");
            if (QFileInfo(kdeHomePath).isDir())
                kdeDirs += kdeHomePath;

            const QString kdeRcPath = QStringLiteral("/etc/kde") + QLatin1String(kdeVersionBA) + QStringLiteral("rc");
            if (QFileInfo(kdeRcPath).isReadable()) {
                QSettings kdeSettings(kdeRcPath, QSettings::IniFormat);
                kdeSettings.beginGroup(QStringLiteral("Directories-default"));
                kdeDirs += kdeSettings.value(QStringLiteral("prefixes")).toStringList();
            }

            const QString kdeVersionPrefix = QStringLiteral("/etc/kde") + QLatin1String(kdeVersionBA);
            if (QFileInfo(kdeVersionPrefix).isDir())
                kdeDirs += kdeVersionPrefix;

            kdeDirs.removeDuplicates();
            if (!kdeDirs.isEmpty()) {
                return new KdeAppMenuPlatformTheme(kdeDirs, kdeVersion);
            }
            else {
                qWarning("%s: Unable to determine KDE dirs", Q_FUNC_INFO);
                WARN << "Unable to determine KDE dirs, falling back to the gnome theme";
            }
#else
            const QString kdeHomePathVar = QString::fromLocal8Bit(qgetenv("KDEHOME"));
            if (!kdeHomePathVar.isEmpty())
                return new KdeAppMenuPlatformTheme(kdeHomePathVar, kdeVersion);

            const QString kdeVersionHomePath = QDir::homePath() + QStringLiteral("/.kde") + QLatin1String(kdeVersionBA);
            if (QFileInfo(kdeVersionHomePath).isDir())
                return new KdeAppMenuPlatformTheme(kdeVersionHomePath, kdeVersion);

            const QString kdeHomePath = QDir::homePath() + QStringLiteral("/.kde");
            if (QFileInfo(kdeHomePath).isDir())
                return new KdeAppMenuPlatformTheme(kdeHomePath, kdeVersion);
#endif // QT_VERSION_CHECK
        }
        else {
            WARN << "KDE version too old or cannot be properly identified, "
                "falling back to the gnome theme";
        }       
    }
#endif // QKDETHEME_STILL_PRIVATE

    // Fallback path - use the Gnome theme in this case
    return new GnomeAppMenuPlatformTheme();
}


///////////////////////////////////////////////////////////

QT_END_NAMESPACE
