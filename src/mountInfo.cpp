/*
 * $Id$
 *
 * File:   mountInfo.cpp
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <string.h>
#include "mountInfo.h"

/**
 * Initializes this object with the given mount line.
 *
 * The accepted format for the mount line parameter is one line that the
 * busybox applet mount command returns.
 *
 * @code{.unparsed}
 * Examples:
 * rootfs on / type rootfs (ro,relatime)
 * tmpfs on /dev type tmpfs (rw,seclabel,nosuid,relatime,mode=755)
 * devpts on /dev/pts type devpts (rw,seclabel,relatime,mode=600)
 * proc on /proc type proc (rw,relatime)
 * sysfs on /sys type sysfs (rw,seclabel,relatime)
 * selinuxfs on /sys/fs/selinux type selinuxfs (rw,relatime)
 * debugfs on /sys/kernel/debug type debugfs (rw,relatime)
 * @endcode
 *
 * @param pcMountLine one mount output line as described above.
 *
 * @result true if and only if line could be parsed without error,
 *         false otherwise.
 */
MountInfo::Entry::Entry(const char* pcMountLine)
{
   parseMountInfo(pcMountLine);
}

/**
 * Test this mount point is mounted read only.
 *
 * @return true if mounted with mount option "ro" false otherwise.
 */
bool MountInfo::Entry::isMountedRo() const
{
    return(m_mountOptions.find("ro") != m_mountOptions.cend());
}

/**
 * Test this mount point is mounted wit no execution option.
 *
 * @return true if mounted with mount option "noexec" false otherwise.
 */
bool MountInfo::Entry::isMountedNoexec() const
{
    return(m_mountOptions.find("noexec") != m_mountOptions.cend());
}

/**
 * Parse the given mount line and store the results in the
 * corresponding member variables.
 *
 * The accepted format for the mount line parameter is one line that the
 * busybox applet mount command returns.
 *
 * @code{.unparsed}
 * Examples:
 * rootfs on / type rootfs (ro,relatime)
 * tmpfs on /dev type tmpfs (rw,seclabel,nosuid,relatime,mode=755)
 * devpts on /dev/pts type devpts (rw,seclabel,relatime,mode=600)
 * proc on /proc type proc (rw,relatime)
 * sysfs on /sys type sysfs (rw,seclabel,relatime)
 * selinuxfs on /sys/fs/selinux type selinuxfs (rw,relatime)
 * debugfs on /sys/kernel/debug type debugfs (rw,relatime)
 * @endcode
 *
 * @param pcMountLine one mount output line as described above.
 */
void MountInfo::Entry::parseMountInfo(const char* pcMountLine)
{
    if (pcMountLine)
    {
        const int iLen(::strlen(pcMountLine) + 1);

        char acTokens[iLen];
        ::strncpy(acTokens, pcMountLine, iLen);

        char* pch = ::strtok(acTokens, " ");
        if (pch)
            m_strFileSystem = pch;

        pch = ::strtok(NULL, " ");
        if (pch && ::strcmp(pch, "on") == 0)
        {
            pch = ::strtok(NULL, " ");
            if (pch)
            {
                m_strMountPoint = pch;
                pch = ::strtok(NULL, " ");
                if (pch && ::strcmp(pch, "type") == 0)
                {
                    pch = ::strtok(NULL, " ");
                    if (pch)
                    {
                        m_strDevType = pch;
                        pch = ::strtok(NULL, "(");
                    }
                }
            }
        }

        if (pch)
        {
            pch = ::strtok(pch, ",)");
            while (pch != NULL)
            {
                m_mountOptions.insert(pch);
                pch = ::strtok (NULL, ",)");
            }
        }
    }
}

/**
 * Initializes this object with output from the busybox mount command.
 *
 * The accepted format for the mount info parameter is one that the
 * busybox applet mount returns. Each output line of the mount command is
 * stored as a queue entry
 *
 * @code{.unparsed}
 * Example:
 * rootfs on / type rootfs (ro,relatime)
 * tmpfs on /dev type tmpfs (rw,seclabel,nosuid,relatime,mode=755)
 * devpts on /dev/pts type devpts (rw,seclabel,relatime,mode=600)
 * proc on /proc type proc (rw,relatime)
 * sysfs on /sys type sysfs (rw,seclabel,relatime)
 * selinuxfs on /sys/fs/selinux type selinuxfs (rw,relatime)
 * debugfs on /sys/kernel/debug type debugfs (rw,relatime)
 * @endcode
 *
 * is stored in a queue with 7 entries
 *
 * @param mountInfo a queue of strings each element containing one mount
 *        output line as described above.
 */
MountInfo::MountInfo(const deque<string>& mountInfo)
{
    for(auto it = mountInfo.begin(); it != mountInfo.end(); ++it)
    {
        Entry entry(it->c_str());
        m_Entries.insert(make_pair(entry.mountPoint(), entry));
    }
}

/**
 * Returns the mount point of a given path.
 *
 * @param pcPath the path for that the mount point is to retrieve.
 *
 * @return a pointer to the mount point entry to which pcPath is mounted or
 *         NULL if there is no mount point entry for the given path.
 */
const MountInfo::Entry* MountInfo::mountPoint(const char* pcPath) const
{
    string strParent(pcPath);
    const Entry* pMountPointEntry(findMountPointEntry(strParent));

    while (!pMountPointEntry && strParent != "/")
    {
        strParent = parent(strParent);
        pMountPointEntry = findMountPointEntry(strParent);
    }

    return(pMountPointEntry);
}

/**
 * Test if the given path is located on a mount point that is mounted read
 * only.
 *
 * @param pcPath the path to test for may be null.
 *
 * @return false if the path is not located on a read only mounted device or
 *         if pcPath argument is null, true otherwise.
 */
bool MountInfo::isMountedRo(const char* pcPath) const
{
    const Entry* pEntry(mountPoint(pcPath));

    return(pEntry ? pEntry->isMountedRo() : false);
}

/**
 * Test if the given path is located on a mount point that is mounted with
 * the noexec option.
 *
 * @param pcPath the path to test for may be null.
 * @return false if the path is not located on a device mounted withe the
 *         noexec option or if pcPath argument is null, true otherwise.
 */
bool MountInfo::isMountedNoexec(const char* pcPath) const
{
    const MountInfo::Entry* pEntry(mountPoint(pcPath));

    return(pEntry ? pEntry->isMountedNoexec() : false);
}

/**
 * Lookup if there is a mount point entry with the given path.
 *
 * @param strPath the path to lookup.
 *
 * @return a pointer to the entry found or NULL if there is no entry with a
 *         mount point of the given path.
 */
const MountInfo::Entry* MountInfo::findMountPointEntry(const string& strPath) const
{
    const Entry* pMountPointEntry(NULL);

    for (auto it = m_Entries.begin(); !pMountPointEntry && it != m_Entries.end(); ++it)
    {
        if (it->first == strPath)
            pMountPointEntry = &it->second;
    }

    return(pMountPointEntry);
}

/**
 * Returns the parent pathname string of the given strPath.
 *
 * The parent of strPath consists of the pathname's prefix, if any, and
 * each name in the strPath name sequence except for the last.
 *
 * @param strPath the pathname thats parent to retrieve.
 *
 * @return The pathname string of the parent directory named by strPath
 */
const string MountInfo::parent(const string& strPath)
{
    string strParent(strPath);
    const size_t uiLen(strParent.length());

    /* if strPath == / we are done*/
    if (uiLen != 1 || strParent[0] != '/')
    {
        if (uiLen > 0)
        {
            /** strip last slash if there is one */
            if (strParent[uiLen - 1] == '/')
                strParent = strParent.substr(0, uiLen - 1);

            const std::size_t uiPos(strParent.rfind('/'));
            if (uiPos != string::npos)
                strParent = strParent.substr(0, uiPos == 0 ? 1 : uiPos);

            if (strParent == strPath)
                strParent = ".";
        }
        else
            strParent = ".";
    }

    return(strParent);
}



