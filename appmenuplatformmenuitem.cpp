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

AppMenuPlatformMenuItem::AppMenuPlatformMenuItem():
    m_action(new QAction(this)),
    m_tag(0)
{
    connect(m_action, &QAction::triggered, this, &QPlatformMenuItem::activated);
    connect(m_action, &QAction::hovered,   this, &QPlatformMenuItem::hovered);
}

AppMenuPlatformMenuItem::~AppMenuPlatformMenuItem()
{
    delete m_action;
}

void AppMenuPlatformMenuItem::setTag(quintptr tag)
{
    m_tag = tag;
}

quintptr AppMenuPlatformMenuItem::tag() const
{
    return m_tag;
}

void AppMenuPlatformMenuItem::setText(const QString &text)
{
    m_action->setText(text);
}

void AppMenuPlatformMenuItem::setIcon(const QIcon &icon)
{
    m_action->setIcon(icon);
}

void AppMenuPlatformMenuItem::setMenu(QPlatformMenu *menu)
{
    if (menu) {
        AppMenuPlatformMenu *myMenu = qobject_cast<AppMenuPlatformMenu *>(menu);
        m_action->setMenu(myMenu->m_menu);
    }
}

void AppMenuPlatformMenuItem::setVisible(bool isVisible)
{
    m_action->setVisible(isVisible);
}

void AppMenuPlatformMenuItem::setIsSeparator(bool isSeparator)
{
    m_action->setSeparator(isSeparator);
}

void AppMenuPlatformMenuItem::setFont(const QFont &font)
{
    m_action->setFont(font);
}

void AppMenuPlatformMenuItem::setRole(MenuRole role)
{
    QAction::MenuRole newRole;
    switch (role) {
    case TextHeuristicRole:
        newRole = QAction::TextHeuristicRole;
        break;
    case ApplicationSpecificRole:
        newRole = QAction::ApplicationSpecificRole;
        break;
    case AboutRole:
        newRole = QAction::AboutRole;
        break;
    case PreferencesRole:
        newRole = QAction::PreferencesRole;
        break;
    case QuitRole:
        newRole = QAction::QuitRole;
        break;
    default:
        newRole = QAction::NoRole;
    }
    m_action->setMenuRole(newRole);
}

void AppMenuPlatformMenuItem::setCheckable(bool checkable)
{
    m_action->setCheckable(checkable);
}

void AppMenuPlatformMenuItem::setChecked(bool isChecked)
{
    m_action->setChecked(isChecked);
}

void AppMenuPlatformMenuItem::setShortcut(const QKeySequence& shortcut)
{
    m_action->setShortcut(shortcut);
}

void AppMenuPlatformMenuItem::setEnabled(bool enabled)
{
    m_action->setEnabled(enabled);
}

void AppMenuPlatformMenuItem::setIconSize(int size)
{
    Q_UNUSED(size);
}
