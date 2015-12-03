/*
 * $Id$
 *
 * File:   fileinfCache.cpp
 * Author: Werner Jaeger
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
#include "fileInfoCache.h"
#include <errno.h>

/** Keep cache entries valid for 120 seconds */
static const int iFileDataCacheSecondsValid(120);

int adbncPush(const string& strLocalSource, const string& strRemoteDestination);
int adbncShell(const string& strCommand);

/**
 * Copy constructor.
 */
FileCache::Entry::Entry(const Entry& orig) : m_Timestamp(orig.m_Timestamp), m_pStatOutput(NULL), m_pReadLinkOutput(NULL)
{
    if (orig.m_pStatOutput)
        m_pStatOutput = new deque<string>(*orig.m_pStatOutput);

    if (orig.m_pReadLinkOutput)
        m_pReadLinkOutput = new deque<string>(*orig.m_pReadLinkOutput);
}

/**
 * Assignment operator.
 *
 * @param orig reference to right hand object.
 *
 * @return a reference to the assignee.
 */
FileCache::Entry& FileCache::Entry::operator=(const Entry& orig)
{
    if(this != &orig) // protect against invalid self-assignment
    {
        if (m_pStatOutput)
            delete m_pStatOutput;

        if (m_pReadLinkOutput)
            delete m_pReadLinkOutput;

        m_pStatOutput = NULL;
        m_pReadLinkOutput = NULL;

        if (orig.m_pStatOutput)
            m_pStatOutput = new deque<string>(*orig.m_pStatOutput);


        if (orig.m_pReadLinkOutput)
            m_pReadLinkOutput = new deque<string>(*orig.m_pReadLinkOutput);

        m_Timestamp = orig.m_Timestamp;
    }

    return(*this);
}

/**
 * Virtual destructor.
 */
FileCache::Entry::~Entry()
{
    if (m_pStatOutput)
        delete m_pStatOutput;

    if (m_pReadLinkOutput)
        delete m_pReadLinkOutput;
}

void FileCache::Entry::statOutput(const deque<string>& output)
{
    if (m_pStatOutput)
        delete m_pStatOutput;

  m_pStatOutput = new deque<string>(output);
}

void FileCache::Entry::readLinkOutput(const deque<string>& output)
{
    if (m_pReadLinkOutput)
        delete m_pReadLinkOutput;

  m_pReadLinkOutput = new deque<string>(output);
}

/**
 * Caches the output of doStat().
 *
 * @param pcPath the pathname of the file thats data to cache.
 *
 * @param statOutput a reference to the data to cache.
 */
void FileCache::putStat(const char *pcPath, const deque<string>& statOutput)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));
    if (it == m_Entries.end())
    {
        Entry entry;
        entry.statOutput(statOutput);
        m_Entries[pcPath] = entry;
    }
    else
    {
        it->second.statOutput(statOutput);
        it->second.timeStamp(::time(NULL));
    }
}

/**
 * Caches the output of adbnc_readlink().
 *
 * @param pcPath the pathname of the file thats resolved link to cache.
 *
 * @param readLinkOutput a reference to resolved link name.
 */
void FileCache::putReadLink(const char *pcPath, const deque<string>& readLinkOutput)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));
    if (it == m_Entries.end())
    {
        Entry entry;
        entry.readLinkOutput(readLinkOutput);
        m_Entries[pcPath] = entry;
    }
    else
    {
        it->second.readLinkOutput(readLinkOutput);
        it->second.timeStamp(::time(NULL));
    }
}

const deque<string>* FileCache::getStat(const char *pcPath) const
{
    const deque<string>* pOut(NULL);

    const map<string, Entry>::const_iterator it(m_Entries.find(pcPath));
    if (it != m_Entries.end())
    {
        if (isValid(it->second))
            pOut = it->second.statOutput();
    }

    return(pOut);
}

const deque<string>* FileCache::getReadLink(const char *pcPath) const
{
    const deque<string>* pOut(NULL);

    const map<string, Entry>::const_iterator it(m_Entries.find(pcPath));
    if (it != m_Entries.end())
    {
        if (isValid(it->second))
            pOut = it->second.readLinkOutput();
    }

    return(pOut);
}

/**
 * Renders the cashed data for the given file as invalid.
 *
 * @param pcPath the pathname to the file.
 */
void FileCache::invalidate(const char *pcPath)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));
    if (it != m_Entries.end())
        m_Entries.erase(it);
}

/**
 * Tests if the given cache entry is valid.
 *
 * @param entry the entry to test.
 */
bool FileCache::isValid(const Entry& entry) const
{
    const time_t current = ::time(NULL);
    return(entry.timeStamp() + iFileDataCacheSecondsValid > current);
}

 int FileStatus::Entry::release(const int iFh)
 {
     int iRes(-EBADF);

     if (iFh != -1)
     {
         if (::close(iFh) == -1)
             iRes = -errno;
     }

     m_fPendingOpen = false;
     m_fForWrite = false;

     return(iRes);
 }

int FileStatus::Entry::flush(const string& strFromLocalPath, const string& strToPath)
{
    int iRes(0);

    if (pendingOpen())
    {
        if (!m_strRenamedFromLocal.empty())
        {
            if (m_fForWrite)
                iRes = adbncPush(m_strRenamedFromLocal, strToPath);

            m_strRenamedFromLocal.clear();
        }
        else
        {
            if (m_fForWrite)
                iRes = adbncPush(strFromLocalPath, strToPath);
        }

        pendingOpen(false, false);

//        if (!iRes)
//            adbncShell(string("sync"));
    }

    return(iRes);
}

/**
 * Notes that on the given path a read or write operation has been performed.
 *
 * adbnc_read(() or adbnc_write() call it with fPendingOpen == true to mark the
 * file for being read from or written to.
 *
 * adbnc_flush() and adbnc_fsync() call it with fPendingOpen == false to
 * indicate that the given file is no longer open for read respectively write
 * operations.
 *
 * To avoid unnecessary adb push operations is is important to
 * know that a file has never been changed (ie no write took place) since the
 * last  adb pull operation. flush() uses this information to
 * decide if a file needs to be pushed back to the android device.
 *
 * @param pcPath pathname to the file.
 * @param fPendingOpen if true indicate a read/write operation has been
 *         performed on the file.
 * @param fForWrite if true a read operation has been performed on the file,
 *        otherwise it was a write operation.
 *
 */
void FileStatus::pendingOpen(const char *pcPath, const bool fPendingOpen, const bool fForWrite)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));

    if (it == m_Entries.end())
    {
        Entry entry;
        entry.pendingOpen(fPendingOpen, fForWrite);
        m_Entries.insert(make_pair(pcPath, entry));
    }
    else
        it->second.pendingOpen(fPendingOpen, fForWrite);
}

/**
 * Transfer existing pending open information to the given path.
 *
 * @param pcPath path to the new name of the file.
 * @param strRenamedFromLocal path to the original name of the file.
 */
void FileStatus::pendingOpen(const char *pcPath, const string& strRenamedFromLocal)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));

    if (it == m_Entries.end())
    {
        Entry entry;
        entry.pendingOpen(strRenamedFromLocal);
        m_Entries.insert(make_pair(pcPath, entry));
    }
    else
        it->second.pendingOpen(strRenamedFromLocal);
}

void FileStatus::truncated(const char *pcPath, const bool fTruncated)
{
    const map<string, Entry>::iterator it(m_Entries.find(pcPath));

    if (it == m_Entries.end())
    {
        Entry entry;
        entry.truncated(fTruncated);
        m_Entries.insert(make_pair(pcPath, entry));
    }
    else
        it->second.truncated(fTruncated);
}

bool FileStatus::pendingOpen(const char *pcPath) const
{
    bool fRet(false);

    const map<string, Entry>::const_iterator it(m_Entries.find(pcPath));

    if (it != m_Entries.end())
        fRet = it->second.pendingOpen();

    return(fRet);
}

bool FileStatus::truncated(const char *pcPath) const
{
    bool fRet(false);

    const map<string, Entry>::const_iterator it(m_Entries.find(pcPath));

    if (it != m_Entries.end())
        fRet = it->second.truncated();

    return(fRet);
}

int FileStatus::flush(const char *pcPath, const string& strFromLocalPath)
{
    int iRes(0);

    const map<string, Entry>::iterator it(m_Entries.find(pcPath));

    if (it != m_Entries.end())
        iRes = it->second.flush(strFromLocalPath, pcPath);

    return(iRes);
}

int FileStatus::release(const char *pcPath, const int iFh)
{
    int iRet(0);

    const map<string, Entry>::iterator it(m_Entries.find(pcPath));

    if (it != m_Entries.end())
        iRet = it->second.release(iFh);

    return(iRet);
}



