# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /home/ch/SOFTWARES/Clion/clion-2020.2.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/ch/SOFTWARES/Clion/clion-2020.2.4/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ch/code/CSAPP_LAB/TinyWeb

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/TinyWeb.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/TinyWeb.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TinyWeb.dir/flags.make

CMakeFiles/TinyWeb.dir/tinyweb.c.o: CMakeFiles/TinyWeb.dir/flags.make
CMakeFiles/TinyWeb.dir/tinyweb.c.o: ../tinyweb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/TinyWeb.dir/tinyweb.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TinyWeb.dir/tinyweb.c.o   -c /home/ch/code/CSAPP_LAB/TinyWeb/tinyweb.c

CMakeFiles/TinyWeb.dir/tinyweb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TinyWeb.dir/tinyweb.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ch/code/CSAPP_LAB/TinyWeb/tinyweb.c > CMakeFiles/TinyWeb.dir/tinyweb.c.i

CMakeFiles/TinyWeb.dir/tinyweb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TinyWeb.dir/tinyweb.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ch/code/CSAPP_LAB/TinyWeb/tinyweb.c -o CMakeFiles/TinyWeb.dir/tinyweb.c.s

CMakeFiles/TinyWeb.dir/csapp.c.o: CMakeFiles/TinyWeb.dir/flags.make
CMakeFiles/TinyWeb.dir/csapp.c.o: ../csapp.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/TinyWeb.dir/csapp.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TinyWeb.dir/csapp.c.o   -c /home/ch/code/CSAPP_LAB/TinyWeb/csapp.c

CMakeFiles/TinyWeb.dir/csapp.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TinyWeb.dir/csapp.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ch/code/CSAPP_LAB/TinyWeb/csapp.c > CMakeFiles/TinyWeb.dir/csapp.c.i

CMakeFiles/TinyWeb.dir/csapp.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TinyWeb.dir/csapp.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ch/code/CSAPP_LAB/TinyWeb/csapp.c -o CMakeFiles/TinyWeb.dir/csapp.c.s

# Object files for target TinyWeb
TinyWeb_OBJECTS = \
"CMakeFiles/TinyWeb.dir/tinyweb.c.o" \
"CMakeFiles/TinyWeb.dir/csapp.c.o"

# External object files for target TinyWeb
TinyWeb_EXTERNAL_OBJECTS =

TinyWeb: CMakeFiles/TinyWeb.dir/tinyweb.c.o
TinyWeb: CMakeFiles/TinyWeb.dir/csapp.c.o
TinyWeb: CMakeFiles/TinyWeb.dir/build.make
TinyWeb: CMakeFiles/TinyWeb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable TinyWeb"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TinyWeb.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TinyWeb.dir/build: TinyWeb

.PHONY : CMakeFiles/TinyWeb.dir/build

CMakeFiles/TinyWeb.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/TinyWeb.dir/cmake_clean.cmake
.PHONY : CMakeFiles/TinyWeb.dir/clean

CMakeFiles/TinyWeb.dir/depend:
	cd /home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ch/code/CSAPP_LAB/TinyWeb /home/ch/code/CSAPP_LAB/TinyWeb /home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug /home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug /home/ch/code/CSAPP_LAB/TinyWeb/cmake-build-debug/CMakeFiles/TinyWeb.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TinyWeb.dir/depend
