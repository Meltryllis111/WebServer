# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /workspaces/WebServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /workspaces/WebServer/build

# Include any dependencies generated for this target.
include CMakeFiles/WebServer.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/WebServer.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/WebServer.dir/flags.make

CMakeFiles/WebServer.dir/src/chatAPI.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/src/chatAPI.cpp.o: ../src/chatAPI.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/WebServer.dir/src/chatAPI.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/src/chatAPI.cpp.o -c /workspaces/WebServer/src/chatAPI.cpp

CMakeFiles/WebServer.dir/src/chatAPI.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/src/chatAPI.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /workspaces/WebServer/src/chatAPI.cpp > CMakeFiles/WebServer.dir/src/chatAPI.cpp.i

CMakeFiles/WebServer.dir/src/chatAPI.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/src/chatAPI.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /workspaces/WebServer/src/chatAPI.cpp -o CMakeFiles/WebServer.dir/src/chatAPI.cpp.s

CMakeFiles/WebServer.dir/src/http_conn.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/src/http_conn.cpp.o: ../src/http_conn.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/WebServer.dir/src/http_conn.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/src/http_conn.cpp.o -c /workspaces/WebServer/src/http_conn.cpp

CMakeFiles/WebServer.dir/src/http_conn.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/src/http_conn.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /workspaces/WebServer/src/http_conn.cpp > CMakeFiles/WebServer.dir/src/http_conn.cpp.i

CMakeFiles/WebServer.dir/src/http_conn.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/src/http_conn.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /workspaces/WebServer/src/http_conn.cpp -o CMakeFiles/WebServer.dir/src/http_conn.cpp.s

CMakeFiles/WebServer.dir/src/locker.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/src/locker.cpp.o: ../src/locker.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/WebServer.dir/src/locker.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/src/locker.cpp.o -c /workspaces/WebServer/src/locker.cpp

CMakeFiles/WebServer.dir/src/locker.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/src/locker.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /workspaces/WebServer/src/locker.cpp > CMakeFiles/WebServer.dir/src/locker.cpp.i

CMakeFiles/WebServer.dir/src/locker.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/src/locker.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /workspaces/WebServer/src/locker.cpp -o CMakeFiles/WebServer.dir/src/locker.cpp.s

CMakeFiles/WebServer.dir/src/main.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/WebServer.dir/src/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/src/main.cpp.o -c /workspaces/WebServer/src/main.cpp

CMakeFiles/WebServer.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /workspaces/WebServer/src/main.cpp > CMakeFiles/WebServer.dir/src/main.cpp.i

CMakeFiles/WebServer.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /workspaces/WebServer/src/main.cpp -o CMakeFiles/WebServer.dir/src/main.cpp.s

CMakeFiles/WebServer.dir/lib/inih/ini.c.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/lib/inih/ini.c.o: ../lib/inih/ini.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/WebServer.dir/lib/inih/ini.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/WebServer.dir/lib/inih/ini.c.o   -c /workspaces/WebServer/lib/inih/ini.c

CMakeFiles/WebServer.dir/lib/inih/ini.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/WebServer.dir/lib/inih/ini.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /workspaces/WebServer/lib/inih/ini.c > CMakeFiles/WebServer.dir/lib/inih/ini.c.i

CMakeFiles/WebServer.dir/lib/inih/ini.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/WebServer.dir/lib/inih/ini.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /workspaces/WebServer/lib/inih/ini.c -o CMakeFiles/WebServer.dir/lib/inih/ini.c.s

CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o: CMakeFiles/WebServer.dir/flags.make
CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o: ../lib/inih/INIReader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o -c /workspaces/WebServer/lib/inih/INIReader.cpp

CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /workspaces/WebServer/lib/inih/INIReader.cpp > CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.i

CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /workspaces/WebServer/lib/inih/INIReader.cpp -o CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.s

# Object files for target WebServer
WebServer_OBJECTS = \
"CMakeFiles/WebServer.dir/src/chatAPI.cpp.o" \
"CMakeFiles/WebServer.dir/src/http_conn.cpp.o" \
"CMakeFiles/WebServer.dir/src/locker.cpp.o" \
"CMakeFiles/WebServer.dir/src/main.cpp.o" \
"CMakeFiles/WebServer.dir/lib/inih/ini.c.o" \
"CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o"

# External object files for target WebServer
WebServer_EXTERNAL_OBJECTS =

WebServer: CMakeFiles/WebServer.dir/src/chatAPI.cpp.o
WebServer: CMakeFiles/WebServer.dir/src/http_conn.cpp.o
WebServer: CMakeFiles/WebServer.dir/src/locker.cpp.o
WebServer: CMakeFiles/WebServer.dir/src/main.cpp.o
WebServer: CMakeFiles/WebServer.dir/lib/inih/ini.c.o
WebServer: CMakeFiles/WebServer.dir/lib/inih/INIReader.cpp.o
WebServer: CMakeFiles/WebServer.dir/build.make
WebServer: /usr/lib/x86_64-linux-gnu/libcurl.so
WebServer: CMakeFiles/WebServer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/workspaces/WebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable WebServer"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/WebServer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/WebServer.dir/build: WebServer

.PHONY : CMakeFiles/WebServer.dir/build

CMakeFiles/WebServer.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/WebServer.dir/cmake_clean.cmake
.PHONY : CMakeFiles/WebServer.dir/clean

CMakeFiles/WebServer.dir/depend:
	cd /workspaces/WebServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /workspaces/WebServer /workspaces/WebServer /workspaces/WebServer/build /workspaces/WebServer/build /workspaces/WebServer/build/CMakeFiles/WebServer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/WebServer.dir/depend

