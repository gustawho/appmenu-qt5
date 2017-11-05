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

#include "appmenuplatformmenuitem.h"
#include "appmenuplatformmenu.h"

AppMenuPlatformMenu::AppMenuPlatformMenu():
    m_menu(new QMenu()),
    m_tag(0)
{
    connect(m_menu, &QMenu::aboutToShow, this, &QPlatformMenu::aboutToShow);
    connect(m_menu, &QMenu::aboutToHide, this, &QPlatformMenu::aboutToHide);
}

AppMenuPlatformMenu::~AppMenuPlatformMenu()
{
    delete m_menu;
}

void AppMenuPlatformMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
    AppMenuPlatformMenuItem *myMenuItem = qobject_cast<AppMenuPlatformMenuItem *>(menuItem);
    AppMenuPlatformMenuItem *myBefore = qobject_cast<AppMenuPlatformMenuItem *>(before);

    QAction *beforeAction = myBefore ? myBefore->m_action : Q_NULLPTR;
    m_menu->insertAction(beforeAction, myMenuItem->m_action);

    if (!myBefore) {
        m_menuitems.append(myMenuItem);
        return;
    }

    for (int i = 0; i < m_menuitems.size(); ++i) {
        if (m_menuitems.at(i)->m_action == beforeAction) {
            m_menuitems.insert(i, myMenuItem);
            return;
        }
    }
}

void AppMenuPlatformMenu::removeMenuItem(QPlatformMenuItem *menuItem)
{
    AppMenuPlatformMenuItem *myMenuItem = qobject_cast<AppMenuPlatformMenuItem *>(menuItem);
    m_menuitems.removeOne(myMenuItem);
    m_menu->removeAction(myMenuItem->m_action);
}

void AppMenuPlatformMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
    Q_UNUSED(menuItem);
}

void AppMenuPlatformMenu::syncSeparatorsCollapsible(bool enable)
{
    m_menu->setSeparatorsCollapsible(enable);
}

void AppMenuPlatformMenu::setTag(quintptr tag)
{
    m_tag = tag;
}

quintptr AppMenuPlatformMenu::tag() const
{
    return m_tag;
}

void AppMenuPlatformMenu::setText(const QString &text)
{
    m_menu->setTitle(text);
}

void AppMenuPlatformMenu::setIcon(const QIcon &icon)
{
    m_menu->setIcon(icon);
}

void AppMenuPlatformMenu::setEnabled(bool enabled)
{
    m_menu->setEnabled(enabled);
}

void AppMenuPlatformMenu::setVisible(bool visible)
{
    m_menu->setVisible(visible);
}

void AppMenuPlatformMenu::setMinimumWidth(int width)
{
    m_menu->setMinimumWidth(width);
}

void AppMenuPlatformMenu::setFont(const QFont &font)
{
    m_menu->setFont(font);
}

QPlatformMenuItem *AppMenuPlatformMenu::menuItemAt(int position) const
{
    return m_menuitems.at(position);
}

QPlatformMenuItem *AppMenuPlatformMenu::menuItemForTag(quintptr tag) const
{
    foreach (AppMenuPlatformMenuItem *item, m_menuitems) {
        if (item->m_tag == tag) {
            return item;
        }
    }
    return Q_NULLPTR;
}

QPlatformMenuItem *AppMenuPlatformMenu::createMenuItem() const
{
    return new AppMenuPlatformMenuItem();
}
