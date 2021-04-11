/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2021~ LXQt team
 * Authors:
 *  Palo Kisa <palo.kisa@gmail.com>
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
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "procreaper.h"
#include "log.h"
#if defined(Q_OS_LINUX)
#include <sys/prctl.h>
#include <proc/readproc.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/procctl.h>
#include <libutil.h>
#include <sys/user.h>
#include <signal.h>
#endif
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>

ProcReaper::ProcReaper()
    : mShouldRun{true}
{
#if defined(Q_OS_LINUX)
    int result = prctl(PR_SET_CHILD_SUBREAPER, 1);
    if (result != 0)
        qCWarning(SESSION) << "Unable to to set PR_SET_CHILD_SUBREAPER, " << result << " - " << strerror(errno);
#elif defined(Q_OS_FREEBSD)
    int result = procctl(P_PID, ::getpid(), PROC_REAP_ACQUIRE, nullptr);
    if (result != 0)
        qCWarning(SESSION) << "Unable to to set PROC_REAP_ACQUIRE, " << result << " - " << strerror(errno);
#endif
}

ProcReaper::~ProcReaper()
{
    stop({});
}

void ProcReaper::run()
{
    pid_t pid = 0;
    while (true)
    {
        if (pid <= 0)
        {
            QMutexLocker guard{&mMutex};
            mWait.wait(&mMutex, 1000); // 1 second
        }

        int status;
        pid = ::waitpid(-1, &status, WNOHANG);
        if (pid < 0)
        {
            if (ECHILD != errno)
                qCDebug(SESSION) << "waitpid failed " << strerror(errno);
        } else if (pid > 0)
        {
            if (WIFEXITED(status))
                qCDebug(SESSION) << "Child process " << pid << " exited with status " << WEXITSTATUS(status);
            else if (WIFSIGNALED(status))
                qCDebug(SESSION) << "Child process " << pid << " terminated on signal " << WTERMSIG(status);
            else
                qCDebug(SESSION) << "Child process " << pid << " ended";
        }
        {
            QMutexLocker guard{&mMutex};
            if (!mShouldRun && pid <= 0)
                break;
        }
    }
}

void ProcReaper::stop(const std::set<int64_t> & excludedPids)
{
    {
        QMutexLocker guard{&mMutex};
        if (!mShouldRun)
            return;
    }
    // send term to all children
    const pid_t my_pid = ::getpid();
    std::vector<pid_t> children;
#if defined(Q_OS_LINUX)
    PROCTAB * proc_dir = ::openproc(PROC_FILLSTAT);
    while (proc_t * proc = ::readproc(proc_dir, nullptr))
    {
        if (proc->ppid == my_pid)
        {
            children.push_back(proc->tgid);
        }
        ::freeproc(proc);
    }
    ::closeproc(proc_dir);
#elif defined(Q_OS_FREEBSD)
    int cnt = 0;
    if (kinfo_proc *proc_info = kinfo_getallproc(&cnt))
    {
        for (int i = 0; i < cnt; ++i)
        {
            if (proc_info[i].ki_ppid == my_pid)
            {
                children.push_back(proc_info[i].ki_pid);
            }
        }
        free(proc_info);
    }
#endif
    for (auto const & child : children)
    {
        if (excludedPids.count(child) == 0)
        {
            qCDebug(SESSION) << "Seding TERM to child " << child;
            ::kill(child, SIGTERM);
        }
    }
    mWait.wakeAll();
    {
        QMutexLocker guard{&mMutex};
        mShouldRun = false;
    }
    QThread::wait(5000); // 5 seconds
}
