/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LxQt - a lightweight, Qt based, desktop toolset
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
    : QDialog(parent),
    ui(new Ui::LeaveDialog),
    mPower(new LxQt::Power(this)),
    mScreensaver(new LxQt::ScreenSaver(this))
{
    ui->setupUi(this);

    ui->logoutButton->setEnabled(mPower->canAction(LxQt::Power::PowerLogout));
    ui->rebootButton->setEnabled(mPower->canAction(LxQt::Power::PowerReboot));
    ui->shutdownButton->setEnabled(mPower->canAction(LxQt::Power::PowerShutdown));
    ui->suspendButton->setEnabled(mPower->canAction(LxQt::Power::PowerSuspend));
    ui->hibernateButton->setEnabled(mPower->canAction(LxQt::Power::PowerHibernate));

    connect(ui->logoutButton,       &QPushButton::clicked, [&] { close(); mPower->logout(); });
    connect(ui->rebootButton,       &QPushButton::clicked, [&] { close(); mPower->reboot(); });
    connect(ui->shutdownButton,     &QPushButton::clicked, [&] { close(); mPower->shutdown(); });
    connect(ui->suspendButton,      &QPushButton::clicked, [&] { close(); mPower->suspend(); });
    connect(ui->hibernateButton,    &QPushButton::clicked, [&] { close(); mPower->hibernate(); });
    connect(ui->lockscreenButton,   &QPushButton::clicked, [&] {
        close();
        QEventLoop loop;
        connect(mScreensaver, &LxQt::ScreenSaver::done, &loop, &QEventLoop::quit);
        mScreensaver->lockScreen();
        loop.exec();
    });

    connect(ui->cancelButton, &QPushButton::clicked, [&] { close(); });
}

LeaveDialog::~LeaveDialog()
{
    delete ui;
}
