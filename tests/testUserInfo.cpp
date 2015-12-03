/*
 * $Id$
 *
 * File:   testUserInfo.cpp
 * Author: Werner Jaeger
 *
 * Created on Nov 28, 2015, 9:59:15 AM
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
#include <unistd.h>
#include <errno.h>
#include "testUserInfo.h"
#include "userInfo.h"

CPPUNIT_TEST_SUITE_REGISTRATION(testUserInfo);

testUserInfo::testUserInfo()
{
}

testUserInfo::~testUserInfo()
{
}

void testUserInfo::setUp()
{
}

void testUserInfo::tearDown()
{
}

void testUserInfo::testAccess()
{
    // user is shell (2000, 2000)
    const UserInfo userInfo("uid=2000 gid=2000 groups=1003,1004,1007,1011,1015,1028,3001,3002,3003,3006");

    // as other
    // 0x41ed -> o40755
    // test for read access to a drwxr-xr-x root root file
    CPPUNIT_ASSERT(userInfo.access(0, 0, 0x41ed, R_OK) == 0);

    // 0x41ed -> o40755
    // test for read/write access to a drwxr-xr-x root root file
    CPPUNIT_ASSERT(userInfo.access(0, 0, 0x41ed, R_OK|W_OK) == EACCES);

    // as owner
    // 0x41ed -> o40755
    // test for read access to a drwxr-xr-x 2000 200 file
    CPPUNIT_ASSERT(userInfo.access(2000, 2000, 0x41ed, R_OK) == 0);

    // 0x41ed -> o40755
    // test for read/write access to a drwxr-xr-x 2000 200 file
    CPPUNIT_ASSERT(userInfo.access(2000, 2000, 0x41ed, R_OK|W_OK) == 0);

    // as group
    // 0x41e9 -> o40751
    // test for a drwxr-x--x root 1028(sdcard_r) file
    CPPUNIT_ASSERT(userInfo.access(0, 1028, 0x41e9, R_OK) == 0);

    // 0x41e9 -> o40751
    // test for read/write access to a drwxr-x--x root 1028(sdcard_r) file
    CPPUNIT_ASSERT(userInfo.access(0, 1028, 0x41e9, R_OK|W_OK) == EACCES);

    // 0x81a4 -> 100644
    // test for read access to a -rw-r--r-- root root file
    CPPUNIT_ASSERT(userInfo.access(0, 0, 0x81a4, R_OK) == 0);

    // 0x81a4 -> 100644
    // test for read/write access to a -rw-r--r-- root root file
    CPPUNIT_ASSERT(userInfo.access(0, 0, 0x81a4, R_OK|W_OK) == EACCES);
}

