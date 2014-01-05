/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

SessionApplication::SessionApplication(int& argc, char** argv) : LxQt::Application(argc, argv)
{
    char* session = NULL;
    char* winmanager = NULL;
    int c;
    while ((c = getopt (argc, argv, "c:w:")) != -1)
    {
        if (c == 'c')
        {
            session = optarg;
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
    lxqt_setenv("XDG_MENU_PREFIX", "lxqt-");

    modman = new LxQtModuleManager(session, winmanager);
    new SessionDBusAdaptor(modman);
    // connect to D-Bus and register as an object:
    QDBusConnection::sessionBus().registerService("org.lxqt.session");
    QDBusConnection::sessionBus().registerObject("/LxQtSession", modman);
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
