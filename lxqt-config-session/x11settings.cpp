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

#include "x11settings.h"
#include "ui_x11settings.h"

#include "../lxqt-session/src/windowmanager.h"
#include "sessionconfigwindow.h"
#include "autostartutils.h"

static const QLatin1String windowManagerKey("window_manager");
static const QLatin1String QtScaleKey("QT_SCALE_FACTOR");
static const QLatin1String x11LockCommandKey("lock_command");
static const QLatin1String GdkScaleKey("GDK_SCALE");
static const QLatin1String openboxValue("openbox");

X11Settings::X11Settings(LXQt::Settings *settings, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    ui(new Ui::X11Settings)
{
    ui->setupUi(this);
    if (QGuiApplication::platformName() == QL1S("wayland"))
        ui->scaleBox->setEnabled(false); // scaling is done by the Wayland compositor

    connect(ui->findWmButton, &QPushButton::clicked, this, &X11Settings::findWmButton_clicked);
    connect(ui->findX11LockCommandButton, &QPushButton::clicked, this, &X11Settings::findX11LockCommandButton_clicked);
    restoreSettings();
}

X11Settings::~X11Settings()
{
    delete ui;
}

void X11Settings::restoreSettings()
{
    QStringList knownWMs;
    const auto wmList = getWindowManagerList(true);
    for (const WindowManager &wm : wmList)
    {
        knownWMs << wm.command;
    }

    QStringList knownX11Locker;
    knownX11Locker << QStringLiteral("i3lock") << QStringLiteral("kscreenlocker") << QStringLiteral("slock") << QStringLiteral("xsecurelock") << QStringLiteral("xlock");

    QString wm = m_settings->value(windowManagerKey, openboxValue).toString();
    SessionConfigWindow::handleCfgComboBox(ui->wmComboBox, knownWMs, wm);

    m_settings->beginGroup(QL1S("Environment"));
    ui->scaleSpinBox->setValue(m_settings->value(QtScaleKey, 1.0).toDouble());
    m_settings->endGroup();

    QString x11LockCommand = m_settings->value(x11LockCommandKey, QString()).toString();
    SessionConfigWindow::handleCfgComboBox(ui->x11LockCommandComboBox, knownX11Locker, x11LockCommand);

    ui->customLockBox->setChecked(!x11LockCommand.isEmpty());
}

void X11Settings::save()
{
    bool doRestart = false;
    const QString windowManager = ui->wmComboBox->currentText();
    const double scaleFactor = ui->scaleSpinBox->value();
    const QString x11LockCommand = ui->customLockBox->isChecked() ? ui->x11LockCommandComboBox->currentText()
                                                                  : QString();

    if (windowManager != m_settings->value(windowManagerKey, openboxValue).toString())
    {
        m_settings->setValue(windowManagerKey, windowManager);
        doRestart = true;
    }

    if (x11LockCommand != m_settings->value(x11LockCommandKey, QString()).toString())
    {
        if (x11LockCommand.isEmpty())
            m_settings->remove(x11LockCommandKey);
        else
            m_settings->setValue(x11LockCommandKey, x11LockCommand);
        doRestart = true;
    }

    bool scaleChanged = false;
    m_settings->beginGroup(QL1S("Environment"));
    if (scaleFactor != m_settings->value(QtScaleKey, 1.0).toDouble()
        || scaleFactor != m_settings->value(GdkScaleKey, 1.0).toDouble())
    {
        m_settings->setValue(QtScaleKey, scaleFactor);
        m_settings->setValue(GdkScaleKey, scaleFactor);
        scaleChanged = true;
    }
    m_settings->endGroup();
    if (scaleChanged)
        emit scaleFactorChanged(); // will update EnvironmentPage in SessionConfigWindow

    if (doRestart || scaleChanged)
        emit needRestart();
}

void X11Settings::findWmButton_clicked()
{
    SessionConfigWindow::updateCfgComboBox(ui->wmComboBox, tr("Select a window manager"));
}

void X11Settings::findX11LockCommandButton_clicked()
{
    SessionConfigWindow::updateCfgComboBox(ui->x11LockCommandComboBox, tr("Select a screenlocker"));
}
