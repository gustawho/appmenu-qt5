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

#ifndef APPMENUPLATFORMMENUBAR_H
#define APPMENUPLATFORMMENUBAR_H

#include <qpa/qplatformmenu.h>
#include <qpa/qplatformtheme.h>
#include <qpa/qplatformthemeplugin.h>

class QMenuBar;
class QWindow;
class QString;

class MenuBarAdapter;

class AppMenuPlatformMenuBar : public QPlatformMenuBar
{
    Q_OBJECT
public:
    AppMenuPlatformMenuBar();
    virtual ~AppMenuPlatformMenuBar();

    virtual void insertMenu(QPlatformMenu *menu, QPlatformMenu* before);
    virtual void removeMenu(QPlatformMenu *menu);
    virtual void syncMenu(QPlatformMenu *menuItem);
    virtual void handleReparent(QWindow *newParentWindow);
    virtual QPlatformMenu *menuForTag(quintptr tag) const;

private:

    QMenuBar *m_menubar;
    QWindow *m_window;
    MenuBarAdapter *m_adapter;
    QString m_objectPath;
};


class AppMenuPlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "appmenu-qt5.json")
public:
    AppMenuPlatformThemePlugin(QObject *parent = 0);

    virtual QPlatformTheme *create(const QString &key, const QStringList &paramList);

    static const char *name;
};

QT_END_NAMESPACE

#endif
