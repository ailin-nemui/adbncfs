/*
 * $Id$
 *
 * File:   spawn.h
 * Author: Werner Jaeger
 *
 * Created on November 19, 2015, 5:31 PM
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

#ifndef SPAWN_H
#define SPAWN_H

#include <ext/stdio_filebuf.h> // NB: Specific to libstdc++
#include <iostream>
#include <fstream>

/** Wrapping pipe in a class makes sure they are closed when we leave scope. */
class Cpipe
{
public:

   Cpipe();
   virtual ~Cpipe();

   const inline int readFd() const;
   const inline int writeFd() const;
   void close();

private:
   int m_aiFd[2];
};

//
// Usage:
//   spawn s(argv)
//   s.stdin << ...
//   s.stdout >> ...
//   s.send_eol()
//   s.wait()
//
/**
 * Spawn a child process.
 *
 *  Usage:
 *    spawn s(argv);
 *    s.outStream() << ...
 *    s.inStream() >> ...
 *    s.sendEof();
 *    s.wait();
 */
class Spawn
{
public:
    Spawn(const char* const argv[], bool fUseStdErr = false, bool fWithPath = false, const char* const envp[] = NULL);
    virtual ~Spawn();

    /**
     * Access the child process stdout as in stream to read from.
     *
     * @return a istream to read the output of the child process.
     */
    std::istream& inStream() { return(stdout); }

    /**
     * Access the child process stdin as out stream to write to the process.
     *
     * @return a ostream to write to the stdin of the child process.
     */
    std::ostream& outStream() { return(stdin); }
    void sendEof();
    int wait();

private:
   /** Prevent copy-construction */
   Spawn(const Spawn& orig);

   /** Prevent assignment */
   Spawn& operator=(const Spawn& orig);

    int m_iChildPid;
    __gnu_cxx::stdio_filebuf<char> *m_pWriteBuf;
    __gnu_cxx::stdio_filebuf<char> *m_pReadBuf;
    std::ostream stdin;
    std::istream stdout;
    Cpipe m_WritePipe;
    Cpipe m_ReadPipe;
};

#endif /* SPAWN_H */

