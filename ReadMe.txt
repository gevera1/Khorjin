Building Khorjin IEEE C37.118 with the cmake build script:
-----------------------------------------------

- Having cmake installed (https://cmake.org/download/), lunch a command line (cmd.exe).
- Create a new subdirectory in the Khorjin IEEE C37.118 folder (libieee_c37_118) and change to this subdirectory (empty "build" folder is already available).
- Invoke the "cmake" with the following format to build the Visual Studio solution:

cmake -G "Visual Studio 15" ..

The command line argument supplies the "generator" that is used by cmake to create the build files.

In this case, the cmake generates Visual Studio 2017 project files.

Similarly, In order to generate Visual Studio 2010 project files, the following line is lunched:

cmake -G "Visual Studio 10" ..

The ".." at the end of the command points to the parent directory.
It tells cmake where to find the main build script file (CMakeLists.txt).






















