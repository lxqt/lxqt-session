/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LxQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org, http://lxde.org/
 *
 * Copyright: 2010-2011 LxQt team
 * Authors:
 *   Petr Vanek <petr@scribus.info>
 *   Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include "lxqtmodman.h"
#include <LXQt/Settings>
#include <XdgAutoStart>
#include <XdgDirs>
#include <unistd.h>

#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QDateTime>
#include "wmselectdialog.h"
#include "windowmanager.h"
#include <wordexp.h>

#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/netwm.h>

#include <QX11Info>

#define MAX_CRASHES_PER_APP 5

using namespace LxQt;

/**
 * @brief the constructor, needs a valid modules.conf
 */
LxQtModuleManager::LxQtModuleManager(const QString & windowManager, QObject* parent)
    : QObject(parent),
      mWindowManager(windowManager),
      mWmProcess(new QProcess(this)),
      mThemeWatcher(new QFileSystemWatcher(this)),
      mWmStarted(false),
      mTrayStarted(false),
      mWaitLoop(NULL)
{
    connect(mThemeWatcher, SIGNAL(directoryChanged(QString)), SLOT(themeFolderChanged(QString)));
    connect(LxQt::Settings::globalSettings(), SIGNAL(lxqtThemeChanged()), SLOT(themeChanged()));

    qApp->installNativeEventFilter(this);
}

void LxQtModuleManager::startup(LxQt::Settings& s)
{
    // The lxqt-confupdate can update the settings of the WM, so run it first.
    startConfUpdate();

    // Start window manager
    startWm(&s);

    startAutostartApps();

    QStringList paths;
    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();

    foreach(QString path, paths)
    {
        QFileInfo fi(QString("%1/lxqt/themes").arg(path));
        if (fi.exists())
            mThemeWatcher->addPath(fi.absoluteFilePath());
    }

    themeChanged();
}

void LxQtModuleManager::startAutostartApps()
{
    // XDG autostart
    XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    QList<XdgDesktopFile*> trayApps;
    for (XdgDesktopFileList::iterator i = fileList.begin(); i != fileList.end(); ++i)
    {
        if (i->value("X-LXQt-Need-Tray", false).toBool())
            trayApps.append(&(*i));
        else
        {
            startProcess(*i);
            qDebug() << "start" << i->fileName();
        }
    }

    if (!trayApps.isEmpty())
    {
        mTrayStarted = QSystemTrayIcon::isSystemTrayAvailable();
        if(!mTrayStarted)
        {
            QEventLoop waitLoop;
            mWaitLoop = &waitLoop;
            // add a timeout to avoid infinite blocking if a WM fail to execute.
            QTimer::singleShot(60 * 1000, &waitLoop, SLOT(quit()));
            waitLoop.exec();
            mWaitLoop = NULL;
        }
        foreach (XdgDesktopFile* f, trayApps)
        {
            qDebug() << "start tray app" << f->fileName();
            startProcess(*f);
        }
    }
}

void LxQtModuleManager::themeFolderChanged(const QString& /*path*/)
{
    QString newTheme;
    if (!QFileInfo(mCurrentThemePath).exists())
    {
        const QList<LxQtTheme> &allThemes = lxqtTheme.allThemes();
        if (!allThemes.isEmpty())
            newTheme = allThemes[0].name();
        else
            return;
    }
    else
        newTheme = lxqtTheme.currentTheme().name();

    LxQt::Settings settings("lxqt");
    if (newTheme == settings.value("theme"))
    {
        // force the same theme to be updated
        settings.setValue("__theme_updated__", QDateTime::currentMSecsSinceEpoch());
    }
    else
        settings.setValue("theme", newTheme);

    sync();
}

void LxQtModuleManager::themeChanged()
{
    if (!mCurrentThemePath.isEmpty())
        mThemeWatcher->removePath(mCurrentThemePath);

    if (lxqtTheme.currentTheme().isValid())
    {
        mCurrentThemePath = lxqtTheme.currentTheme().path();
        mThemeWatcher->addPath(mCurrentThemePath);
    }
}

void LxQtModuleManager::startWm(LxQt::Settings *settings)
{
    // If the WM is active do not run WM.
    NETRootInfo info(QX11Info::connection(), NET::Supported);
    if (info.isSupported(NET::SupportingWMCheck))
        return;

    if (mWindowManager.isEmpty())
    {
        mWindowManager = settings->value("window_manager").toString();
    }

    // If previuos WM was removed, we show dialog.
    if (mWindowManager.isEmpty() || ! findProgram(mWindowManager.split(' ')[0]))
    {
        mWindowManager = showWmSelectDialog();
        settings->setValue("window_manager", mWindowManager);
        settings->sync();
    }

    mWmProcess->start(mWindowManager);
    // other autostart apps will be handled after the WM becomes available

    // Wait until the WM loads
    QEventLoop waitLoop;
    mWaitLoop = &waitLoop;
    // add a timeout to avoid infinite blocking if a WM fail to execute.
    QTimer::singleShot(30 * 1000, &waitLoop, SLOT(quit()));
    waitLoop.exec();
    mWaitLoop = NULL;
    // FIXME: blocking is a bad idea. We need to start as many apps as possible and
    //         only wait for the start of WM when it's absolutely needed.
    //         Maybe we can add a X-Wait-WM=true key in the desktop entry file?
}

void LxQtModuleManager::startProcess(const XdgDesktopFile& file)
{
    if (!file.value("X-LXQt-Module", false).toBool())
    {
        file.startDetached();
        return;
    }
    QStringList args = file.expandExecString();
    if (args.isEmpty())
    {
        qWarning() << "Wrong desktop file" << file.fileName();
        return;
    }
    LxQtModule* proc = new LxQtModule(file, this);
    connect(proc, SIGNAL(moduleStateChanged(QString,bool)), this, SIGNAL(moduleStateChanged(QString,bool)));
    proc->start();

    QString name = QFileInfo(file.fileName()).fileName();
    mNameMap[name] = proc;

    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(restartModules(int, QProcess::ExitStatus)));
}

void LxQtModuleManager::startProcess(const QString& name)
{
    if (!mNameMap.contains(name))
    {
        foreach (const XdgDesktopFile& file, XdgAutoStart::desktopFileList(false))
        {
            if (QFileInfo(file.fileName()).fileName() == name)
            {
                startProcess(file);
                return;
            }
        }
    }
}

void LxQtModuleManager::stopProcess(const QString& name)
{
    if (mNameMap.contains(name))
        mNameMap[name]->terminate();
}

QStringList LxQtModuleManager::listModules() const
{
    return QStringList(mNameMap.keys());
}

void LxQtModuleManager::startConfUpdate()
{
    XdgDesktopFile desktop(XdgDesktopFile::ApplicationType, ":lxqt-confupdate", "lxqt-confupdate --watch");
    desktop.setValue("Name", "LXQt config updater");
    desktop.setValue("X-LXQt-Module", true);
    startProcess(desktop);
}

void LxQtModuleManager::restartModules(int exitCode, QProcess::ExitStatus exitStatus)
{
    LxQtModule* proc = qobject_cast<LxQtModule*>(sender());
    Q_ASSERT(proc);

    if (!proc->isTerminating())
    {
        QString procName = proc->file.name();
        switch (exitStatus)
        {
            case QProcess::NormalExit:
                qDebug() << "Process" << procName << "(" << proc << ") exited correctly.";
                break;
            case QProcess::CrashExit:
            {
                qDebug() << "Process" << procName << "(" << proc << ") has to be restarted";
                time_t now = time(NULL);
                mCrashReport[proc].prepend(now);
                while (now - mCrashReport[proc].back() > 60)
                    mCrashReport[proc].pop_back();
                if (mCrashReport[proc].length() >= MAX_CRASHES_PER_APP)
                {
                    QMessageBox::warning(0, tr("Crash Report"),
                                        tr("<b>%1</b> crashed too many times. Its autorestart has been disabled until next login.").arg(procName));
                }
                else
                {
                    proc->start();
                    return;
                }
                break;
            }
        }
    }
    mNameMap.remove(proc->fileName);
    proc->deleteLater();
}


LxQtModuleManager::~LxQtModuleManager()
{
    qApp->removeNativeEventFilter(this);
    qDeleteAll(mNameMap);
    delete mWmProcess;
}

/**
* @brief this logs us out by terminating our session
**/
void LxQtModuleManager::logout()
{
    // modules
    ModulesMapIterator i(mNameMap);
    while (i.hasNext())
    {
        i.next();
        qDebug() << "Module logout" << i.key();
        LxQtModule* p = i.value();
        p->terminate();
    }
    i.toFront();
    while (i.hasNext())
    {
        i.next();
        LxQtModule* p = i.value();
        if (p->state() != QProcess::NotRunning && !p->waitForFinished())
        {
            qWarning() << QString("Module '%1' won't terminate ... killing.").arg(i.key());
            p->kill();
        }
    }

    mWmProcess->terminate();
    if (mWmProcess->state() != QProcess::NotRunning && !mWmProcess->waitForFinished())
    {
        qWarning() << QString("Window Manager won't terminate ... killing.");
        mWmProcess->kill();
    }

    QCoreApplication::exit(0);
}

QString LxQtModuleManager::showWmSelectDialog()
{
    WindowManagerList availableWM = getWindowManagerList(true);
    if (availableWM.count() == 1)
        return availableWM.at(0).command;

    WmSelectDialog dlg(availableWM);
    dlg.exec();
    return dlg.windowManager();
}

void LxQtModuleManager::resetCrashReport()
{
    mCrashReport.clear();
}

bool LxQtModuleManager::nativeEventFilter(const QByteArray & eventType, void * message, long * result)
{
    if (eventType != "xcb_generic_event_t") // We only want to handle XCB events
        return false;

    if(!mWmStarted && mWaitLoop)
    {
        NETRootInfo info(QX11Info::connection(), NET::Supported);
        NET::Properties prop = info.event(reinterpret_cast<xcb_generic_event_t*>(message));

        if (prop & NET::SupportingWMCheck)
        {
            qDebug() << "Window Manager started";
            mWmStarted = true;
            if (mWaitLoop->isRunning())
                mWaitLoop->exit();
        }
    }

    if (!mTrayStarted && QSystemTrayIcon::isSystemTrayAvailable() && mWaitLoop)
    {
        qDebug() << "System Tray started";
        mTrayStarted = true;
        if (mWaitLoop->isRunning())
            mWaitLoop->exit();
    }

    return false;
}

void lxqt_setenv(const char *env, const QByteArray &value)
{
    wordexp_t p;
    wordexp(value, &p, 0);
    if (p.we_wordc == 1)
    {

        qDebug() << "Environment variable" << env << "=" << p.we_wordv[0];
        qputenv(env, p.we_wordv[0]);
    }
    else
    {
        qWarning() << "Error expanding environment variable" << env << "=" << value;
        qputenv(env, value);
    }
     wordfree(&p);
}

void lxqt_setenv_prepend(const char *env, const QByteArray &value, const QByteArray &separator)
{
    QByteArray orig(qgetenv(env));
    orig = orig.prepend(separator);
    orig = orig.prepend(value);
    qDebug() << "Setting special" << env << " variable:" << orig;
    lxqt_setenv(env, orig);
}

LxQtModule::LxQtModule(const XdgDesktopFile& file, QObject* parent) :
    QProcess(parent),
    file(file),
    fileName(QFileInfo(file.fileName()).fileName()),
    mIsTerminating(false)
{
    connect(this, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(updateState(QProcess::ProcessState)));
}

void LxQtModule::start()
{
    mIsTerminating = false;
    QStringList args = file.expandExecString();
    QString command = args.takeFirst();
    QProcess::start(command, args);
}

void LxQtModule::terminate()
{
    mIsTerminating = true;
    QProcess::terminate();
}

bool LxQtModule::isTerminating()
{
    return mIsTerminating;
}

void LxQtModule::updateState(QProcess::ProcessState newState)
{
    if (newState != QProcess::Starting)
        emit moduleStateChanged(fileName, (newState == QProcess::Running));
}
