/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXDE-Qt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <LXQt/Application>
#include <LXQt/PowerManager>
#include <LXQt/ScreenSaver>
#include <LXQt/Translator>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QCoreApplication>

#include "leavedialog.h"

int main(int argc, char *argv[])
{
    LXQt::Application a(argc, argv);
    LXQt::Translator::translateApplication();

    LXQt::PowerManager powermanager(&a);
    LXQt::ScreenSaver screensaver(&a);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("lxqt-leave"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption logoutOption(QStringLiteral("logout"), QCoreApplication::translate("main", "Logout."));
    parser.addOption(logoutOption);

    QCommandLineOption lockscreenOption(QStringLiteral("lockscreen"), QCoreApplication::translate("main", "Lockscreen."));
    parser.addOption(lockscreenOption);

    QCommandLineOption suspendOption(QStringLiteral("suspend"), QCoreApplication::translate("main", "Suspend."));
    parser.addOption(suspendOption);

    QCommandLineOption hibernateOption(QStringLiteral("hibernate"), QCoreApplication::translate("main", "Hibernate."));
    parser.addOption(hibernateOption);

    QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QCoreApplication::translate("main", "Shutdown."));
    parser.addOption(shutdownOption);

    QCommandLineOption rebootOption(QStringLiteral("reboot"), QCoreApplication::translate("main", "Reboot."));
    parser.addOption(rebootOption);

    parser.process(a);

    if (parser.isSet(logoutOption)) {
        powermanager.logout();
        return 0;
    }

    if (parser.isSet(lockscreenOption)) {
        a.connect(&screensaver, &LXQt::ScreenSaver::done, &a, &LXQt::Application::quit);
        screensaver.lockScreen();
        a.exec();
        return 0;
    }

    if (parser.isSet(suspendOption)) {
        powermanager.suspend();
        return 0;
    }

    if (parser.isSet(hibernateOption)) {
        powermanager.hibernate();
        return 0;
    }

    if (parser.isSet(shutdownOption)) {
        powermanager.shutdown();
        return 0;
    }

    if (parser.isSet(rebootOption)) {
        powermanager.reboot();
        return 0;
    }

    LeaveDialog dialog;
    dialog.setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                Qt::AlignCenter,
                dialog.size(),
                qApp->desktop()->screenGeometry(QCursor::pos())));
    dialog.setMaximumSize(dialog.minimumSize());
    return dialog.exec();
}
