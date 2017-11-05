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

#ifndef APPMENUPLATFORMMENU_H
#define APPMENUPLATFORMMENU_H

#include <QtCore/qlist.h>
#include <QtWidgets/qmenu.h>
#include <qpa/qplatformmenu.h>
#include "appmenuplatformmenuitem.h"

class AppMenuPlatformMenu : public QPlatformMenu
{
    Q_OBJECT
    friend class AppMenuPlatformMenuItem;
    friend class AppMenuPlatformSystemTrayIcon;

public:
    AppMenuPlatformMenu();
    virtual ~AppMenuPlatformMenu();

    virtual void insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before);
    virtual void removeMenuItem(QPlatformMenuItem *menuItem);
    virtual void syncMenuItem(QPlatformMenuItem *menuItem);
    virtual void syncSeparatorsCollapsible(bool enable);
    virtual void setTag(quintptr tag);
    virtual quintptr tag() const;
    virtual void setText(const QString &text);
    virtual void setIcon(const QIcon &icon);
    virtual void setEnabled(bool enabled);
    virtual void setVisible(bool visible);
    virtual void setMinimumWidth(int width);
    virtual void setFont(const QFont &font);
    virtual QPlatformMenuItem *menuItemAt(int position) const;
    virtual QPlatformMenuItem *menuItemForTag(quintptr tag) const;
    virtual QPlatformMenuItem *createMenuItem() const;

private:
    Q_DISABLE_COPY(AppMenuPlatformMenu)

    QMenu *m_menu;
    QList<AppMenuPlatformMenuItem *> m_menuitems;
    quintptr m_tag;
};

#endif /* APPMENUPLATFORMMENU_H */
