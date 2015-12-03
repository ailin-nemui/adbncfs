/*
 * $Id$
 *
 * File:   userInfo.h
 * Author: Werner Jaeger
 *
 * Created on November 26, 2015, 8:53 PM
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

#ifndef USERINFO_H
#define USERINFO_H

#include <set>

/**
 * A class to manage informations obtained from the output of the busybox
 * applet id command.
 *
 * # External resources
 * [busybox](http://www.busybox.net/downloads/BusyBox.html)
 */
class UserInfo
{
public:
   UserInfo(const int iUid, const int iGid, const std::set<int>& groups) : m_iUid(iUid), m_iGid(iGid), m_Groups(groups) {}
   UserInfo(const char* pcUserInfo);
   virtual ~UserInfo() {}

   int access(const int iUid, const int iGid, const unsigned int uiRawMode, const int iMask) const;

private:
   // prevent default constructor
   UserInfo() {}

   bool belongs2Group(const int iGuid) const;
   int testAccesss(const int iMode, const int iMask) const;

   void parseUserInfoString(const char* pcUserInfo);

   int m_iUid;
   int m_iGid;
   std::set<int> m_Groups;
};

#endif /* USERINFO_H */

