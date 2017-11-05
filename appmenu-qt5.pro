TARGET = appmenu-qt5

QT += core-private gui-private theme_support_private fontdatabase_support_private eventdispatcher_support_private dbus widgets x11extras
DBUS_INTERFACES += com.canonical.AppMenu.Registrar.xml
DBUS_ADAPTORS += org.kde.StatusNotifierItem.xml
QDBUSXML2CPP_ADAPTOR_HEADER_FLAGS += -i dbusstructures.h

CONFIG += X11 link_pkgconfig debug
PKGCONFIG += dbusmenu-qt5 gtk+-2.0
DESTDIR = ./

HEADERS += \
        appmenuplatformmenuitem.h \
        appmenuplatformmenu.h \
        appmenuplatformmenubar.h \
        appmenuplatformsystemtrayicon.h \
        dbusstructures.h \
        iconcache.h

SOURCES += \
        appmenuplatformmenuitem.cpp \
        appmenuplatformmenu.cpp \
        appmenuplatformmenubar.cpp \
        appmenuplatformsystemtrayicon.cpp \
        dbusstructures.cpp \
        iconcache.cpp

PLUGIN_TYPE = platformthemes
PLUGIN_CLASS_NAME = AppMenuPlatformThemePlugin
load(qt_plugin)
