#
# $Id$
#
# File:   Makefile
# Author: Werner Jaeger
#
# Created on November 17, 2015, 1:28 PM
#
# Copyright 2015 Werner Jaeger.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Environment
INSTALL_PROGRAM      = install -m 755 -p
INSTALL_FILE         = install -m 644 -p
DEL_FILE             = rm -f
DEL_DIR              = rmdir
CHK_DIR_EXISTS       = test -d
MKDIR                = mkdir -p
MANDB                = mandb -q
API_DIR              = docs/api
TARGET_DIR           = ${CND_ARTIFACT_DIR_${CONF}}
TARGET               = ${CND_ARTIFACT_NAME_${CONF}}
INSTALL_DIR          = usr/local/bin
MAN_PAGE_INSTALL_DIR = usr/local/share/man/man1
MAN_PAGE_TARGET_DIR  = docs
MAN_PAGE_TARGET      = adbncfs.1
CCADMIN=CCadmin

# build
build: .build-post

.build-pre:

.build-post: .build-impl

# clean
clean: .clean-post

.clean-pre:

.clean-post: .clean-impl

# clobber
clobber: .clobber-post

.clobber-pre:

.clobber-post: .clobber-impl
	-$(DEL_FILE)r $(API_DIR)


# all
all: .all-post

.all-pre:

.all-post: .all-impl


# build tests
build-tests: .build-tests-post

.build-tests-pre:

.build-tests-post: .build-tests-impl


# run tests
test: .test-post

.test-pre: build-tests

.test-post: .test-impl

# help
help: FORCE
	@echo "This makefile supports the following configurations:"
	@echo "    ${ALLCONFS}"
	@echo ""
	@echo "and the following targets:"
	@echo "    build  (default target)"
	@echo "    clean"
	@echo "    clobber"
	@echo "    all"
	@echo "    install"
	@echo "    uninstall"
	@echo "    help"
	@echo ""
	@echo "Makefile Usage:"
	@echo "    make [CONF=<CONFIGURATION>] [SUB=no] build"
	@echo "    make [CONF=<CONFIGURATION>] [SUB=no] clean"
	@echo "    make [SUB=no] clobber"
	@echo "    make [SUB=no] all"
	@echo "    make [CONF=<CONFIGURATION>] [INSTALL_ROOT=<Base directory to install in>] install"
	@echo "    make [INSTALL_ROOT=<Base directory to uninstall from>] uninstall"
	@echo "    make help"
	@echo ""
	@echo "Target 'build' will build a specific configuration and, unless 'SUB=no',"
	@echo "    also build subprojects."
	@echo "Target 'clean' will clean a specific configuration and, unless 'SUB=no',"
	@echo "    also clean subprojects."
	@echo "Target 'clobber' will remove all built files from all configurations and,"
	@echo "    unless 'SUB=no', also from subprojects."
	@echo "Target 'all' will will build all configurations and, unless 'SUB=no',"
	@echo "    also build subprojects."
	@echo "Target 'doc' will generate documentation from source code"
	@echo "Target 'srccheck' will perform a static analysis"
	@echo "Target 'install' will install a specific configuration of the program"
	@echo "       in [INSTALL_ROOT]/$(INSTALL_DIR)/"
	@echo "Target 'uninstall' will uninstall the program from [INSTALL_ROOT]/$(INSTALL_DIR)/"
	@echo "Target 'help' prints this message."
	@echo ""

install:  FORCE
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/$(INSTALL_DIR)/ || $(MKDIR) $(INSTALL_ROOT)/$(INSTALL_DIR)/
	-$(INSTALL_PROGRAM) "$(TARGET_DIR)/$(TARGET)" "$(INSTALL_ROOT)/$(INSTALL_DIR)/$(TARGET)"
	@$(CHK_DIR_EXISTS) $(INSTALL_ROOT)/$(MAN_PAGE_INSTALL_DIR)/ || $(MKDIR) $(INSTALL_ROOT)/$(MAN_PAGE_INSTALL_DIR)/
	-$(INSTALL_FILE) "$(MAN_PAGE_TARGET_DIR)/$(MAN_PAGE_TARGET)" "$(INSTALL_ROOT)/$(MAN_PAGE_INSTALL_DIR)/$(MAN_PAGE_TARGET)"
	@$(MANDB)

uninstall:  FORCE
	-$(DEL_FILE) "$(INSTALL_ROOT)/$(INSTALL_DIR)/$(TARGET)"
	@$(DEL_DIR) --ignore-fail-on-non-empty $(INSTALL_ROOT)/$(INSTALL_DIR)/
	-$(DEL_FILE) "$(INSTALL_ROOT)/$(MAN_PAGE_INSTALL_DIR)/$(MAN_PAGE_TARGET)"
	@$(DEL_DIR) --ignore-fail-on-non-empty $(INSTALL_ROOT)/$(MAN_PAGE_INSTALL_DIR)/
	@$(MANDB)

doc:
	doxygen docs/Doxyfile

srccheck:
	cppcheck -q -I src --enable=all --suppress=missingIncludeSystem src

FORCE:

# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk

# default configuration is release
DEFAULTCONF=Release
