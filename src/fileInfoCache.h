/*
 * $Id$
 *
 * File:   fileInfoCche.h
 * Author: Werner Jaeger
 *
 * Created on November 19, 2015, 5:15 PM
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

#ifndef FILEINFOCACHE_H
#define FILEINFOCACHE_H

#include <string>
#include <queue>
#include <map>
#include <unistd.h>

using namespace std;

/**
 * A cache for file attributes and resolved links.
 */
class FileCache
{
public:
   /** Default constructor. */
   FileCache() : m_Entries() {}

   /** Virtual destructor. */
   virtual ~FileCache() {}

   // setters
   void putStat(const char *pcPath, const deque<string>& statOutput);
   void putReadLink(const char *pcPath, const deque<string>& readLinkOutput);

   // getters
   const deque<string>* getStat(const char *pcPath) const ;
   const deque<string>* getReadLink(const char *pcPath) const;

   //operations
   void invalidate(const char *pcPath);

private:
   /**
    * Represents an entry of FileCache.
    */
   class Entry
   {
   public:
      /** Default constructor. */
      Entry() : m_Timestamp(::time(NULL)), m_pStatOutput(NULL), m_pReadLinkOutput(NULL) {}
      Entry(const Entry& orig);
      Entry& operator=(const Entry& orig);
      virtual ~Entry();

      /**
       * Set the entries time stamp
       *
       * @param time the time stamp to set.
       */
      void timeStamp(const time_t& time) { m_Timestamp = time; }
      void statOutput(const deque<string>& output);
      void readLinkOutput(const deque<string>& output);

      // getters
      const time_t timeStamp() const { return(m_Timestamp); }
      const deque<string> *statOutput() const { return(m_pStatOutput); }
      const deque<string> *readLinkOutput() const { return(m_pReadLinkOutput); }

   private:
      time_t m_Timestamp;
      deque<string> *m_pStatOutput;
       deque<string> *m_pReadLinkOutput;
   };

   /** Prevent copy-construction */
   FileCache(const FileCache& orig);

   /** Prevent assignment */
   FileCache operator=(const FileCache& orig);

   bool isValid(const Entry& entry) const;

   map<string, Entry> m_Entries;
};

/**
 * Keep track of files opened and truncated files.
 */
class FileStatus
{
public:
   /** Default constructor. */
   FileStatus() : m_Entries() {}

   /** Virtual destructor. */
   virtual ~FileStatus() {}

   // setters
   void pendingOpen(const char *pcPath, const bool fPendingOpen, const bool fForWrite);
   void pendingOpen(const char *pcPath, const string& strRenamedFromLocal);
   void truncated(const char *pcPath, const bool fTruncated);

   // getters
   bool pendingOpen(const char *pcPath) const;
   bool truncated(const char *pcPath) const;

   // operations
   int flush(const char *pcPath, const string& strFromLocalPath);
   int release(const char *pcPath, const int iFh);

private:
   /**
    * Represents an entry of FileStatus.
    */
   class Entry
   {
   public:
      /** Default constructor. */
      Entry() : m_fPendingOpen(false), m_fForWrite(false), m_fTruncated(false), m_strRenamedFromLocal() {}

      /** Virtual destructor. */
      virtual ~Entry() {}

      // setters
      void pendingOpen(const bool fPendingOpen, const bool fForWrite) { m_fPendingOpen = fPendingOpen; m_fForWrite = fForWrite; }
      void pendingOpen(const string& strRenamedFromLocal) { m_fPendingOpen = true; m_strRenamedFromLocal.assign(strRenamedFromLocal); }
      void truncated(const bool fTruncated) { m_fTruncated = fTruncated; }

      // getters
      const bool pendingOpen() const { return(m_fPendingOpen); }
      const bool truncated() const { return(m_fTruncated); }

      // ops
      int release(const int iFh);
      int flush(const string& strFromLocalPath, const string& strToPat);

   private:
      bool m_fPendingOpen;
      bool m_fForWrite;
      bool m_fTruncated;
      string m_strRenamedFromLocal;
   };

   /** Prevent copy-construction */
   FileStatus(const FileStatus& orig);

   /** Prevent assignment */
   FileStatus operator=(const FileStatus& orig);

   map<string, Entry> m_Entries;
};


#endif /* FILEINFOCACHE_H */

