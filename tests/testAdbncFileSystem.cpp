/*
 * $Id$
 *
 * File:   testAdbncFileSystem.cpp
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
#include "testAdbncFileSystem.h"
#include "adbncfs.h"

using namespace std;

// Forward declarations for static function in adbncfs.cpp.
extern vector<string> tokenize(const string& strData);
void stringReplacer(string& strSource, const string& strFind, const string& strReplace);
string parent(const string& strPath);

CPPUNIT_TEST_SUITE_REGISTRATION(testAdbncFileSystem);

testAdbncFileSystem::testAdbncFileSystem()
{
}

testAdbncFileSystem::~testAdbncFileSystem()
{
}

void testAdbncFileSystem::setUp()
{
}

void testAdbncFileSystem::tearDown()
{
}

void testAdbncFileSystem::testTokenize()
{
    vector<string> tokens(tokenize("abc def\tghi"));

    CPPUNIT_ASSERT(tokens.size() == 3);
    CPPUNIT_ASSERT(tokens[0] == "abc");
    CPPUNIT_ASSERT(tokens[1] == "def");
    CPPUNIT_ASSERT(tokens[2] == "ghi");
}

void testAdbncFileSystem::testStringReplacer()
{
    string strTestee("/storage/extSdCard/test1/test1.txt");
    stringReplacer(strTestee, "/", "-");
    CPPUNIT_ASSERT(strTestee == "-storage-extSdCard-test1-test1.txt");
}

void testAdbncFileSystem::testParent()
{
    const string str1("/usr/lib");
    const string str2("/usr/");
    const string str3("usr");
    const string str4("/");
    const string str5(".");
    const string str6("..");
    const string str7("");

    CPPUNIT_ASSERT(parent(str1) == "/usr");
    CPPUNIT_ASSERT(parent(str2) == "/");
    CPPUNIT_ASSERT(parent(str3) == ".");
    CPPUNIT_ASSERT(parent(str4) == "/");
    CPPUNIT_ASSERT(parent(str5) == ".");
    CPPUNIT_ASSERT(parent(str6) == ".");
    CPPUNIT_ASSERT(parent(str7) == ".");
}
