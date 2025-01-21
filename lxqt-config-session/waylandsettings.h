/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2024 LXQt team

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

#ifndef WAYLANDSETTINGS_H
#define WAYLANDSETTINGS_H

#include <QWidget>
#include <LXQt/Settings>

namespace Ui {
class WaylandSettings;
}

class WaylandSettings : public QWidget
{
    Q_OBJECT

public:
    explicit WaylandSettings(LXQt::Settings *settings, QWidget *parent = nullptr);
    ~WaylandSettings() override;

signals:
    void needRestart();

public slots:
    void restoreSettings();
    void save();

private:
    LXQt::Settings* m_settings;
    Ui::WaylandSettings* ui;

private slots:
    void findCompositorButton_clicked();
    void findWayLockCommandButton_clicked();
};

#endif // WAYLANDSETTINGS_H
