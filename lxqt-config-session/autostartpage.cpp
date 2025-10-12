/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright (C) 2012  Alec Moskvin <alecm@gmx.com>
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

#include "autostartpage.h"
#include "ui_autostartpage.h"

#include "autostartedit.h"
#include "autostartutils.h"

#include <LXQt/Globals>

#include <QMessageBox>

AutoStartPage::AutoStartPage(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::AutoStartPage)
{
    ui->setupUi(this);

    connect(ui->addButton,    &QPushButton::clicked, this, &AutoStartPage::addButton_clicked);
    connect(ui->editButton,   &QPushButton::clicked, this, &AutoStartPage::editButton_clicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &AutoStartPage::deleteButton_clicked);

    restoreSettings();
}

AutoStartPage::~AutoStartPage()
{
    delete ui;
}

void AutoStartPage::restoreSettings()
{
    QAbstractItemModel* oldModel = ui->autoStartView->model();
    mXdgAutoStartModel = new AutoStartItemModel(ui->autoStartView);
    ui->autoStartView->setModel(mXdgAutoStartModel);
    delete oldModel;
    ui->autoStartView->setExpanded(mXdgAutoStartModel->index(0, 0), true);
    ui->autoStartView->setExpanded(mXdgAutoStartModel->index(1, 0), true);
    updateButtons();
    connect(mXdgAutoStartModel, &AutoStartItemModel::dataChanged, this, &AutoStartPage::updateButtons);
    connect(ui->autoStartView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AutoStartPage::selectionChanged);
}

void AutoStartPage::save()
{
    bool doRestart = false;

    /*
     * Get the previous settings
     */
    QMap<QString, AutostartItem> previousItems(AutostartItem::createItemMap());
    QMutableMapIterator<QString, AutostartItem> i(previousItems);
    while (i.hasNext()) {
        i.next();
        if (AutostartUtils::isLXQtModule(i.value().file()))
            i.remove();
    }


    /*
     * Get the settings from the Ui
     */
    QMap<QString, AutostartItem> currentItems = mXdgAutoStartModel->items();
    QMutableMapIterator<QString, AutostartItem> j(currentItems);

    while (j.hasNext()) {
        j.next();
        if (AutostartUtils::isLXQtModule(j.value().file()))
            j.remove();
    }


    /* Compare the settings */

    if (previousItems.count() != currentItems.count())
    {
        doRestart = true;
    }
    else
    {
        QMap<QString, AutostartItem>::const_iterator k = currentItems.constBegin();
        while (k != currentItems.constEnd())
        {
            if (previousItems.contains(k.key()))
            {
                if (k.value().file() != previousItems.value(k.key()).file())
                {
                    doRestart = true;
                    break;
                }
            }
            else
            {
                doRestart = true;
                break;
            }
            ++k;
        }
    }

    if (doRestart)
        emit needRestart();

    mXdgAutoStartModel->writeChanges();
}

void AutoStartPage::addButton_clicked()
{
    AutoStartEdit edit(QString(), QString(), false, false, this);
    bool success = false;
    while (!success && edit.exec() == QDialog::Accepted)
    {
        const auto trimmedName = edit.name().trimmed();
        const auto trimmedCommand = edit.command().trimmed();
        QModelIndex index = ui->autoStartView->selectionModel()->currentIndex();
        if (trimmedName.isEmpty() || trimmedCommand.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("Please provide Name and Command"));
            continue;
        }
        if (trimmedName.startsWith(QL1S(".")) || trimmedName.contains(QL1S("/")))
        {
            QMessageBox::critical(this, tr("Error"), tr("Name should not start with dot or contain slash"));
            continue;
        }
        XdgDesktopFile file(XdgDesktopFile::ApplicationType, trimmedName, trimmedCommand);
        if (edit.needTray())
            file.setValue(QL1S("X-LXQt-Need-Tray"), true);
        if (edit.x11Only())
            file.setValue(QL1S("X-LXQt-X11-Only"), true);
        if (mXdgAutoStartModel->setEntry(index, file))
            success = true;
        else
            QMessageBox::critical(this, tr("Error"), tr("File '%1' already exists!").arg(file.fileName()));
    }
}

void AutoStartPage::editButton_clicked()
{
    QModelIndex index = ui->autoStartView->selectionModel()->currentIndex();
    XdgDesktopFile file = mXdgAutoStartModel->desktopFile(index);
    AutoStartEdit edit(file.name(),
                       file.value(QL1S("Exec")).toString(),
                       file.value(QL1S("X-LXQt-Need-Tray"), false).toBool(),
                       file.value(QL1S("X-LXQt-X11-Only"), false).toBool(),
                       this);
    bool success = false;
    while (!success && edit.exec() == QDialog::Accepted)
    {
        const auto trimmedName = edit.name().trimmed();
        const auto trimmedCommand = edit.command().trimmed();
        if (trimmedName.isEmpty() || trimmedCommand.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("Please provide Name and Command"));
            continue;
        }
        if (trimmedName.startsWith(QL1S(".")) || trimmedName.contains(QL1S("/")))
        {
            QMessageBox::critical(this, tr("Error"), tr("Name should not start with dot or contain slash"));
            continue;
        }
        file.setLocalizedValue(QL1S("Name"), trimmedName);
        file.setValue(QL1S("Exec"), trimmedCommand);
        if (edit.needTray())
            file.setValue(QL1S("X-LXQt-Need-Tray"), true);
        else
            file.removeEntry(QL1S("X-LXQt-Need-Tray"));
        if (edit.x11Only())
            file.setValue(QL1S("X-LXQt-X11-Only"), true);
        else
            file.removeEntry(QL1S("X-LXQt-X11-Only"));

        if (mXdgAutoStartModel->setEntry(index, file, true))
            success = true;
        else
            QMessageBox::critical(this, tr("Error"), tr("File '%1' already exists!").arg(file.fileName()));
    }
}

void AutoStartPage::deleteButton_clicked()
{
    QModelIndex index = ui->autoStartView->selectionModel()->currentIndex();
    mXdgAutoStartModel->removeRow(index.row(), index.parent());
}

void AutoStartPage::selectionChanged(QModelIndex curr)
{
    AutoStartItemModel::ActiveButtons active = mXdgAutoStartModel->activeButtons(curr);
    ui->addButton->setEnabled(active & AutoStartItemModel::AddButton);
    ui->editButton->setEnabled(active & AutoStartItemModel::EditButton);
    ui->deleteButton->setEnabled(active & AutoStartItemModel::DeleteButton);
}

void AutoStartPage::updateButtons()
{
    selectionChanged(ui->autoStartView->selectionModel()->currentIndex());
}
