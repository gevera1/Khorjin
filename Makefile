<<<<<<< HEAD
LIBIEC_HOME=.

include make/target_system.mk

LIB_SOURCE_DIRS = src/mms/iso_acse
LIB_SOURCE_DIRS += src/mms/iso_acse/asn1c
LIB_SOURCE_DIRS += src/mms/iso_presentation/asn1c 
LIB_SOURCE_DIRS += src/mms/iso_presentation
LIB_SOURCE_DIRS += src/mms/iso_session
LIB_SOURCE_DIRS += src/common
LIB_SOURCE_DIRS += src/mms/asn1
LIB_SOURCE_DIRS += src/mms/iso_cotp
LIB_SOURCE_DIRS += src/mms/iso_mms/server
LIB_SOURCE_DIRS += src/mms/iso_mms/client
LIB_SOURCE_DIRS += src/mms/iso_client
LIB_SOURCE_DIRS += src/mms/iso_common
LIB_SOURCE_DIRS += src/mms/iso_mms/common
LIB_SOURCE_DIRS += src/mms/iso_mms/asn1c
LIB_SOURCE_DIRS += src/mms/iso_server
ifndef EXCLUDE_ETHERNET_WINDOWS
LIB_SOURCE_DIRS += src/goose
endif
LIB_SOURCE_DIRS += src/iec61850/client
LIB_SOURCE_DIRS += src/iec61850/common
LIB_SOURCE_DIRS += src/iec61850/server
LIB_SOURCE_DIRS += src/iec61850/server/model
LIB_SOURCE_DIRS += src/iec61850/server/mms_mapping
LIB_SOURCE_DIRS += src/iec61850/server/impl
ifeq ($(HAL_IMPL), WIN32)
LIB_SOURCE_DIRS += src/hal/socket/win32
LIB_SOURCE_DIRS += src/hal/thread/win32
LIB_SOURCE_DIRS += src/hal/ethernet/win32
LIB_SOURCE_DIRS += src/hal/filesystem/win32
LIB_SOURCE_DIRS += src/hal/time/win32
else ifeq ($(HAL_IMPL), POSIX)
LIB_SOURCE_DIRS += src/hal/socket/linux
LIB_SOURCE_DIRS += src/hal/thread/linux
LIB_SOURCE_DIRS += src/hal/ethernet/linux
LIB_SOURCE_DIRS += src/hal/filesystem/linux
LIB_SOURCE_DIRS += src/hal/time/unix
else ifeq ($(HAL_IMPL), BSD)
LIB_SOURCE_DIRS += src/hal/socket/bsd
LIB_SOURCE_DIRS += src/hal/thread/linux
LIB_SOURCE_DIRS += src/hal/ethernet/bsd
LIB_SOURCE_DIRS += src/hal/filesystem/linux
LIB_SOURCE_DIRS += src/hal/time/unix
endif
LIB_INCLUDE_DIRS +=	config
LIB_INCLUDE_DIRS += src/common/inc
LIB_INCLUDE_DIRS += src/mms/iso_mms/asn1c
LIB_INCLUDE_DIRS += src/mms/inc
LIB_INCLUDE_DIRS +=	src/mms/inc_private
LIB_INCLUDE_DIRS +=	src/hal/inc
LIB_INCLUDE_DIRS +=	src/goose
LIB_INCLUDE_DIRS += src/iec61850/inc
LIB_INCLUDE_DIRS += src/iec61850/inc_private
ifeq ($(HAL_IMPL), WIN32)
LIB_INCLUDE_DIRS += third_party/winpcap/Include
endif

LIB_INCLUDES = $(addprefix -I,$(LIB_INCLUDE_DIRS))

ifndef INSTALL_PREFIX
INSTALL_PREFIX = ./.install
endif

LIB_API_HEADER_FILES = src/hal/inc/hal_time.h 
LIB_API_HEADER_FILES += src/hal/inc/hal_thread.h
LIB_API_HEADER_FILES += src/hal/inc/hal_filesystem.h 
LIB_API_HEADER_FILES += src/common/inc/libiec61850_common_api.h
LIB_API_HEADER_FILES += src/common/inc/linked_list.h
LIB_API_HEADER_FILES += src/common/inc/byte_buffer.h
LIB_API_HEADER_FILES += src/common/inc/lib_memory.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_client.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_common.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_server.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_model.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_cdc.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_dynamic_model.h
LIB_API_HEADER_FILES += src/iec61850/inc/iec61850_config_file_parser.h
LIB_API_HEADER_FILES += src/mms/inc/mms_value.h
LIB_API_HEADER_FILES += src/mms/inc/mms_common.h
LIB_API_HEADER_FILES += src/mms/inc/mms_types.h
LIB_API_HEADER_FILES += src/mms/inc/mms_device_model.h
LIB_API_HEADER_FILES += src/mms/inc/mms_server.h
LIB_API_HEADER_FILES += src/mms/inc/mms_named_variable_list.h
LIB_API_HEADER_FILES += src/mms/inc/mms_type_spec.h
LIB_API_HEADER_FILES += src/mms/inc/mms_client_connection.h
LIB_API_HEADER_FILES += src/mms/inc/iso_connection_parameters.h
LIB_API_HEADER_FILES += src/mms/inc/iso_server.h
LIB_API_HEADER_FILES += src/mms/inc/ber_integer.h
LIB_API_HEADER_FILES += src/mms/inc/asn1_ber_primitive_value.h
LIB_API_HEADER_FILES += src/goose/goose_subscriber.h
LIB_API_HEADER_FILES += src/goose/goose_receiver.h

get_sources_from_directory  = $(wildcard $1/*.c)
get_sources = $(foreach dir, $1, $(call get_sources_from_directory,$(dir)))
src_to = $(addprefix $(LIB_OBJS_DIR)/,$(subst .c,$1,$2))

LIB_SOURCES = $(call get_sources,$(LIB_SOURCE_DIRS))

LIB_OBJS = $(call src_to,.o,$(LIB_SOURCES))

CFLAGS += -std=gnu99
#CFLAGS += -Wno-error=format 
CFLAGS += -Wstrict-prototypes

ifneq ($(HAL_IMPL), WIN32)
CFLAGS += -Wuninitialized 
endif

CFLAGS += -Wsign-compare 
CFLAGS += -Wpointer-arith 
CFLAGS += -Wnested-externs 
CFLAGS += -Wmissing-declarations 
CFLAGS += -Wshadow
CFLAGS += -Wall
#CFLAGS += -Werror  

all:	lib

static_checks:	lib
	splint -preproc +posixlib +skip-sys-headers +gnuextensions $(LIB_INCLUDES) $(LIB_SOURCES)

cppcheck:	lib
	cppcheck --force --std=c99 --enable=all $(LIB_INCLUDES) $(LIB_SOURCES) 2> cppcheck-output.xml

lib:	$(LIB_NAME)

dynlib: CFLAGS += -fPIC

dynlib:	$(DYN_LIB_NAME)

.PHONY:	examples

examples:
	cd examples; $(MAKE)

$(LIB_NAME):	$(LIB_OBJS)
	$(AR) r $(LIB_NAME) $(LIB_OBJS)
	$(RANLIB) $(LIB_NAME)
	
$(DYN_LIB_NAME):	$(LIB_OBJS)
	$(CC) $(LDFLAGS) $(DYNLIB_LDFLAGS) -shared -o $(DYN_LIB_NAME) $(LIB_OBJS) $(LDLIBS)

$(LIB_OBJS_DIR)/%.o: %.c config
	@echo compiling $(notdir $<)
	$(SILENCE)mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(LIB_INCLUDES) $(OUTPUT_OPTION) $<
	
install:	$(LIB_NAME)
	mkdir -p $(INSTALL_PREFIX)/include
	mkdir -p $(INSTALL_PREFIX)/lib
	cp $(LIB_API_HEADER_FILES) $(INSTALL_PREFIX)/include
	cp $(LIB_NAME) $(INSTALL_PREFIX)/lib

clean:
	rm -f $(EXAMPLES)
	rm -rf $(LIB_OBJS_DIR)
=======
# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/geverajet/libieee_c37_118

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/geverajet/libieee_c37_118/build

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components

.PHONY : list_install_components/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/geverajet/libieee_c37_118/build/CMakeFiles /home/geverajet/libieee_c37_118/build/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/geverajet/libieee_c37_118/build/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named c37_118_Receiver

# Build rule for target.
c37_118_Receiver: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 c37_118_Receiver
.PHONY : c37_118_Receiver

# fast build rule for target.
c37_118_Receiver/fast:
	$(MAKE) -f examples/c37_118_Receiver/CMakeFiles/c37_118_Receiver.dir/build.make examples/c37_118_Receiver/CMakeFiles/c37_118_Receiver.dir/build
.PHONY : c37_118_Receiver/fast

#=============================================================================
# Target rules for targets named ieeeC37_118

# Build rule for target.
ieeeC37_118: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 ieeeC37_118
.PHONY : ieeeC37_118

# fast build rule for target.
ieeeC37_118/fast:
	$(MAKE) -f src/CMakeFiles/ieeeC37_118.dir/build.make src/CMakeFiles/ieeeC37_118.dir/build
.PHONY : ieeeC37_118/fast

#=============================================================================
# Target rules for targets named ieeeC37_118-shared

# Build rule for target.
ieeeC37_118-shared: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 ieeeC37_118-shared
.PHONY : ieeeC37_118-shared

# fast build rule for target.
ieeeC37_118-shared/fast:
	$(MAKE) -f src/CMakeFiles/ieeeC37_118-shared.dir/build.make src/CMakeFiles/ieeeC37_118-shared.dir/build
.PHONY : ieeeC37_118-shared/fast

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... install/strip"
	@echo "... install"
	@echo "... install/local"
	@echo "... list_install_components"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... c37_118_Receiver"
	@echo "... ieeeC37_118"
	@echo "... ieeeC37_118-shared"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system
>>>>>>> b6c836dac759871d724fbcc522c6fe3a1f8a5bf8

