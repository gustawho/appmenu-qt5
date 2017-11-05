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

#ifndef APPMENUPLATFORMMENUITEM_H
#define APPMENUPLATFORMMENUITEM_H

#include <QtWidgets/qaction.h>
#include <qpa/qplatformmenu.h>

class AppMenuPlatformMenuItem : public QPlatformMenuItem
{
    Q_OBJECT
    friend class AppMenuPlatformMenu;

public:
    AppMenuPlatformMenuItem();
    virtual ~AppMenuPlatformMenuItem();

    virtual void setTag(quintptr tag);
    virtual quintptr tag() const;
    virtual void setText(const QString &text);
    virtual void setIcon(const QIcon &icon);
    virtual void setMenu(QPlatformMenu *menu);
    virtual void setVisible(bool isVisible);
    virtual void setIsSeparator(bool isSeparator);
    virtual void setFont(const QFont &font);
    virtual void setRole(MenuRole role);
    virtual void setCheckable(bool checkable);
    virtual void setChecked(bool isChecked);
    virtual void setShortcut(const QKeySequence& shortcut);
    virtual void setEnabled(bool enabled);
    virtual void setIconSize(int size);

private:
    Q_DISABLE_COPY(AppMenuPlatformMenuItem)

    QAction *m_action;
    quintptr m_tag;
};

#endif /* APPMENUPLATFORMMENUITEM_H */
