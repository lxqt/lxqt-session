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

#ifndef SESSIONAPPLICATION_H
#define SESSIONAPPLICATION_H

#include <lxqt/lxqtapplication.h>
#include <lxqt/lxqtsettings.h>

class LxQtModuleManager;

class SessionApplication : public LxQt::Application
{
    Q_OBJECT
public:
    SessionApplication(int& argc, char** argv);
    ~SessionApplication();
    virtual bool x11EventFilter(XEvent* event );

private Q_SLOTS:
    bool startup();

private:
    void loadEnvironmentSettings(LxQt::Settings& settings);
    void loadKeyboardSettings(LxQt::Settings& settings);
    void loadMouseSettings(LxQt::Settings& settings);

    void mergeXrdb(const char* content, int len);
    void setLeftHandedMouse(bool mouse_left_handed);
private:
    LxQtModuleManager* modman;
    QString configName;
};

#endif // SESSIONAPPLICATION_H
