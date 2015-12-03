/*
 * $Id$
 *
 * File:   testMountPoint.h
 * Author: Werner Jaeger
 *
 * Created on Nov 29, 2015, 7:52:09 PM
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
#ifndef TESTMOUNTPOINT_H
#define TESTMOUNTPOINT_H

#include <cppunit/extensions/HelperMacros.h>
#include "mountInfo.h"

class testMountPoint : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(testMountPoint);

   CPPUNIT_TEST(testMountPointInfo);
   CPPUNIT_TEST(testMountPointOptions);

   CPPUNIT_TEST_SUITE_END();

public:
   testMountPoint();
   virtual ~testMountPoint();
   void setUp() override;
   void tearDown() override;

private:
   void testMountInfoEntry();
   void testMountPointInfo();
   void testMountPointOptions();

   static std::deque<std::string> initMountInfo();

   const MountInfo m_MountInfo;
};

#endif /* TESTMOUNTPOINT_H */

