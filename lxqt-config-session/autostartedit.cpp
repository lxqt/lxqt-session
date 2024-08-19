/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright (C) 2011  Alec Moskvin <alecm@gmx.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QFileDialog>

#include "autostartedit.h"
#include "ui_autostartedit.h"

#include <LXQt/Globals>

AutoStartEdit::AutoStartEdit(const QString& name, const QString& command, bool needTray, bool x11Only, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AutoStartEdit)
{
    ui->setupUi(this);
    ui->nameEdit->setFocus();
    ui->nameEdit->setText(name);
    ui->commandEdit->setText(command);
    ui->trayCheckBox->setChecked(needTray);
    ui->x11CheckBox->setChecked(x11Only);
    connect(ui->browseButton, &QPushButton::clicked, this, &AutoStartEdit::browse);
}

QString AutoStartEdit::name() const
{
    return ui->nameEdit->text();
}

QString AutoStartEdit::command() const
{
    return ui->commandEdit->text();
}

bool AutoStartEdit::needTray() const
{
    return ui->trayCheckBox->isChecked();
}

bool AutoStartEdit::x11Only() const
{
    return ui->x11CheckBox->isChecked();
}

void AutoStartEdit::browse()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Application"), QSL("/usr/bin/"));
    if (!filePath.isEmpty())
        ui->commandEdit->setText(filePath);
}

AutoStartEdit::~AutoStartEdit()
{
    delete ui;
}
