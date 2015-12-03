#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/adbncfs.o \
	${OBJECTDIR}/src/fileinfoCache.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/mountInfo.o \
	${OBJECTDIR}/src/spawn.o \
	${OBJECTDIR}/src/userInfo.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f2

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/tests/adbncFileSystemTestRunner.o \
	${TESTDIR}/tests/mountPointTestRunner.o \
	${TESTDIR}/tests/testAdbncFileSystem.o \
	${TESTDIR}/tests/testMountPoint.o \
	${TESTDIR}/tests/testUserInfo.o \
	${TESTDIR}/tests/userInfoTestRunner.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/adbncfs

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/adbncfs: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/adbncfs ${OBJECTFILES} ${LDLIBSOPTIONS} -pthread -lfuse

${OBJECTDIR}/src/adbncfs.o: src/adbncfs.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/adbncfs.o src/adbncfs.cpp

${OBJECTDIR}/src/fileinfoCache.o: src/fileinfoCache.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fileinfoCache.o src/fileinfoCache.cpp

${OBJECTDIR}/src/main.o: src/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/mountInfo.o: src/mountInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/mountInfo.o src/mountInfo.cpp

${OBJECTDIR}/src/spawn.o: src/spawn.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/spawn.o src/spawn.cpp

${OBJECTDIR}/src/userInfo.o: src/userInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/userInfo.o src/userInfo.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f3: ${TESTDIR}/tests/mountPointTestRunner.o ${TESTDIR}/tests/testMountPoint.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS} `cppunit-config --libs` `pkg-config --libs fuse`   

${TESTDIR}/TestFiles/f1: ${TESTDIR}/tests/adbncFileSystemTestRunner.o ${TESTDIR}/tests/testAdbncFileSystem.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS} `cppunit-config --libs` `pkg-config --libs fuse`   

${TESTDIR}/TestFiles/f2: ${TESTDIR}/tests/testUserInfo.o ${TESTDIR}/tests/userInfoTestRunner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc}   -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS} `cppunit-config --libs` `pkg-config --libs fuse`   


${TESTDIR}/tests/mountPointTestRunner.o: tests/mountPointTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/mountPointTestRunner.o tests/mountPointTestRunner.cpp


${TESTDIR}/tests/testMountPoint.o: tests/testMountPoint.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/testMountPoint.o tests/testMountPoint.cpp


${TESTDIR}/tests/adbncFileSystemTestRunner.o: tests/adbncFileSystemTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/adbncFileSystemTestRunner.o tests/adbncFileSystemTestRunner.cpp


${TESTDIR}/tests/testAdbncFileSystem.o: tests/testAdbncFileSystem.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/testAdbncFileSystem.o tests/testAdbncFileSystem.cpp


${TESTDIR}/tests/testUserInfo.o: tests/testUserInfo.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/testUserInfo.o tests/testUserInfo.cpp


${TESTDIR}/tests/userInfoTestRunner.o: tests/userInfoTestRunner.cpp 
	${MKDIR} -p ${TESTDIR}/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -Isrc -std=c++11 `cppunit-config --cflags` -MMD -MP -MF "$@.d" -o ${TESTDIR}/tests/userInfoTestRunner.o tests/userInfoTestRunner.cpp


${OBJECTDIR}/src/adbncfs_nomain.o: ${OBJECTDIR}/src/adbncfs.o src/adbncfs.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/adbncfs.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/adbncfs_nomain.o src/adbncfs.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/adbncfs.o ${OBJECTDIR}/src/adbncfs_nomain.o;\
	fi

${OBJECTDIR}/src/fileinfoCache_nomain.o: ${OBJECTDIR}/src/fileinfoCache.o src/fileinfoCache.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/fileinfoCache.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fileinfoCache_nomain.o src/fileinfoCache.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/fileinfoCache.o ${OBJECTDIR}/src/fileinfoCache_nomain.o;\
	fi

${OBJECTDIR}/src/main_nomain.o: ${OBJECTDIR}/src/main.o src/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main_nomain.o src/main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/main_nomain.o;\
	fi

${OBJECTDIR}/src/mountInfo_nomain.o: ${OBJECTDIR}/src/mountInfo.o src/mountInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/mountInfo.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/mountInfo_nomain.o src/mountInfo.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/mountInfo.o ${OBJECTDIR}/src/mountInfo_nomain.o;\
	fi

${OBJECTDIR}/src/spawn_nomain.o: ${OBJECTDIR}/src/spawn.o src/spawn.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/spawn.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/spawn_nomain.o src/spawn.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/spawn.o ${OBJECTDIR}/src/spawn_nomain.o;\
	fi

${OBJECTDIR}/src/userInfo_nomain.o: ${OBJECTDIR}/src/userInfo.o src/userInfo.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/userInfo.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O2 -D_FILE_OFFSET_BITS=64 -std=c++11 -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/userInfo_nomain.o src/userInfo.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/userInfo.o ${OBJECTDIR}/src/userInfo_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/adbncfs

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
