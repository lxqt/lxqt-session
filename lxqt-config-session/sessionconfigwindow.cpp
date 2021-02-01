/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2011 LXQt team
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

#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>

#include <LXQt/Globals>
#include <LXQt/Settings>

#include "sessionconfigwindow.h"
#include "../lxqt-session/src/windowmanager.h"
#include "basicsettings.h"
#include "autostartpage.h"
#include "environmentpage.h"
#include "userlocationspage.h"


SessionConfigWindow::SessionConfigWindow() :
      LXQt::ConfigDialog(tr("LXQt Session Settings"), new LXQt::Settings(QSL("session")), nullptr)
{
    BasicSettings* basicSettings = new BasicSettings(mSettings, this);
    addPage(basicSettings, tr("Basic Settings"), QSL("preferences-desktop-display-color"));
    connect(basicSettings, &BasicSettings::needRestart, this, &SessionConfigWindow::setRestart);
    connect(this, &SessionConfigWindow::reset, basicSettings, &BasicSettings::restoreSettings);
    connect(this, &SessionConfigWindow::save,  basicSettings, &BasicSettings::save);

    UserLocationsPage* userLocations = new UserLocationsPage(this);
    addPage(userLocations, tr("User Directories"), QStringLiteral("folder"));
    connect(userLocations, &UserLocationsPage::needRestart, this, &SessionConfigWindow::setRestart);
    connect(this, &SessionConfigWindow::reset, userLocations, &UserLocationsPage::restoreSettings);
    connect(this, &SessionConfigWindow::save,  userLocations, &UserLocationsPage::save);

    AutoStartPage* autoStart = new AutoStartPage(this);
    addPage(autoStart, tr("Autostart"), QSL("preferences-desktop-launch-feedback"));
    connect(autoStart, &AutoStartPage::needRestart, this, &SessionConfigWindow::setRestart);
    connect(this, &SessionConfigWindow::reset, autoStart, &AutoStartPage::restoreSettings);
    connect(this, &SessionConfigWindow::save, autoStart, &AutoStartPage::save);

    EnvironmentPage* environmentPage = new EnvironmentPage(mSettings, this);
    addPage(environmentPage, tr("Environment (Advanced)"), QSL("preferences-system-session-services"));
    connect(environmentPage, &EnvironmentPage::needRestart, this, &SessionConfigWindow::setRestart);
    connect(this, &SessionConfigWindow::reset, environmentPage, &EnvironmentPage::restoreSettings);
    connect(this, &SessionConfigWindow::save,  environmentPage, &EnvironmentPage::save);

    // sync Default Apps and Environment
    environmentPage->restoreSettings();
    connect(this, &SessionConfigWindow::reset, this, &SessionConfigWindow::clearRestart);
    m_restart = false;

    adjustSize();
}

SessionConfigWindow::~SessionConfigWindow()
{
    delete mSettings;
}

void SessionConfigWindow::closeEvent(QCloseEvent * event)
{
    LXQt::ConfigDialog::closeEvent(event);
    if (m_restart) {
        QMessageBox::information(this, tr("Session Restart Required"),
                                tr("Some settings will not take effect until the next log in.")
                                );
    }
}

void SessionConfigWindow::handleCfgComboBox(QComboBox * cb,
                                            const QStringList &availableValues,
                                            const QString &value
                                           )
{
    QStringList realValues;
    for (const QString &s : availableValues)
    {
        if (findProgram(s))
            realValues << s;
    }
    cb->clear();
    cb->addItems(realValues);

    int ix = cb->findText(value);
    if (ix == -1)
        cb->lineEdit()->setText(value);
    else
        cb->setCurrentIndex(ix);
}

void SessionConfigWindow::updateCfgComboBox(QComboBox * cb,
                                            const QString &prompt
                                           )
{
    QString fname = QFileDialog::getOpenFileName(cb, prompt, QSL("/usr/bin/"));
    if (fname.isEmpty())
        return;

    QFileInfo fi(fname);
    if (!fi.exists() || !fi.isExecutable())
        return;

    cb->lineEdit()->setText(fname);
}

void SessionConfigWindow::setRestart()
{
    m_restart = true;
}

void SessionConfigWindow::clearRestart()
{
    m_restart = false;
}
