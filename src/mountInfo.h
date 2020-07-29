/*
 * $Id$
 *
 * File:   mountInfo.h
 * Author: Werner Jaeger
 *
 * Created on November 28, 2015, 10:38 AM
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

#ifndef MOUNTINFO_H
#define MOUNTINFO_H

#include <deque>
#include <set>
#include <map>
#include <string>

using namespace std;

/**
 * A class to manage informations obtained from the output of the busybox
 * applet mount command.
 *
 * # External resources
 * [busybox](http://www.busybox.net/downloads/BusyBox.html)
 */
class MountInfo
{
private:
   /**
    * A class to manage a line obtained from the output of the
    * busybox applet mount command.
    */
   class Entry
   {
   public:
      Entry(const char* pcMountLine);
      virtual ~Entry() {}

      /**
       * Retrieve the file system name.
       *
       * @return the name of the file system.
       */
      const string& fileSystem() const { return(m_strFileSystem); }

      /**
       * Retrieve the mount point path.
       *
       * @return the mount point.
       */
      const string& mountPoint() const { return(m_strMountPoint); }

      /**
       * Retrieve the device type.
       *
       * @return the device type.
       */
      const string& deviceType() const { return(m_strDevType); }

      /**
       * Retrieve the set of mount options.
       *
       * @return the set of mount options.
       */
      const set<string>& mountOptions() const { return(m_mountOptions); }

      bool isMountedRo() const;
      bool isMountedNoexec() const;

   private:
      // Prevent default construction
      Entry();

      void parseMountInfo(const char* pcMountLine);

      string m_strFileSystem;
      string m_strMountPoint;
      string m_strDevType;
      set<string> m_mountOptions;
   };

   // Prevent default construction
   MountInfo();

   const Entry* findMountPointEntry(const string& strPath) const;
   static const string parent(const string& strPath);

   // key is the mount point
   map<string, Entry> m_Entries;
public:
   MountInfo(const deque<string>& mountInfo);
   virtual ~MountInfo() {}

   const Entry* mountPoint(const char* pcPath) const;
   bool isMountedRo(const char* pcPath) const;
   bool isMountedNoexec(const char* pcPath) const ;
};

#endif /* MOUNTINFO_H */

