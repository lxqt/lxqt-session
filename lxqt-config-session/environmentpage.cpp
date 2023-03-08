/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2012 LXQt team
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

#include "environmentpage.h"
#include "ui_environmentpage.h"

#include <LXQt/Globals>
#include <QHeaderView>

EnvironmentPage::EnvironmentPage(LXQt::Settings *settings, QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    ui(new Ui::EnvironmentPage)
{
    ui->setupUi(this);

    // give enough space to the first section of the header
    ui->treeWidget->header()->resizeSection(0, ui->treeWidget->header()->sectionSizeHint(0));

    connect(ui->addButton,    &QPushButton::clicked,     this, &EnvironmentPage::addButton_clicked);
    connect(ui->deleteButton, &QPushButton::clicked,     this, &EnvironmentPage::deleteButton_clicked);
    connect(ui->treeWidget,   &QTreeWidget::itemChanged, this, &EnvironmentPage::itemChanged);

    /* restoreSettings() is called from SessionConfigWindow
       after connections with DefaultApps have been set up */
}

EnvironmentPage::~EnvironmentPage()
{
    delete ui;
}

void EnvironmentPage::restoreSettings()
{
    m_settings->beginGroup(QL1S("Environment"));
    QString value;
    ui->treeWidget->clear();
    const auto keys = m_settings->childKeys();
    for (const QString& i : keys)
    {
        value = m_settings->value(i).toString();
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList() << i << value);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->treeWidget->addTopLevelItem(item);
        emit envVarChanged(i, value);
    }

    if (m_settings->value(QL1S("BROWSER")).isNull())
        emit envVarChanged(QL1S("BROWSER"), QString());
    if (m_settings->value(QL1S("TERM")).isNull())
        emit envVarChanged(QL1S("TERM"), QString());

    m_settings->endGroup();
}

void EnvironmentPage::save()
{
    bool doRestart = false;
    QMap<QString, QString> oldSettings;

    m_settings->beginGroup(QL1S("Environment"));

    /* We erase the Environment group and then write the UI settings. To know if
       they changed or not we need to save them to memory.
     */
    const auto keys = m_settings->childKeys();
    for (const QString &key : keys)
        oldSettings[key] = m_settings->value(key, QString()).toString();
    m_settings->remove(QString());

    const int nItems = ui->treeWidget->topLevelItemCount();
    for(int i = 0; i < nItems; ++i)
    {
        QTreeWidgetItem *item = ui->treeWidget->topLevelItem(i);
        const QString key = item->text(0);
        const QString value = item->text(1);

        if (oldSettings.value(key) != value)
            doRestart = true;

        m_settings->setValue(item->text(0), item->text(1));
    }
    m_settings->endGroup();

    if (oldSettings.size() != nItems)
        doRestart = true;

    if (doRestart)
        emit needRestart();
}

void EnvironmentPage::addButton_clicked()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList() << QString() << QString());
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->setCurrentItem(item);
}

void EnvironmentPage::deleteButton_clicked()
{
    const auto items = ui->treeWidget->selectedItems();
    for (QTreeWidgetItem* item : items)
    {
        emit envVarChanged(item->text(0), QString());
        delete item;
    }
}

void EnvironmentPage::itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    emit envVarChanged(item->text(0), item->text(1));
}

void EnvironmentPage::updateItem(const QString& var, const QString& val)
{
    const QList<QTreeWidgetItem*> itemList = ui->treeWidget->findItems(var, Qt::MatchExactly);
    if (itemList.isEmpty())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList() << var << val);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->treeWidget->addTopLevelItem(item);
        return;
    }

    for (QTreeWidgetItem* item : itemList)
    {
        if (!val.isEmpty())
            item->setText(1, val);
        else
            delete item;
    }
}

void EnvironmentPage::updateScaleFactor()
{
    m_settings->beginGroup(QL1S("Environment"));
    updateItem(QL1S("QT_SCALE_FACTOR"), m_settings->value(QL1S("QT_SCALE_FACTOR"), 1).toString());
    updateItem(QL1S("GDK_SCALE"), m_settings->value(QL1S("GDK_SCALE"), 1).toString());
    m_settings->endGroup();
}

