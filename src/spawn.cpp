/*
 * $Id$
 *
 * File:   spawn.cpp
 * Author: Werner Jaeger
 *
 * Created on November 19, 2015, 5:33 PM
 *
 * Copyright 2015 Werner Jaeger.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPAWN_CPP
#define SPAWN_CPP

#include "spawn.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdexcept>

using namespace std;

Cpipe::Cpipe()
{
  if (::pipe(m_aiFd))
  {
      string strErr("Failed to create pipe. Errno: ");
      strErr += to_string(errno);
      throw runtime_error(strErr);
  }
}

const inline int Cpipe::readFd() const
{
  return m_aiFd[0];
}

const inline int Cpipe::writeFd() const
{
  return m_aiFd[1];
}

void Cpipe::close()
{
  ::close(m_aiFd[0]);
  ::close(m_aiFd[1]);
}

Cpipe::~Cpipe()
{
  close();
}

/**
 * Spawns a child process.
 *
 * @param argv an array of pointers to null-terminated strings that represent
 *        the argument list available to the new program. The first argument,
 *        by convention, must point to the filename associated with the file
 *        being executed. The array of pointers must be terminated by a NULL
 *        pointer.
 * @param fUseStdErr if true stderr is redirected instead of stdout.
 * @param fWithPath if true and argv[0] does not contain a slash (/) executable
 *        is sought in the PATH environment variable, otherwise PATH is
 *        ignored, and the file at the specified pathname is executed.
 * @param envp is an array of strings, conventionally of the form key=value,
 *        which are passed as environment to the new program. must be
 *        terminated by a NULL pointer.
 *
 * @throws runtime_error if child process could not be started of if
 *         no pipe could be created.
 */
Spawn::Spawn(const char* const argv[], bool fUseStdErr, bool fWithPath, const char* const envp[]): m_iChildPid(-1), m_pWriteBuf(NULL), m_pReadBuf(NULL), stdin(NULL), stdout(NULL), m_WritePipe(), m_ReadPipe()
{
    m_iChildPid = ::fork();
    if (m_iChildPid == -1)
        throw runtime_error("Failed to start child process");

    if (m_iChildPid == 0)
    {
        // In child process
        ::dup2(m_WritePipe.readFd(), STDIN_FILENO);

        if (fUseStdErr)
            ::dup2(m_ReadPipe.writeFd(), STDERR_FILENO);
        else
            ::dup2(m_ReadPipe.writeFd(), STDOUT_FILENO);

        m_WritePipe.close();
        m_ReadPipe.close();

        int iRes;
        if (fWithPath)
        {
            if (envp != 0)
                iRes = ::execvpe(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            else
                iRes = ::execvp(argv[0], const_cast<char* const*>(argv));
        }
        else
        {
            if (envp != 0)
                iRes = ::execve(argv[0], const_cast<char* const*>(argv), const_cast<char* const*>(envp));
            else
                iRes = ::execv(argv[0], const_cast<char* const*>(argv));
        }

        if (iRes == -1)
        {
            if (fUseStdErr)
                cout << "Error: Failed to launch program \"" << argv[0] << "\" Error: " << to_string(errno) << endl;
            else
                cerr << "Error: Failed to launch program \"" << argv[0] << "\" Error: " << to_string(errno) << endl;

            ::exit(errno);
        }
    }
    else
    {
        ::close(m_WritePipe.readFd());
        ::close(m_ReadPipe.writeFd());
        m_pWriteBuf = new __gnu_cxx::stdio_filebuf<char>(m_WritePipe.writeFd(), ios::out);
        m_pReadBuf = new __gnu_cxx::stdio_filebuf<char>(m_ReadPipe.readFd(), ios::in);
        stdin.rdbuf(m_pWriteBuf);
        stdout.rdbuf(m_pReadBuf);
    }
}

Spawn::~Spawn()
{
    if (m_pWriteBuf)
        delete m_pWriteBuf;

    if (m_pReadBuf)
        delete m_pReadBuf;
}

void Spawn::sendEof()
{
    m_pWriteBuf->close();
    m_pReadBuf->close();
}

/**
 * Wait until child process has terminated.
 *
 * The returned status can be inspected with the following macros:
 *     - WIFEXITED(status)
 *     - WEXITSTATUS(status)
 *     - WIFSIGNALED(status)
 *     - WTERMSIG(status)
 *     - WCOREDUMP(status)
 *
 * @return the status of the terminated child process or EXIT_FAILURE if an
 *         error occurred.
 *
 * @see wait(2) - Linux manual page - man7.org
 */
int Spawn::wait()
{
    int iStatus;

    do
    {
        pid_t iTerminatedPid(::waitpid(m_iChildPid, &iStatus, WUNTRACED | WCONTINUED));
        if (iTerminatedPid == -1)
        {
            iStatus = EXIT_FAILURE;
            break;
        }
    } while (!WIFEXITED(iStatus) && !WIFSIGNALED(iStatus));

    return(iStatus);
}

#endif /* SPAWN_CPP */
