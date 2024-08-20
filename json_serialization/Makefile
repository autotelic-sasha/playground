#############
# Utilities #
#############
# CURRENT_PATH is the path where this makefile lives.
# It is very useful for setting up the configuration of the build.
CURRENT_PATH := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

#########################
# Project configuration #
#########################
# Build target name
TARGET_BASE_NAME := json_serialization
# To build shared libraries, set this to yes.
TARGET_IS_LIBRARY := no
# Base name of the base shared library being tested, if this is a dll test project.
BASE_SO_BASE_NAME := 
# Additional macro definitions for the build (NDEBUG for release and DEBUG for debug are always defined anyway).
RELEASE_MACRO_DEFINITIONS := 
DEBUG_MACRO_DEFINITIONS := $(RELEASE_MACRO_DEFINITIONS)
# Additional include paths
INCLUDE_PATHS := -I$(CURRENT_PATH)/../
# cpp files to exclude from linux builds.
EXCLUDE_FILES := 
# cpp files that are only to be built for tests
TEST_ONLY_FILES := json_serialization_test.cpp json_serialization_examples.cpp
ADDITIONAL_TEST_FILES := 
# Linker flags.
COMMON_LDFLAGS := -L/usr/lib -lstdc++ -lm 
RELEASE_ONLY_LDFLAGS := 
DEBUG_ONLY_LDFLAGS := 
# LD_LIBRARY_PATH to set before invoking g++ linker.
RELEASE_ONLY_LD_LIBRARY_PATHS := 
DEBUG_ONLY_LD_LIBRARY_PATHS := 
# Include files to package.
INCLUDE_FILES_TO_PACKAGE := 
# Additional packaging commands. 
# Note: these are executed from within RELEASE_PACKAGE_DIR and DEBUG_PACKAGE_DIR resp.
RELEASE_ADDITIONAL_PACKAGING := 
DEBUG_ADDITIONAL_PACKAGING := 
# LD_LIBRARY_PATHs for packaging.
RELEASE_LD_LIBRARY_PATH :=
DEBUG_LD_LIBRARY_PATH :=
# Pre-build and post-build steps.
# Note: These are executed from CURRENT_PATH directory.
RELEASE_PRE_BUILD_STEP := 
DEBUG_PRE_BUILD_STEP := $(RELEASE_PRE_BUILD_STEP)

RELEASE_POST_BUILD_STEP := 
DEBUG_POST_BUILD_STEP := $(RELEASE_POST_BUILD_STEP) 
#############################
# Project configuration end #
#############################

############################################
# Internals.                               #
# This is complicated, I know. 			   #
# That's why we do it for you :)           #
############################################

# Tidying up - spaces are  not allowed in target names.
EMPTY := 
SPACE := $(EMPTY) $(EMPTY)
TIDY_TARGET_NAME := $(subst $(SPACE),_,$(strip $(TARGET_BASE_NAME)))

# Directories
BUILD    :=  $(CURRENT_PATH)/linux_build
OBJ_DIR  := $(BUILD)/objects
RELEASE_OBJ_DIR  := $(OBJ_DIR)/release
DEBUG_OBJ_DIR  := $(OBJ_DIR)/debug
PACKAGE_DIR  := $(BUILD)/packages
RELEASE_PACKAGE_DIR  := $(PACKAGE_DIR)/$(TIDY_TARGET_NAME)/release
DEBUG_PACKAGE_DIR  := $(PACKAGE_DIR)/$(TIDY_TARGET_NAME)/debug

ifeq ($(TARGET_IS_LIBRARY), yes)
# When building shared libraries, the target names are made to match the usual *nix standards'
# They are also packaged into a 'lib' directory.
SO_PREFIX := lib
SO_SUFFIX := .so
RELEASE_TARGET   := $(SO_PREFIX)$(TIDY_TARGET_NAME)$(SO_SUFFIX)
DEBUG_TARGET   := $(SO_PREFIX)$(TIDY_TARGET_NAME)_d$(SO_SUFFIX)
OUT_DIR := $(BUILD)/lib
RELEASE_PACKAGE_LIB_DIR  := $(RELEASE_PACKAGE_DIR)/lib
DEBUG_PACKAGE_LIB_DIR  := $(DEBUG_PACKAGE_DIR)/lib
RELEASE_TEST_TARGET   := 
DEBUG_TEST_TARGET   := 
else
RELEASE_TARGET   := $(TIDY_TARGET_NAME)
DEBUG_TARGET   := $(TIDY_TARGET_NAME)_d
OUT_DIR := $(BUILD)/apps
RELEASE_TEST_TARGET   := $(TIDY_TARGET_NAME)_test
DEBUG_TEST_TARGET   := $(TIDY_TARGET_NAME)_test_d
endif

ifneq ($(INCLUDE_FILES_TO_PACKAGE), )
RELEASE_PACKAGE_INCLUDE_DIR  := $(RELEASE_PACKAGE_DIR)/include
DEBUG_PACKAGE_INCLUDE_DIR  := $(DEBUG_PACKAGE_DIR)/include
endif

INCLUDE  := $(INCLUDE_PATHS)

# Shared library test project setup.
# For these, the package contains the copy of the library itself, LD_LIBRARY_PATH needs to be setup ... all sorts.
ifneq ($(BASE_SO_BASE_NAME), )
# Details of the base shared library being tested.
RELEASE_BASE_SO_PATH := $(CURRENT_PATH)/../linux_build/lib
DEBUG_BASE_SO_PATH := $(CURRENT_PATH)/../linux_build/lib
# Linker flags.
RELEASE_LDFLAGS := $(COMMON_LDFLAGS) $(RELEASE_ONLY_LDFLAGS) -L$(RELEASE_BASE_SO_PATH) -l$(BASE_SO_BASE_NAME)
DEBUG_LDFLAGS := $(COMMON_LDFLAGS) $(DEBUG_ONLY_LDFLAGS) -L$(DEBUG_BASE_SO_PATH) -l$(BASE_SO_BASE_NAME)_d
# LD_LIBRARY_PATH to set before invoking g++. 
ifneq ($(RELEASE_ONLY_LD_LIBRARY_PATHS), )
RELEASE_ADDITIONAL_LD_LIBRARY_PATHS := $(RELEASE_ONLY_LD_LIBRARY_PATHS):$(RELEASE_BASE_SO_PATH)
else
RELEASE_ADDITIONAL_LD_LIBRARY_PATHS := $(RELEASE_BASE_SO_PATH)
endif
ifneq ($(DEBUG_ONLY_LD_LIBRARY_PATHS), )
DEBUG_ADDITIONAL_LD_LIBRARY_PATHS := $(DEBUG_ONLY_LD_LIBRARY_PATHS):$(DEBUG_BASE_SO_PATH)
else
DEBUG_ADDITIONAL_LD_LIBRARY_PATHS := $(DEBUG_BASE_SO_PATH)
endif
# Additional packaging commands. 
ifneq ($(RELEASE_ADDITIONAL_PACKAGING), )
RELEASE_ADDITIONAL_PACKAGING := $(RELEASE_ADDITIONAL_PACKAGING); cp -vf $(RELEASE_BASE_SO_PATH)/lib$(BASE_SO_BASE_NAME).so .
else
RELEASE_ADDITIONAL_PACKAGING := cp -vf $(RELEASE_BASE_SO_PATH)/lib$(BASE_SO_BASE_NAME).so .
endif
ifneq ($(DEBUG_ADDITIONAL_PACKAGING), )
DEBUG_ADDITIONAL_PACKAGING := $(DEBUG_ADDITIONAL_PACKAGING); cp -vf $(DEBUG_BASE_SO_PATH)/lib$(BASE_SO_BASE_NAME)_d.so .
else
DEBUG_ADDITIONAL_PACKAGING := echo $(pwd);cp -vf $(DEBUG_BASE_SO_PATH)/lib$(BASE_SO_BASE_NAME)_d.so .
endif
# LD_LIBRARY_PATHs for packaging
ifneq ($(RELEASE_LD_LIBRARY_PATH), )
RELEASE_LD_LIBRARY_PATH :=$(RELEASE_LD_LIBRARY_PATH):'$$(dirname $$0)'
else
RELEASE_LD_LIBRARY_PATH :='$$(dirname $$0)'
endif
ifneq ($(DEBUG_LD_LIBRARY_PATH), )
DEBUG_LD_LIBRARY_PATH :=$(DEBUG_LD_LIBRARY_PATH):'$$(dirname $$0)'
else
DEBUG_LD_LIBRARY_PATH :='$$(dirname $$0)'
endif
# There is a dependency here on the base library being built	
ifneq ($(RELEASE_PRE_BUILD_STEP), )
RELEASE_PRE_BUILD_STEP := cd $(CURRENT_PATH)/..;make package_release;cd $(CURRENT_PATH);$(RELEASE_PRE_BUILD_STEP)
else
RELEASE_PRE_BUILD_STEP := cd $(CURRENT_PATH)/..;make package_release;cd $(CURRENT_PATH)
endif
ifneq ($(DEBUG_PRE_BUILD_STEP), )
DEBUG_PRE_BUILD_STEP := cd $(CURRENT_PATH)/..;make package_debug;cd $(CURRENT_PATH);$(DEBUG_PRE_BUILD_STEP)
else
DEBUG_PRE_BUILD_STEP := cd $(CURRENT_PATH)/..;make package_debug;cd $(CURRENT_PATH)
endif
else # not a shared library shared project
# Linker flags.
RELEASE_LDFLAGS := $(COMMON_LDFLAGS) $(RELEASE_ONLY_LDFLAGS) 
DEBUG_LDFLAGS := $(COMMON_LDFLAGS) $(DEBUG_ONLY_LDFLAGS) 
# LD_LIBRARY_PATH to set before invoking g++. 
RELEASE_ADDITIONAL_LD_LIBRARY_PATHS := $(RELEASE_ONLY_LD_LIBRARY_PATHS) 
DEBUG_ADDITIONAL_LD_LIBRARY_PATHS := $(DEBUG_ONLY_LD_LIBRARY_PATHS) 
endif
# Tidying up - removing spaces where wew don't want them, mostly.
# This is for shell script snippets that we will be running.
TIDY_DEBUG_ADDITIONAL_PACKAGING := $(strip $(DEBUG_ADDITIONAL_PACKAGING))
TIDY_RELEASE_ADDITIONAL_PACKAGING := $(strip $(RELEASE_ADDITIONAL_PACKAGING))
TIDY_RELEASE_LD_LIBRARY_PATH := $(strip $(RELEASE_ADDITIONAL_LD_LIBRARY_PATHS))
TIDY_DEBUG_LD_LIBRARY_PATH := $(strip $(DEBUG_ADDITIONAL_LD_LIBRARY_PATHS))
TIDY_RELEASE_PRE_BUILD_STEP := $(strip $(RELEASE_PRE_BUILD_STEP))
TIDY_DEBUG_PRE_BUILD_STEP := $(strip $(DEBUG_PRE_BUILD_STEP))
TIDY_RELEASE_POST_BUILD_STEP := $(strip $(RELEASE_POST_BUILD_STEP))
TIDY_DEBUG_POST_BUILD_STEP := $(strip $(DEBUG_POST_BUILD_STEP))

##################
# Compiler setup #
##################
ifeq ($(TIDY_RELEASE_LD_LIBRARY_PATH),)
RELEASE_CXX      := g++
else
RELEASE_CXX      := export LD_LIBRARY_PATH=$(TIDY_RELEASE_LD_LIBRARY_PATH):${LD_LIBRARY_PATH};g++
endif
ifeq ($(TIDY_DEBUG_LD_LIBRARY_PATH),)
DEBUG_CXX      := g++
else
DEBUG_CXX      := export LD_LIBRARY_PATH=$(TIDY_DEBUG_LD_LIBRARY_PATH):${LD_LIBRARY_PATH};g++
endif
CXX_IGNORED_WARNINGS := -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-ignored-qualifiers 
CXX_BASE_FLAGS := -std=c++14 -fvisibility=hidden -fdiagnostics-color=always -Wall -Wextra -Werror $(CXX_IGNORED_WARNINGS)
CXX_DEBUG_FLAGS := -D_DEBUG -g
CXX_RELEASE_FLAGS := -DNDEBUG -O2
ifeq ($(TARGET_IS_LIBRARY), yes)
CXX_SO_PARAMETERS := -shared
else
CXX_SO_PARAMETERS := 
endif

# Source files
ALL_SRC      :=           \
   $(wildcard *.cpp)         

SRC := $(filter-out $(TEST_ONLY_FILES), $(filter-out $(EXCLUDE_FILES),$(ALL_SRC)))
HEADERS := $(wildcard *.h)

RELEASE_OBJECTS  := $(SRC:%.cpp=$(RELEASE_OBJ_DIR)/%.o)
DEBUG_OBJECTS  := $(SRC:%.cpp=$(DEBUG_OBJ_DIR)/%_d.o)

TEST_SRC := $(TEST_ONLY_FILES) $(ADDITIONAL_TEST_FILES)
RELEASE_TEST_OBJECTS  := $(TEST_SRC:%.cpp=$(RELEASE_OBJ_DIR)/%.o)
DEBUG_TEST_OBJECTS  := $(TEST_SRC:%.cpp=$(DEBUG_OBJ_DIR)/%_d.o)

######################
# Targets start here #
######################

$(DEBUG_OBJ_DIR)/%_d.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(DEBUG_CXX) $(DEBUG_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_DEBUG_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(OUT_DIR)/$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	@mkdir -p $(@D)
	$(DEBUG_CXX) $(DEBUG_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_DEBUG_FLAGS) -o $(OUT_DIR)/$(DEBUG_TARGET) $^ $(CXX_SO_PARAMETERS) $(DEBUG_LDFLAGS)

$(RELEASE_OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(RELEASE_CXX) $(RELEASE_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS)  $(CXX_RELEASE_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(OUT_DIR)/$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	@mkdir -p $(@D)
	$(RELEASE_CXX) $(RELEASE_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_RELEASE_FLAGS) -o $(OUT_DIR)/$(RELEASE_TARGET) $^ $(CXX_SO_PARAMETERS) $(RELEASE_LDFLAGS)

$(DEBUG_OBJ_DIR)/$(TIDY_TARGET_NAME)_test_d.o: $(TIDY_TARGET_NAME)_test.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(DEBUG_CXX) $(DEBUG_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_DEBUG_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(OUT_DIR)/$(DEBUG_TEST_TARGET): $(DEBUG_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(DEBUG_CXX) $(DEBUG_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_DEBUG_FLAGS) -o $(OUT_DIR)/$(DEBUG_TEST_TARGET) $^ $(CXX_SO_PARAMETERS) $(DEBUG_LDFLAGS)

$(RELEASE_OBJ_DIR)/$(TIDY_TARGET_NAME)_test.o: $(TIDY_TARGET_NAME)_test.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(RELEASE_CXX) $(RELEASE_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS)  $(CXX_RELEASE_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(OUT_DIR)/$(RELEASE_TEST_TARGET): $(RELEASE_TEST_OBJECTS)
	@mkdir -p $(@D)
	$(RELEASE_CXX) $(RELEASE_MACRO_DEFINITIONS) $(CXX_BASE_FLAGS) $(CXX_RELEASE_FLAGS) -o $(OUT_DIR)/$(RELEASE_TEST_TARGET) $^ $(CXX_SO_PARAMETERS) $(RELEASE_LDFLAGS)

.PHONY: all rebuild_all create_folders debug release clean_all clean_debug clean_release rebuild_debug rebuild_release package_debug package_release package sense

create_debug_folders:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(DEBUG_OBJ_DIR)
	@mkdir -p $(DEBUG_PACKAGE_DIR)
	@if [ "${TARGET_IS_LIBRARY}" = "yes" ]; then \
		mkdir -p ${DEBUG_PACKAGE_LIB_DIR}; \
	fi
	@if [ "${INCLUDE_FILES_TO_PACKAGE}" != "" ]; then \
		mkdir -p ${DEBUG_PACKAGE_INCLUDE_DIR}; \
	fi

create_release_folders:
	@mkdir -p $(OUT_DIR)
	@mkdir -p $(RELEASE_OBJ_DIR)
	@mkdir -p $(RELEASE_PACKAGE_DIR)
	@if [ "${TARGET_IS_LIBRARY}" = "yes" ]; then \
		mkdir -p ${RELEASE_PACKAGE_LIB_DIR}; \
	fi
	@if [ "${INCLUDE_FILES_TO_PACKAGE}" != "" ]; then \
		mkdir -p ${RELEASE_PACKAGE_INCLUDE_DIR}; \
	fi

create_folders: create_release_folders create_debug_folders

release_prebuild: 
	@if [ "${TIDY_RELEASE_PRE_BUILD_STEP}" != "" ]; then \
		eval "cd ${CURRENT_PATH}/; ${TIDY_RELEASE_PRE_BUILD_STEP}" ; \
	fi	

debug_prebuild: 
	@if [ "${TIDY_DEBUG_PRE_BUILD_STEP}" != "" ]; then \
		eval "cd ${CURRENT_PATH}/; ${TIDY_DEBUG_PRE_BUILD_STEP}" ; \
	fi	

release_postbuild: 
	@if [ "${TIDY_RELEASE_POST_BUILD_STEP}" != "" ]; then \
		eval "cd ${CURRENT_PATH}/; ${TIDY_RELEASE_POST_BUILD_STEP}" ; \
	fi	

debug_postbuild: 
	@if [ "${TIDY_DEBUG_POST_BUILD_STEP}" != "" ]; then \
		eval "cd ${CURRENT_PATH}/; ${TIDY_DEBUG_POST_BUILD_STEP}" ; \
	fi	

debug: create_debug_folders debug_prebuild $(OUT_DIR)/$(DEBUG_TARGET) $(OUT_DIR)/$(DEBUG_TEST_TARGET) debug_postbuild

release: create_release_folders release_prebuild $(OUT_DIR)/$(RELEASE_TARGET) $(OUT_DIR)/$(RELEASE_TEST_TARGET) release_postbuild

clean_release: 
	-@rm -rvf $(RELEASE_OBJ_DIR)/*
	-@rm -rvf $(OUT_DIR)/$(RELEASE_TARGET)
	-@rm -rvf $(OUT_DIR)/$(RELEASE_TEST_TARGET)
	-@rm -rvf $(RELEASE_PACKAGE_DIR)/*

clean_debug: 
	-@rm -rvf $(DEBUG_OBJ_DIR)/*
	-@rm -rvf $(OUT_DIR)/$(DEBUG_TARGET)
	-@rm -rvf $(OUT_DIR)/$(DEBUG_TEST_TARGET)
	-@rm -rvf $(DEBUG_PACKAGE_DIR)/*

clean_all: 
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(OUT_DIR)/*
	-@rm -rvf $(PACKAGE_DIR)/*

all: release debug

rebuild_release: clean_release release

rebuild_debug: clean_debug debug

rebuild_all: rebuild_release rebuild_debug

package_release: rebuild_release
	@if [ "${TIDY_RELEASE_ADDITIONAL_PACKAGING}" != "" ]; then \
		eval "cd ${RELEASE_PACKAGE_DIR}/; ${TIDY_RELEASE_ADDITIONAL_PACKAGING}" ; \
	fi
	@if [ "${TARGET_IS_LIBRARY}" != "yes" ]; then \
		cp -rf $(OUT_DIR)/$(RELEASE_TARGET) $(RELEASE_PACKAGE_DIR)/$(RELEASE_TARGET); \
		cp -rf $(OUT_DIR)/$(RELEASE_TEST_TARGET) $(RELEASE_PACKAGE_DIR)/$(RELEASE_TEST_TARGET); \
		chmod a+x ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET}; \
		if [ "${RELEASE_LD_LIBRARY_PATH}" != "" ]; then \
			echo '#!/bin/sh' > ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET}.sh ; \
			echo export LD_LIBRARY_PATH=$(RELEASE_LD_LIBRARY_PATH):${LD_LIBRARY_PATH} >> ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET}.sh ; \
			echo  ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET} '$$@' >> ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET}.sh ; \
			chmod a+x ${RELEASE_PACKAGE_DIR}/${RELEASE_TARGET}.sh; \
		fi; \
	else \
			cp -rf $(OUT_DIR)/$(RELEASE_TARGET) $(RELEASE_PACKAGE_LIB_DIR)/$(RELEASE_TARGET); \
	fi
	@if [ "${INCLUDE_FILES_TO_PACKAGE}" != "" ]; then \
		cp -rf -t ${RELEASE_PACKAGE_INCLUDE_DIR}/ ${INCLUDE_FILES_TO_PACKAGE}; \
	fi

package_debug: rebuild_debug
	@if [ "${TIDY_DEBUG_ADDITIONAL_PACKAGING}" != "" ]; then \
		eval "cd ${DEBUG_PACKAGE_DIR}/; ${TIDY_DEBUG_ADDITIONAL_PACKAGING}" ; \
	fi
	@if [ "${TARGET_IS_LIBRARY}" != "yes" ]; then \
		cp -rf $(OUT_DIR)/$(DEBUG_TARGET) $(DEBUG_PACKAGE_DIR)/$(DEBUG_TARGET); \
		cp -rf $(OUT_DIR)/$(DEBUG_TEST_TARGET) $(DEBUG_PACKAGE_DIR)/$(DEBUG_TEST_TARGET); \
		chmod a+x ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET}; \
		if [ "${DEBUG_LD_LIBRARY_PATH}" != "" ]; then \
			echo '#!/bin/sh' > ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET}.sh ; \
			echo export LD_LIBRARY_PATH=$(DEBUG_LD_LIBRARY_PATH):${LD_LIBRARY_PATH} >> ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET}.sh ; \
			echo  ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET} '$$@' >> ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET}.sh ; \
			chmod a+x ${DEBUG_PACKAGE_DIR}/${DEBUG_TARGET}.sh; \
		fi; \
	else \
		cp -rf $(OUT_DIR)/$(DEBUG_TARGET) $(DEBUG_PACKAGE_LIB_DIR)/$(DEBUG_TARGET); \
	fi
	@if [ "${INCLUDE_FILES_TO_PACKAGE}" != "" ]; then \
		cp -rf -t ${DEBUG_PACKAGE_INCLUDE_DIR}/ ${INCLUDE_FILES_TO_PACKAGE}; \
	fi

package: package_release package_debug

sense:
	@echo "Files and Directories:"
	@echo "	[*] Output dir:         ${OUT_DIR}"
	@echo "	[*] Packages dir:       ${PACKAGE_DIR}"
	@echo "	[*] Release Object dir: ${RELEASE_OBJ_DIR}"
	@echo "	[*] Debug Object dir:   ${DEBUG_OBJ_DIR}"
	@echo "	[*] Release Target:     ${RELEASE_TARGET}"
	@echo "	[*] Debug Target:       ${DEBUG_TARGET}"
	@echo " "
	@echo "Build Targets:"
	@echo "	[*] rebuild_all:        Cleans and rebuilds release and debug."
	@echo "	[*] all:                Release and debug builds all at once."
	@echo "	[*] rebuild_release:    Cleans and rebuilds release build."
	@echo "	[*] release:            Release build."
	@echo "	[*] rebuild_debug:      Cleans and rebuilds debug build."
	@echo "	[*] debug:              Debug build."
	@echo "	[*] clean_all:          Cleans everything."
	@echo "	[*] clean_release:      Cleans release build artefacts."
	@echo "	[*] clean_debug:      	Cleans debug build artefacts."
	@echo "	[*] package:            Cleans, rebuilds, and packages everything for shipping."
	@echo "	[*] package_release:    Cleans, rebuilds, and packages release build for shipping."
	@echo "	[*] package_debug:      Cleans, rebuilds, and packages debug build for shipping."
	@echo "	[*] sense:              Makes sense."
	@echo "	"
	@echo "Packages are release and debug folders ready to ship."


