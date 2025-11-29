/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2024 LXQt team
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

#include "waylandsettings.h"
#include "ui_waylandsettings.h"

#include "sessionconfigwindow.h"
#include "../lxqt-session/src/windowmanager.h"

static const QLatin1String compositorKey("compositor");
static const QLatin1String wayLockCommandKey("lock_command_wayland");

WaylandSettings::WaylandSettings(LXQt::Settings *settings, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    ui(new Ui::WaylandSettings)
{
    ui->setupUi(this);
    connect(ui->findCompositorButton, &QPushButton::clicked, this, &WaylandSettings::findCompositorButton_clicked);
    connect(ui->findWayLockCommandButton, &QPushButton::clicked, this, &WaylandSettings::findWayLockCommandButton_clicked);
    restoreSettings();
}

WaylandSettings::~WaylandSettings()
{
    delete ui;
}

void WaylandSettings::restoreSettings()
{
    QStringList knownCompositors;
    const auto wmList = getWindowManagerList(true, true);
    for (const WindowManager &wm : wmList)
    {
        knownCompositors << wm.command;
    }

    QString compositor = m_settings->value(compositorKey).toString();
    SessionConfigWindow::handleCfgComboBox(ui->compositorComboBox, knownCompositors, compositor);

    QStringList knownWayLocker;
    knownWayLocker << QStringLiteral("swaylock") << QStringLiteral("waylock") << QStringLiteral("waylock-fancy") << QStringLiteral("hyprlock");

    QString wayLockCommand = m_settings->value(wayLockCommandKey).toString();
    SessionConfigWindow::handleCfgComboBox(ui->wayLockCommandComboBox, knownWayLocker, wayLockCommand);
}

void WaylandSettings::save()
{
    bool doRestart = false;
    const QString compositor = ui->compositorComboBox->currentText();
    const QString wayLockCommand = ui->wayLockCommandComboBox->currentText();

    if (compositor != m_settings->value(compositorKey).toString())
    {
        m_settings->setValue(compositorKey, compositor);
        doRestart = true;
    }

    if (wayLockCommand != m_settings->value(wayLockCommandKey).toString())
    {
        m_settings->setValue(wayLockCommandKey, wayLockCommand);
        doRestart = true;
    }
    if (doRestart)
        emit needRestart();
}

void WaylandSettings::findCompositorButton_clicked()
{
    SessionConfigWindow::updateCfgComboBox(ui->compositorComboBox, tr("Select a Wayland Compositor"));
}

void WaylandSettings::findWayLockCommandButton_clicked()
{
    SessionConfigWindow::updateCfgComboBox(ui->wayLockCommandComboBox, tr("Select a Screenlocker for Wayland"));
}
