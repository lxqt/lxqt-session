/*
 * This file is part of the LXQt project. <http://lxqt.org>
 * Copyright (C) 2015 Luís Pereira <luis.artur.pereira@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "userlocationspage.h"

#include <XdgDirs>
#include <XdgIcon>

#include <QCoreApplication>
#include <QLabel>
#include <QSignalMapper>
#include <QLineEdit>
#include <QToolButton>
#include <QStringList>
#include <QGridLayout>
#include <QSpacerItem>
#include <QFileDialog>
#include <QMessageBox>


class UserLocationsPagePrivate {
public:

    UserLocationsPagePrivate();
    static const QStringList locationsName;

    QList<QString> initialLocations;
    QList<QLineEdit *> locations;
    QSignalMapper *signalMapper;

    void getUserDirs();
    void populate();
};

UserLocationsPagePrivate::UserLocationsPagePrivate()
    : locations(),
      signalMapper(0)
{
}

// This labels haveto match XdgDirs::UserDirectories
const QStringList UserLocationsPagePrivate::locationsName = QStringList() <<
    qApp->translate("UserLocationsPrivate", "Desktop") <<
    qApp->translate("UserLocationsPrivate", "Downloads") <<
    qApp->translate("UserLocationsPrivate", "Templates") <<
    qApp->translate("UserLocationsPrivate", "Public Share") <<
    qApp->translate("UserLocationsPrivate", "Documents") <<
    qApp->translate("UserLocationsPrivate", "Music") <<
    qApp->translate("UserLocationsPrivate", "Pictures") <<
    qApp->translate("UserLocationsPrivate", "Videos");

void UserLocationsPagePrivate::getUserDirs()
{
    const int N = locationsName.count();
    for(int i = 0; i < N; ++i) {
        const QString userDir = XdgDirs::userDir(static_cast<XdgDirs::UserDirectory> (i));
        const QDir dir(userDir);
        initialLocations.append(dir.canonicalPath());
    }
}

void UserLocationsPagePrivate::populate()
{
    const int N = initialLocations.count();

    Q_ASSERT(N == locationsName.count());

    for (int i = 0; i < N; ++i) {
        locations.at(i)->setText(initialLocations.at(i));
    }
}


UserLocationsPage::UserLocationsPage(QWidget *parent)
    : QWidget(parent),
      d(new UserLocationsPagePrivate())
{
    d->signalMapper = new QSignalMapper(this);
    QGridLayout *gridLayout = new QGridLayout(this);

    int row = 0;

    QLabel *description = new QLabel(tr("Locations for Personal Files"), this);
    QFont font;
    font.setBold(true);
    description->setFont(font);

    gridLayout->addWidget(description, row++, 0, 1, -1);

    for (int i = 0; i < d->locationsName.size(); ++i, ++row) {
        QLabel *label = new QLabel(d->locationsName.at(i), this);

        QLineEdit *edit = new QLineEdit(this);
        d->locations.append(edit);
        edit->setClearButtonEnabled(true);

        QToolButton *button = new QToolButton(this);
        button->setIcon(XdgIcon::fromTheme(QStringLiteral("folder")));
        connect(button, SIGNAL(clicked()), d->signalMapper, SLOT(map()));
        d->signalMapper->setMapping(button, i);

        gridLayout->addWidget(label, row, 0);
        gridLayout->addWidget(edit, row, 1);
        gridLayout->addWidget(button, row, 2);
    }
    connect(d->signalMapper, SIGNAL(mapped(int)),
            this, SLOT(clicked(int)));

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum,
                                                  QSizePolicy::Expanding);
    gridLayout->addItem(verticalSpacer, row++, 1, 1, 1);
    setLayout(gridLayout);

    d->getUserDirs();
    d->populate();
}

UserLocationsPage::~UserLocationsPage()
{
    // It's fine to delete a null pointer. No need to check.
    delete d;
    d = 0;
}

void UserLocationsPage::restoreSettings()
{
    d->populate();
}

void UserLocationsPage::save()
{
    bool restartWarn = false;

    const int N = d->locations.count();
    for (int i = 0; i < N; ++i) {
        const QDir dir(d->locations.at(i)->text());
        const QString s = dir.canonicalPath();

        if (s != d->initialLocations.at(i)) {
            const bool ok = XdgDirs::setUserDir(
                        static_cast<XdgDirs::UserDirectory> (i), s, true);
            if (!ok) {
                const int ret = QMessageBox::warning(this,
                    tr("LXQt Session Settings - Locations"),
                    tr("An error ocurred while applying the settings for the %1 location").arg(d->locationsName.at(i)),
                    QMessageBox::Ok);
                Q_UNUSED(ret);
            }
            restartWarn = true;
        }
    }

    if (restartWarn)
        emit needRestart();
}

void UserLocationsPage::clicked(int id)
{
    const QString& currentDir = d->locations.at(id)->text();
    const QString dir = QFileDialog::getExistingDirectory(this,
                    tr("Choose Location"),
                    currentDir,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        const QDir dd(dir);
        d->locations.at(id)->setText(dd.canonicalPath());
    }
}
