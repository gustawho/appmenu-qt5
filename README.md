# Usage

After installing the appmenu-qt5 platform theme (to the qt5/plugins/platformthemes/ plugin directory) for the appmenu support to work
you need to force Qt5 to use this platform theme plugin. This can be done by setting the QT_QPA_PLATFORMTHEME environment variable to
appmenu-qt5, e.g.: `QT_QPA_PLATFORMTHEME=appmenu-qt5`

Now every Qt5 application that is started in the environment will export its menu through DBus.

The source also provides a profile.d script that sets the environment to enable appmenu-qt5 as the platformtheme by default. To install
this script, simply run `qmake CONFIG+=enable-by-default` when configuring the source before building.
