/*
 * $Id$
 *
 * File:   testAdbncFileSystem.h
 * Author: Werner Jaeger
 *
 * Created on Nov 17, 2015, 2:15:10 PM
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
#ifndef TESTADBNCSFILESYSTEM_H
#define TESTADBNCSFILESYSTEM_H

#include <cppunit/extensions/HelperMacros.h>

class testAdbncFileSystem : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(testAdbncFileSystem);

   CPPUNIT_TEST(testTokenize);
   CPPUNIT_TEST(testStringReplacer);
   CPPUNIT_TEST(testParent);

   CPPUNIT_TEST_SUITE_END();

public:
   testAdbncFileSystem();
   virtual ~testAdbncFileSystem();
   void setUp() override;
   void tearDown() override;

private:
   void testTokenize();
   void testStringReplacer();
   void testParent();
};

#endif /* TESTADBNCSFILESYSTEM_H */

