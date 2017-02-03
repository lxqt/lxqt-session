/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org, http://lxde.org/
 *
 * Copyright: 2010-2015 LXQt team
 * Authors:
 *   Paulo Lieuthier <paulolieuthier@gmail.com>
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
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "leavedialog.h"

LeaveDialog::LeaveDialog(QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint),
    ui(new Ui::LeaveDialog),
    mPower(new LXQt::Power(this)),
    mPowerManager(new LXQt::PowerManager(this)),
    mScreensaver(new LXQt::ScreenSaver(this))
{
    ui->setupUi(this);

    ui->lockscreenButton->setIcon(QIcon::fromTheme(QLatin1String("system-lock-screen")));
    ui->logoutButton->setIcon(QIcon::fromTheme(QLatin1String("system-log-out")));
    ui->shutdownButton->setIcon(QIcon::fromTheme(QLatin1String("system-shutdown")));
    ui->rebootButton->setIcon(QIcon::fromTheme(QLatin1String("system-reboot")));
    ui->suspendButton->setIcon(QIcon::fromTheme(QLatin1String("system-suspend")));
    ui->hibernateButton->setIcon(QIcon::fromTheme(QLatin1String("system-suspend-hibernate")));

    /* This is a special dialog. We want to make it hard to ignore.
       We make it:
           * Undraggable
           * Frameless
           * Stays on top of all other windows
           * Present in all desktops
    */
    setWindowFlags((Qt::CustomizeWindowHint | Qt::FramelessWindowHint |
                    Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint));

    ui->logoutButton->setEnabled(mPower->canAction(LXQt::Power::PowerLogout));
    ui->rebootButton->setEnabled(mPower->canAction(LXQt::Power::PowerReboot));
    ui->shutdownButton->setEnabled(mPower->canAction(LXQt::Power::PowerShutdown));
    ui->suspendButton->setEnabled(mPower->canAction(LXQt::Power::PowerSuspend));
    ui->hibernateButton->setEnabled(mPower->canAction(LXQt::Power::PowerHibernate));

    /*
     * Make all the buttons have equal widths
     */
    QVector<QToolButton*> buttons(6);
    buttons[0] = ui->logoutButton;
    buttons[1] = ui->lockscreenButton;
    buttons[2] = ui->suspendButton;
    buttons[3] = ui->hibernateButton;
    buttons[4] = ui->rebootButton;
    buttons[5] = ui->shutdownButton;

    int maxWidth = 0;
    const int N = buttons.size();
    for (int i = 0; i < N; ++i) {
        // Make sure that the button size is adjusted to the text width
        buttons.at(i)->adjustSize();
        maxWidth = qMax(maxWidth, buttons.at(i)->width());
    }
    for (int i = 0; i < N; ++i)
        buttons.at(i)->setMinimumWidth(maxWidth);

    connect(ui->logoutButton,       &QPushButton::clicked, [&] { close(); mPowerManager->logout();    });
    connect(ui->rebootButton,       &QPushButton::clicked, [&] { close(); mPowerManager->reboot();    });
    connect(ui->shutdownButton,     &QPushButton::clicked, [&] { close(); mPowerManager->shutdown();  });
    connect(ui->suspendButton,      &QPushButton::clicked, [&] { close(); mPowerManager->suspend();   });
    connect(ui->hibernateButton,    &QPushButton::clicked, [&] { close(); mPowerManager->hibernate(); });
    connect(ui->cancelButton,       &QPushButton::clicked, [&] { close();                             });
    connect(ui->lockscreenButton,   &QPushButton::clicked, [&] {
        close();
        QEventLoop loop;
        connect(mScreensaver, &LXQt::ScreenSaver::done, &loop, &QEventLoop::quit);
        mScreensaver->lockScreen();
        loop.exec();
    });

}

LeaveDialog::~LeaveDialog()
{
    delete ui;
}

void LeaveDialog::resizeEvent(QResizeEvent* event)
{
    QRect screen = QApplication::desktop()->screenGeometry();
    move((screen.width()  - this->width()) / 2,
         (screen.height() - this->height()) / 2);

}
