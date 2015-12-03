/*
 * $Id$
 *
 * File:   userInfo.cpp
 * Author: Werner Jaeger
 *
 * Created on November 26, 2015, 8:54 PM
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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "userInfo.h"

using namespace std;

/**
 * Initialize this object with the specified user id and groups.
 *
 * The accepted format for the user info parameter is one that the
 * busybox applet id returns.
 *
 * Example:
 * uid=2000 gid=2000 groups=1003,1004,1007,1011,1015,1028,3001,3002,3003,3006
 *
 * @param pcUserInfo string containing user informations as described above.
 */
UserInfo::UserInfo(const char* pcUserInfo)
{
    parseUserInfoString(pcUserInfo);
}

/**
 * This is the similar to the access(2) system call.
 *
 * It returns -EACCESS if the requested permission for this user
 * isn't available, or 0 for success.
 *
 * The following flags are defined for the uiRawMode argument:
 *
 * S_IFMT     0170000   bit mask for the file type bit fields
 * S_IFSOCK   0140000   socket
 * S_IFLNK    0120000   symbolic link
 * S_IFREG    0100000   regular file
 * S_IFBLK    0060000   block device
 * S_IFDIR    0040000   directory
 * S_IFCHR    0020000   character device
 * S_IFIFO    0010000   FIFO
 * S_ISUID    0004000   set UID bit
 * S_ISGID    0002000   set-group-ID bit (see below)
 * S_ISVTX    0001000   sticky bit (see below)
 * S_IRWXU    00700     mask for file owner permissions
 * S_IRUSR    00400     owner has read permission
 * S_IWUSR    00200     owner has write permission
 * S_IXUSR    00100     owner has execute permission
 * S_IRWXG    00070     mask for group permissions
 * S_IRGRP    00040     group has read permission
 * S_IWGRP    00020     group has write permission
 * S_IXGRP    00010     group has execute permission
 * S_IRWXO    00007     mask for permissions for others (not in group)
 * S_IROTH    00004     others have read permission
 * S_IWOTH    00002     others have write permission
 * S_IXOTH    00001     others have execute permission
 * This is a bit field, and we can see that the last four digits match the
 * access mode
 *
 * @param iUid  check access for an object with this user id.
 * @param iGid check access for an object with this group id.
 * @param uiRawMode the mode to check for, as defined in struct stat st_mode
 *        field, it is described in the programmer's manual for the stat
 *        function (man 2 stat).
 * @param iMask specifies the accessibility check(s) to be performed.
 *        is either bitwise OR of one or more of R_OK, W_OK, and X_OK as
 *        defined in unistd.h.
 *
 * @return -EACCESS if access is denied or 0 if access is granted.
 */
int UserInfo::access(const int iUid, const int iGid, const unsigned int uiRawMode, const int iMask) const
{
    int iRet((m_iUid == 0 && m_iGid == 0) ? 0 : EACCES); // root has all permissions

    if (m_iUid == iUid)
        iRet = testAccesss((uiRawMode & S_IRWXU) >> 6, iMask);
    else if (belongs2Group(iGid))
        iRet = testAccesss((uiRawMode & S_IRWXG) >> 3, iMask);
    else
        iRet = testAccesss(uiRawMode & S_IRWXO, iMask);

    return(iRet);
}

/**
 * Tests if this user belongs to the specified group,
 *
 * @param iGuid the group id to test for.
 * @return true if and only if this user belongs to the specified group,
 *         false otherwise.
 */
bool UserInfo::belongs2Group(const int iGuid) const
{
    return(m_iGid == iGuid || m_Groups.find(iGuid) != m_Groups.end());
}

/**
 * Test requested access permissions against the mode;
 *
 * @param iMode permissions to test for.
 * @param iMask the requested access permissions.
 */
int UserInfo::testAccesss(const int iMode, const int iMask) const
{
    int iTestMode(iMode & iMask);
    return(iTestMode == iMask ? 0 : EACCES);
}

/**
 * Parse the specified user info string and store the results in the
 * corresponding member variables.
 *
 * The accepted format for the user info parameter is one that the
 * busybox applet id returns.
 *
 * Example:
 * uid=2000 gid=2000 groups=1003,1004,1007,1011,1015,1028,3001,3002,3003,3006
 *
 * @param pcUserInfo string containing user informations as described above.
 */
void UserInfo::parseUserInfoString(const char* pcUserInfo)
{
    if (pcUserInfo)
    {
        const char* pcEqualDelim = "= ";
        const char* pcUid = "uid";
        const char* pcGroups = "groups";

        const int iLen(::strlen(pcUserInfo) + 1);

        char acTokens[iLen];
        ::strncpy(acTokens, pcUserInfo, iLen);

        char* pch = ::strtok(acTokens, pcEqualDelim);
        if (pch && ::strcmp(pch, pcUid) == 0)
        {
            pch = ::strtok(NULL, pcEqualDelim);
            if (pch)
            {
                m_iUid = ::atoi(pch);
                pch = ::strtok(NULL, pcEqualDelim);
                const char* pcGid = "gid";
                if (pch && ::strcmp(pch, pcGid) == 0)
                {
                    pch = ::strtok(NULL, pcEqualDelim);
                    if (pch)
                    {
                        m_iGid = ::atoi(pch);
                        pch = ::strtok(NULL, pcEqualDelim);
                    }
                }
            }
        }

        if (pch && ::strcmp(pch, pcGroups) == 0)
        {
            const char* pcCommaDelim = ",";
            pch = ::strtok(NULL, pcCommaDelim);
            while (pch != NULL)
            {
                m_Groups.insert(::atoi(pch));
                pch = ::strtok (NULL, pcCommaDelim);
            }
        }
    }
}
