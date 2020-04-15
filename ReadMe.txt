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


<<<<<<< HEAD

















=======
Instructions for Linux:
----------------------------------------------

- Go to empty "build" directory in the Khorjin IEEE C37.118 folder (libieee_c37_118)
- Invoke cmake by accessing your terminal (ensure you are in the correct directory, listed above) and type in the following:

cmake ..

- The code should automatically be compiled using gcc

The ".." at the end of the command points to the parent directory.
It tells cmake where to find the main build script file (CMakeLists.txt).

- If you wish to make changes to the cache, go to the directory with the Unix Makefile and type the following:

ccmake OR cmake-gui

- If you run into errors, make sure you have ccmake installed. To do so, simply type in:

sudo apt-get install cmake-curses-gui

- After finalizing everything in your project, go to the build directory and type in: 

make
>>>>>>> b6c836dac759871d724fbcc522c6fe3a1f8a5bf8



