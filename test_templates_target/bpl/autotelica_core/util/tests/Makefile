CXX      := -c++
CXX_IGNORED_WARNINGS := -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function -Wno-ignored-qualifiers 
CXX_BASE_FLAGS :=  -std=c++14 -fdiagnostics-color=always -Wall -Wextra -Werror $(CXX_IGNORED_WARNINGS)
CXX_DEBUG_FLAGS := -DDEBUG -g
CXX_RELEASE_FLAGS := -O2

LDFLAGS  := -L/usr/lib -lstdc++ -lm

BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
RELEASE_OBJ_DIR  := $(OBJ_DIR)/release
DEBUG_OBJ_DIR  := $(OBJ_DIR)/debug
APP_DIR  := $(BUILD)/apps
PACKAGE_DIR  := $(BUILD)/packages
RELEASE_PACKAGE_DIR  := $(PACKAGE_DIR)/release/
DEBUG_PACKAGE_DIR  := $(PACKAGE_DIR)/debug/
RELEASE_TARGET   := test_runner
DEBUG_TARGET   := test_runner_d
INCLUDE  := -I../include/
SRC      :=                      \
   $(wildcard *.cpp)         \

HEADERS := $(wildcard *.h)

RELEASE_OBJECTS  := $(SRC:%.cpp=$(RELEASE_OBJ_DIR)/%.o)
DEBUG_OBJECTS  := $(SRC:%.cpp=$(DEBUG_OBJ_DIR)/%_d.o)

$(DEBUG_OBJ_DIR)/%_d.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CXX_PER_BULD_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $(APP_DIR)/$(DEBUG_TARGET) $^ $(LDFLAGS)

$(RELEASE_OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CXX_PER_BULD_FLAGS) $(INCLUDE) -c $< -MMD -o $@

$(APP_DIR)/$(RELEASE_TARGET): $(RELEASE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(CXX_RELEASE_FLAGS) -o $(APP_DIR)/$(RELEASE_TARGET) $^ $(LDFLAGS)

.PHONY: all rebuild_all create_folders debug release clean_all clean_debug clean_release rebuild_debug rebuild_release package_debug package_release package sense

create_folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(RELEASE_OBJ_DIR)
	@mkdir -p $(DEBUG_OBJ_DIR)
	@mkdir -p $(RELEASE_PACKAGE_DIR)
	@mkdir -p $(DEBUG_PACKAGE_DIR)

debug: create_folders $(APP_DIR)/$(DEBUG_TARGET)

release: create_folders $(APP_DIR)/$(RELEASE_TARGET)

clean_release: create_folders
	-@rm -rvf $(RELEASE_OBJ_DIR)/*
	-@rm -f $(APP_DIR)/$(RELEASE_TARGET)
	-@rm -f $(RELEASE_PACKAGE_DIR)/*

clean_debug: create_folders
	-@rm -rvf $(DEBUG_OBJ_DIR)/*
	-@rm -f $(APP_DIR)/$(DEBUG_TARGET)
	-@rm -f $(DEBUG_PACKAGE_DIR)/*

clean_all: 
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(PACKAGE_DIR)/*

all: release debug

rebuild_release: clean_release release

rebuild_debug: clean_debug debug

rebuild_all: rebuild_release rebuild_debug

package_release: rebuild_release
	@cp -rf $(APP_DIR)/$(RELEASE_TARGET) $(RELEASE_PACKAGE_DIR)

package_debug: rebuild_release
	@cp -rf $(APP_DIR)/$(DEBUG_TARGET) $(DEBUG_PACKAGE_DIR)

package: rebuild_all
	@cp -rf $(APP_DIR)/$(RELEASE_TARGET) $(RELEASE_PACKAGE_DIR)
	@cp -rf $(APP_DIR)/$(DEBUG_TARGET) $(DEBUG_PACKAGE_DIR)

sense:
	@echo "Files and Directories:"
	@echo "	[*] Application dir:    ${APP_DIR}"
	@echo "	[*] Release Object dir: ${RELEASE_OBJ_DIR}"
	@echo "	[*] Debug Object dir:   ${DEBUG_OBJ_DIR}"
	@echo "	[*] Release Target:     ${RELEASE_TARGET}"
	@echo "	[*] Debug Target:       ${DEBUG_TARGET}"
	@echo "	[*] Sources:            ${SRC}"
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

