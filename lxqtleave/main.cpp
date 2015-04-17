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

#include "leavedialog.h"

int main(int argc, char *argv[])
{
    LxQt::Application a(argc, argv);
    LxQt::Translator::translateApplication();

    LxQt::PowerManager powermanager(&a);
    LxQt::ScreenSaver screensaver(&a);

    for (int i = 1; i < argc; ++i)
    {
        QString arg = QString::fromLocal8Bit(argv[i]);

        if (arg == QStringLiteral("--logout"))
        {
            powermanager.logout();
            return 0;
        }

        if (arg == QStringLiteral("--suspend"))
        {
            powermanager.suspend();
            return 0;
        }

        if (arg == QStringLiteral("--hibernate"))
        {
            powermanager.hibernate();
            return 0;
        }

        if (arg == QStringLiteral("--shutdown"))
        {
            powermanager.shutdown();
            return 0;
        }

        if (arg == QStringLiteral("--reboot"))
        {
            powermanager.reboot();
            return 0;
        }

        if (arg == QStringLiteral("--lockscreen"))
        {
            a.connect(&screensaver, &LxQt::ScreenSaver::done, &a, &LxQt::Application::quit);
            screensaver.lockScreen();
            a.exec();
            return 0;
        }

        if (arg == QStringLiteral("--gui"))
        {
            LeaveDialog dialog;
            dialog.setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                                    Qt::AlignCenter,
                                                    dialog.size(),
                                                    qApp->desktop()->screenGeometry(QCursor::pos())));
            dialog.setMaximumSize(dialog.minimumSize());
            dialog.exec();
        }
    }
}
