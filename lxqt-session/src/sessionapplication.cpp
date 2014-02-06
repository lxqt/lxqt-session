/*
 * Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#include "sessionapplication.h"
#include "sessiondbusadaptor.h"
#include "lxqtmodman.h"
#include <unistd.h>
#include <alloca.h>
#include <lxqt/lxqtsettings.h>
#include <QProcess>
#include <QDebug>

#include <QX11Info>
// XKB, this should be disabled in Wayland?
#include <X11/XKBlib.h>

SessionApplication::SessionApplication(int& argc, char** argv) : LxQt::Application(argc, argv)
{
    char* winmanager = NULL;
    int c;
    while ((c = getopt (argc, argv, "c:w:")) != -1)
    {
        if (c == 'c')
        {
            configName = optarg;
            break;
        }
        else if (c == 'w')
        {
            winmanager = optarg;
            break;
        }
    }

#ifdef PATH_PREPEND
    // PATH for out own bundled XDG tools
    lxqt_setenv_prepend("PATH", PATH_PREPEND);
#endif // PATH_PREPEND

    // special variable for LxQt environment menu
    // NOTE: it's better not to hard-code lxqt- here.
    // lxqt_setenv("XDG_MENU_PREFIX", "lxqt-");

    if(configName.isEmpty())
      configName = "session";

    // tell the world which config file we're using.
    qputenv("LXQT_SESSION_CONFIG", configName.toUtf8());

    modman = new LxQtModuleManager(winmanager);
    new SessionDBusAdaptor(modman);
    // connect to D-Bus and register as an object:
    QDBusConnection::sessionBus().registerService("org.lxqt.session");
    QDBusConnection::sessionBus().registerObject("/LxQtSession", modman);

    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));
}

SessionApplication::~SessionApplication()
{
    delete modman;
}

bool SessionApplication::x11EventFilter(XEvent* event)
{
    modman->x11EventFilter(event);
    return false;
}

bool SessionApplication::startup()
{
    LxQt::Settings settings(configName);
    qDebug() << __FILE__ << ":" << __LINE__ << "Session" << configName << "about to launch (default 'session')";

    loadEnvironmentSettings(settings);
    loadFontSettings(settings);
    loadKeyboardSettings(settings);
    loadMouseSettings(settings);

    // launch module manager and autostart apps
    modman->startup(settings);

    return true;
}

void SessionApplication::mergeXrdb(const char* content, int len)
{
    qDebug() << "xrdb:" << content;
    QProcess xrdb;
    xrdb.start("xrdb -merge -");
    xrdb.write(content, len);
    xrdb.closeWriteChannel();
    xrdb.waitForFinished();
}

void SessionApplication::loadEnvironmentSettings(LxQt::Settings& settings)
{
    // first - set some user defined environment variables (like TERM...)
    settings.beginGroup("Environment");
    QByteArray envVal;
    Q_FOREACH (QString i, settings.childKeys())
    {
        envVal = settings.value(i).toByteArray();
        lxqt_setenv(i.toUtf8().constData(), envVal);
    }
    settings.endGroup();
}

void SessionApplication::loadKeyboardSettings(LxQt::Settings& settings)
{
    settings.beginGroup("Keyboard");
    XKeyboardControl values;
    /* Keyboard settings */
    unsigned int delay, interval;
    if(XkbGetAutoRepeatRate(QX11Info::display(), XkbUseCoreKbd, (unsigned int*) &delay, (unsigned int*) &interval))
    {
        delay = settings.value("delay", delay).toUInt();
        interval = settings.value("interval", interval).toUInt();
        XkbSetAutoRepeatRate(QX11Info::display(), XkbUseCoreKbd, delay, interval);
    }

    bool beep = settings.value("beep").toBool();
    values.bell_percent = beep ? -1 : 0;
    XChangeKeyboardControl(QX11Info::display(), KBBellPercent, &values);
    settings.endGroup();
}

void SessionApplication::loadMouseSettings(LxQt::Settings& settings)
{
    settings.beginGroup("Mouse");

    // mouse cursor (does this work?)
    QString cursorTheme = settings.value("cursor_theme").toString();
    int cursorSize = settings.value("cursor_size").toInt();
    QByteArray buf;
    if(!cursorTheme.isEmpty()) {
        buf += "Xcursor.theme:";
        buf += cursorTheme;
        buf += '\n';
    }
    if(cursorSize > 0) {
        buf += "Xcursor.size:";
        buf += QString("%1").arg(cursorSize);
        buf += '\n';
    }
    if(!buf.isEmpty()) {
        buf += "Xcursor.theme_core:true\n";
        mergeXrdb(buf.constData(), buf.length());
    }

    // other mouse settings
    int accel_factor = settings.value("accel_factor").toInt();
    int accel_threshold = settings.value("accel_threshold").toInt();
    if(accel_factor || accel_threshold)
        XChangePointerControl(QX11Info::display(), accel_factor != 0, accel_threshold != 0, accel_factor, 10, accel_threshold);

    // left handed mouse?
    bool left_handed = settings.value("left_handed", false).toBool();
    setLeftHandedMouse(left_handed);

    settings.endGroup();
}

void SessionApplication::loadFontSettings(LxQt::Settings& settings)
{
    // set some Xft config values, such as antialiasing & subpixel
    // may call mergeXrdb() to do it. (will this work?)
    // font settings of gtk+ programs are controlled by gtkrc and settings.ini files.
    settings.beginGroup("Font");
    QByteArray buf;
    bool antialias = settings.value("antialias", true).toBool();
    buf += "Xft.antialias:";
    buf += antialias ? "true" : "false";
    buf += '\n';

    buf += "Xft.rgba:rgb\n";

    int dpi = settings.value("dpi", 96).toInt();
    buf += "Xft.dpi:";
    buf += QString("%1").arg(dpi);
    buf += '\n';

    bool hinting = settings.value("hintint", true).toBool();
    buf += "Xft.hinting";
    buf += hinting ? "true" : "false";
    buf += '\n';

    buf += "Xft.hintstyle:hintslight\n";
    if(!buf.isEmpty())
        mergeXrdb(buf.constData(), buf.length());
    settings.endGroup();
}

/* This function is taken from Gnome's control-center 2.6.0.3 (gnome-settings-mouse.c) and was modified*/
#define DEFAULT_PTR_MAP_SIZE 128
void SessionApplication::setLeftHandedMouse(bool mouse_left_handed)
{
    unsigned char *buttons;
    int n_buttons, i;
    int idx_1 = 0, idx_3 = 1;

    buttons = (unsigned char*)alloca (DEFAULT_PTR_MAP_SIZE);
    n_buttons = XGetPointerMapping(QX11Info::display(), buttons, DEFAULT_PTR_MAP_SIZE);
    if (n_buttons > DEFAULT_PTR_MAP_SIZE)
    {
        buttons = (unsigned char*)alloca (n_buttons);
        n_buttons = XGetPointerMapping(QX11Info::display(), buttons, n_buttons);
    }

    for (i = 0; i < n_buttons; i++)
    {
        if (buttons[i] == 1)
            idx_1 = i;
        else if (buttons[i] == ((n_buttons < 3) ? 2 : 3))
            idx_3 = i;
    }

    if ((mouse_left_handed && idx_1 < idx_3) ||
        (!mouse_left_handed && idx_1 > idx_3))
    {
        buttons[idx_1] = ((n_buttons < 3) ? 2 : 3);
        buttons[idx_3] = 1;
        XSetPointerMapping(QX11Info::display(), buttons, n_buttons);
    }
}
