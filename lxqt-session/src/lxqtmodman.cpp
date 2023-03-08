/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2010-2011 LXQt team
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
#include <LXQt/Globals>
#include <LXQt/Settings>
#include <XdgAutoStart>
#include <XdgDirs>
#include <unistd.h>

#include <QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDateTime>
#include <QPointer>
#include "wmselectdialog.h"
#include "windowmanager.h"
#include <wordexp.h>
#include "log.h"

#include <KWindowSystem/KWindowSystem>
#include <KWindowSystem/netwm.h>

#include <QX11Info>

#define MAX_CRASHES_PER_APP 5

using namespace LXQt;

/**
 * @brief the constructor, needs a valid modules.conf
 */
LXQtModuleManager::LXQtModuleManager(QObject* parent)
    : QObject(parent),
      mWmProcess(new QProcess(this)),
      mThemeWatcher(new QFileSystemWatcher(this))
{
    connect(mThemeWatcher, &QFileSystemWatcher::directoryChanged, this, &LXQtModuleManager::themeFolderChanged);
    connect(LXQt::Settings::globalSettings(), &LXQt::GlobalSettings::lxqtThemeChanged, this, &LXQtModuleManager::themeChanged);

    mProcReaper.start();
}

void LXQtModuleManager::setWindowManager(const QString & windowManager)
{
    mWindowManager = windowManager;
}

void LXQtModuleManager::startup(LXQt::Settings& s)
{
    // The lxqt-confupdate can update the settings of the WM, so run it first.
    startConfUpdate();

    // Start window manager
    if (QGuiApplication::platformName() == QStringLiteral("xcb"))
        startWm(&s);

    startAutostartApps();

    QStringList paths;
    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();

    for(const QString &path : qAsConst(paths))
    {
        QFileInfo fi(QString::fromLatin1("%1/lxqt/themes").arg(path));
        if (fi.exists())
            mThemeWatcher->addPath(fi.absoluteFilePath());
    }

    themeChanged();
}

void LXQtModuleManager::startAutostartApps()
{
    // XDG autostart
    const XdgDesktopFileList fileList = XdgAutoStart::desktopFileList();
    QList<const XdgDesktopFile*> trayApps;
    for (XdgDesktopFileList::const_iterator i = fileList.constBegin(); i != fileList.constEnd(); ++i)
    {
        if (i->value(QSL("X-LXQt-Need-Tray"), false).toBool())
            trayApps.append(&(*i));
        else
        {
            startProcess(*i);
            qCDebug(SESSION) << "start" << i->fileName();
        }
    }

    if (!trayApps.isEmpty())
    {
        QPointer<QTimer> t{new QTimer};
        auto starter = [this, fileList, trayApps, t] (const bool forceStart = false) {
            if (!t)
                return;

            if (QSystemTrayIcon::isSystemTrayAvailable())
                qCDebug(SESSION) << "System Tray started";
            else if (forceStart)
                qCWarning(SESSION) << "System Tray haven't stared yet! Starting tray apps anyway...";
            else
                return;

            QScopedPointer<QTimer> releaser{t};
            for (const XdgDesktopFile* const f : qAsConst(trayApps))
            {
                qCDebug(SESSION) << "start tray app" << f->fileName();
                startProcess(*f);
            }
            t->stop();
        };
        connect(t, &QTimer::timeout, this, starter);
        t->setSingleShot(false);
        t->start(1000);
        // try to start instantly, no need to wait the 1st sec
        starter();
        // start the apps anyway after a timeout
        QTimer::singleShot(15 * 1000, this, std::bind(starter, true));
    }
}

void LXQtModuleManager::themeFolderChanged(const QString& /*path*/)
{
    QString newTheme;
    if (!QFileInfo::exists(mCurrentThemePath))
    {
        const QList<LXQtTheme> &allThemes = lxqtTheme.allThemes();
        if (!allThemes.isEmpty())
            newTheme = allThemes[0].name();
        else
            return;
    }
    else
        newTheme = lxqtTheme.currentTheme().name();

    LXQt::Settings settings(QSL("lxqt"));
    if (newTheme == settings.value(QL1S("theme")))
    {
        // force the same theme to be updated
        settings.setValue(QSL("__theme_updated__"), QDateTime::currentMSecsSinceEpoch());
    }
    else
        settings.setValue(QL1S("theme"), newTheme);

    sync();
}

void LXQtModuleManager::themeChanged()
{
    if (!mCurrentThemePath.isEmpty())
        mThemeWatcher->removePath(mCurrentThemePath);

    if (lxqtTheme.currentTheme().isValid())
    {
        mCurrentThemePath = lxqtTheme.currentTheme().path();
        mThemeWatcher->addPath(mCurrentThemePath);
    }
}

void LXQtModuleManager::startWm(LXQt::Settings *settings)
{
    // if the WM is active do not run WM.
    // all window managers must set their name according to the spec
    if (!QString::fromUtf8(NETRootInfo(QX11Info::connection(), NET::SupportingWMCheck).wmName()).isEmpty())
    {
        return;
    }

    if (mWindowManager.isEmpty())
    {
        mWindowManager = settings->value(QL1S("window_manager")).toString();
    }

    // If previuos WM was removed, we show dialog.
    if (mWindowManager.isEmpty() || ! findProgram(mWindowManager.split(QL1C(' '))[0]))
    {
        mWindowManager = showWmSelectDialog();
        settings->setValue(QL1S("window_manager"), mWindowManager);
        settings->sync();
    }

    mWmProcess->start(mWindowManager, QStringList());

    // other autostart apps will be handled after the WM becomes available

    // Wait until the WM loads
    QEventLoop waitLoop;
    auto checker = [&waitLoop] {
        // all window managers must set their name according to the spec
        if (!QString::fromUtf8(NETRootInfo(QX11Info::connection(), NET::SupportingWMCheck).wmName()).isEmpty())
        {
            qCDebug(SESSION) << "Window Manager started";
            waitLoop.exit();
        }
    };
    QTimer t;
    connect(&t, &QTimer::timeout, this, checker);
    t.setSingleShot(false);
    t.start(500);
    // add a timeout to avoid infinite blocking if a WM fail to execute.
    QTimer::singleShot(30 * 1000, &waitLoop, &QEventLoop::quit);
    // Note: the timer object is destructed sooner than the wait loop upon finishing this method
    waitLoop.exec();
    // FIXME: blocking is a bad idea. We need to start as many apps as possible and
    //         only wait for the start of WM when it's absolutely needed.
    //         Maybe we can add a X-Wait-WM=true key in the desktop entry file?
}

void LXQtModuleManager::startProcess(const XdgDesktopFile& file)
{
    if (!file.value(QL1S("X-LXQt-Module"), false).toBool())
    {
        file.startDetached();
        return;
    }
    QStringList args = file.expandExecString();
    if (args.isEmpty())
    {
        qCWarning(SESSION) << "Wrong desktop file" << file.fileName();
        return;
    }
    LXQtModule* proc = new LXQtModule(file, this);
    connect(proc, &LXQtModule::moduleStateChanged, this, &LXQtModuleManager::moduleStateChanged);
    proc->start();

    QString name = QFileInfo(file.fileName()).fileName();
    mNameMap[name] = proc;

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LXQtModuleManager::restartModules);
}

void LXQtModuleManager::startProcess(const QString& name)
{
    if (!mNameMap.contains(name))
    {
        const auto files = XdgAutoStart::desktopFileList(false);
        for (const XdgDesktopFile& file : files)
        {
            if (QFileInfo(file.fileName()).fileName() == name)
            {
                startProcess(file);
                return;
            }
        }
    }
}

void LXQtModuleManager::stopProcess(const QString& name)
{
    if (mNameMap.contains(name))
        mNameMap[name]->terminate();
}

QStringList LXQtModuleManager::listModules() const
{
    return QStringList(mNameMap.keys());
}

void LXQtModuleManager::startConfUpdate()
{
    XdgDesktopFile desktop(XdgDesktopFile::ApplicationType, QSL(":lxqt-confupdate"), QSL("lxqt-confupdate --watch"));
    desktop.setValue(QSL("Name"), QSL("LXQt config updater"));
    desktop.setValue(QL1S("X-LXQt-Module"), true);
    startProcess(desktop);
}

void LXQtModuleManager::restartModules(int exitCode, QProcess::ExitStatus exitStatus)
{
    LXQtModule* proc = qobject_cast<LXQtModule*>(sender());
    if (nullptr == proc) {
        qCWarning(SESSION) << "Got an invalid (null) module to restart. Ignoring it";
        return;
    }

    if (!proc->isTerminating())
    {
        QString procName = proc->file.name();
        switch (exitStatus)
        {
            case QProcess::NormalExit:
                qCDebug(SESSION) << "Process" << procName << "(" << proc << ") exited with code" << exitCode;
                if (exitCode == 0)
                    break;
                // Falls through.
            case QProcess::CrashExit:
            {
                qCDebug(SESSION) << "Process" << procName << "(" << proc << ") has to be restarted";
                time_t now = time(nullptr);
                mCrashReport[proc].prepend(now);
                while (now - mCrashReport[proc].back() > 60)
                    mCrashReport[proc].pop_back();
                if (mCrashReport[proc].length() >= MAX_CRASHES_PER_APP)
                {
                    QMessageBox::warning(nullptr, tr("Crash Report"),
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


LXQtModuleManager::~LXQtModuleManager()
{
    // We disconnect the finished signal before deleting the process. We do
    // this to prevent a crash that results from a state change signal being
    // emitted while deleting a crashing module.
    // If the module is still connect restartModules will be called with a
    // invalid sender.

    ModulesMapIterator i(mNameMap);
    while (i.hasNext())
    {
        i.next();

        auto p = i.value();
        disconnect(p);

        delete p;
        mNameMap[i.key()] = nullptr;
    }

    delete mWmProcess;
}

/**
* @brief this logs us out by terminating our session
**/
void LXQtModuleManager::logout(bool doExit)
{
    // modules
    ModulesMapIterator i(mNameMap);
    while (i.hasNext())
    {
        i.next();
        qCDebug(SESSION) << "Module logout" << i.key();
        LXQtModule* p = i.value();
        p->terminate();
    }
    i.toFront();
    while (i.hasNext())
    {
        i.next();
        LXQtModule* p = i.value();
        if (p->state() != QProcess::NotRunning && !p->waitForFinished(2000))
        {
            qCWarning(SESSION, "Module %s won't terminate ... killing.", qPrintable(i.key()));
            p->kill();
        }
    }

    // terminate all possible children except WM
    mProcReaper.stop({mWmProcess->processId()});

    mWmProcess->terminate();
    if (mWmProcess->state() != QProcess::NotRunning && !mWmProcess->waitForFinished(2000))
    {
        qCWarning(SESSION) << "Window Manager won't terminate ... killing.";
        mWmProcess->kill();
    }

    if (doExit)
        QCoreApplication::exit(0);
}

QString LXQtModuleManager::showWmSelectDialog()
{
    WindowManagerList availableWM = getWindowManagerList(true);
    if (availableWM.count() == 1)
        return availableWM.at(0).command;

    WmSelectDialog dlg(availableWM);
    dlg.exec();
    return dlg.windowManager();
}

void LXQtModuleManager::resetCrashReport()
{
    mCrashReport.clear();
}

void lxqt_setenv(const char *env, const QByteArray &value)
{
    wordexp_t p;
    wordexp(value.constData(), &p, 0);
    if (p.we_wordc == 1)
    {

        qCDebug(SESSION) << "Environment variable" << env << "=" << p.we_wordv[0];
        qputenv(env, p.we_wordv[0]);
    }
    else
    {
        qCWarning(SESSION) << "Error expanding environment variable" << env << "=" << value;
        qputenv(env, value);
    }
     wordfree(&p);
}

void lxqt_setenv_prepend(const char *env, const QByteArray &value, const QByteArray &separator)
{
    QByteArray orig(qgetenv(env));
    orig = orig.prepend(separator);
    orig = orig.prepend(value);
    qCDebug(SESSION) << "Setting special" << env << " variable:" << orig;
    lxqt_setenv(env, orig);
}

LXQtModule::LXQtModule(const XdgDesktopFile& file, QObject* parent) :
    QProcess(parent),
    file(file),
    fileName(QFileInfo(file.fileName()).fileName()),
    mIsTerminating(false)
{
    QProcess::setProcessChannelMode(QProcess::ForwardedChannels);
    connect(this, &LXQtModule::stateChanged, this, &LXQtModule::updateState);
}

void LXQtModule::start()
{
    mIsTerminating = false;
    QStringList args = file.expandExecString();
    QString command = args.takeFirst();
    QProcess::start(command, args);
}

void LXQtModule::terminate()
{
    mIsTerminating = true;
    QProcess::terminate();
}

bool LXQtModule::isTerminating()
{
    return mIsTerminating;
}

void LXQtModule::updateState(QProcess::ProcessState newState)
{
    if (newState != QProcess::Starting)
        emit moduleStateChanged(fileName, (newState == QProcess::Running));
}
