# newgene

## Build Instructions for Windows

Part 1: Building NewGeneBackEnd.lib in Visual Studio 2017

1) Download Microsoft Visual Studio 2017

2) Download the Boost libraries from this web site: https://www.boost.org/users/download/. The boost_1_67_0.zip file should work.

3) Right click the zip file and click "Extract All". Note that it may take awhile for all of the files to be extracted.

4) Follow the instructions to build the full set of Boost libraries from this web site: https://www.boost.org/doc/libs/1_67_0/more/getting_started/windows.html. Section 5.1 should be sufficient. Note: the Boost libraries are buggy. It is possible that a file may be missing. If this is the case, you can try manually adding that file to the folder, re-downloading the most up-to-date version of Boost, or downloading the previous version of Boost.

5) Add the following environment variables to your Operating System (in Windows 10 go to Control Panel -> System and Security -> System -> Advanced System Settings -> Environment Variables): BOOST_ROOT and BOOST_LIB_MSVC12_X86. BOOST_ROOT should be set to the root Boost directory, and BOOST_LIB_MSVC12_X86 should be set to the directory containing the built Boost libraries.

6) Download the NewGene code from here: https://github.com/daniel347x/newgene. Note: You will need to be added to the project in order to make any commits. This can be done upon request.

7) In newgene/NewGeneBackEnd/Boost_Pool, you will find 3 C++ header files. These files should replace the 3 correspondingly named files from your downloaded Boost directory. These custom files had to be created due to the specific memory management requirements of NewGene.

8) In Visual Studio open NewGeneBackEnd.sln (located in newgene/NewGeneBackEnd).

9) Set the mode you wish to build from the top menu: either Debug or Release.

10) Click "Build -> Build Solution". This will build the file NewGeneBackEnd.lib.

Part 2: Building NewGene.exe in Qt Creator

1) Make sure the steps from Part 1 were successfully completed.

2) Download Qt Creator from the following web site: https://www.qt.io/download-qt-installer?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5

3) When installing Qt, make sure you select the MSVC 2015 32-bit compiler.

4) Once Qt is open, right click the Qt icon on the task bar, then right click the program and click "Run as administrator".

5) Click "Open Project", then open NewGene.pro (located in newgene/NewGeneUI/NewGene).

6) Make sure the build settings are set so that the MSVC 2015 32-bit compiler is selected.

7) Click the hammer in the lower-left corner to build NewGene.exe.

Part 3: Building NewGeneSetup.exe in DeployMaster

1) In order to build a Windows installer in the same fashion as is currently available on www.newgenesoftware.org, DeployMaster will be necessary. This software can be downloaded from www.deploymaster.com.

2) As a template, the NewGene.deploy file that I used to build the latest version of NewGene is located in the root of this repository. However, it will need to be modified to each user's individual paths.

3) The most difficult parts in this process are knowing which DLLs to include in the installer, and setting up the initial dataset to tie into the installer. However, if you already have a working installation of NewGene on your computer, the files that will need to be included in the installer are simply the files in your own installation folder. Similarly, you can use the files in the NewGene folder in your "Documents" folder (assuming this path wasn't modified after your installation of NewGene) as the input and output dataset files to tie to the installer.

4) (Optional but recommended) It is a good idea to clean up the dataset files by vacuuming the .db file first using a SQLite IDE such as SQLite Expert Professional.

## Build Instructions for Mac

Note: I wrote these instructions significantly after the last time I actually built NewGene on Mac. As such, they are less detailed than the Windows instructions and may be missing one or two steps. Please feel free to update this README with more details.

1) Download Boost directory, as done for the Windows build. Don't forget to replace the three appropriate C++ header files with the corresponding files from newgene/NewGeneBackEnd/Boost_Pool 

2) Add the following lines to your ~/.bash_profile file:

- export BOOST_ROOT="local_boost_path"
- export BOOST_LIB="local_boost_path/stage/lib" #(or whatever local path you have for Boost libraries)

3) Download the latest versions of Xcode and Qt Creator for Mac. Install Xcode with the following compiler:

- Apple LLVM version 7.0.0 (clang-700.0.57.2)
- Target: x86_64-apple-darwin17.5.0
- Thread model: posix

4) Clone this repository on your Mac.

5) Open NewGene.xcodeproj in Xcode.

6) There are a number of hard-coded paths in the build settings that will need to be updated in order for the build to work on your computer. I may be missing some here, but I think there are a few places where you can define "HEADER_SEARCH_PATHS". Anywhere that says "/Users/daniel347x/boost_1_62_0" should be changed to you local Boost directory path.

7) Build the solution in Xcode.

8) Follow the instructions from the Windows portion of this manual to build the final solution in Qt Creator.

9) Run the macinstaller.sh script from the command line in order to build the NewGene installer for Mac. Note that there are a number of hard-coded paths in this script which you will have to change appropriately. Also, this script uses the command line tools "pkgbuild" and "productbuild". I believe that these command line tools are installed automatically with Xcode.
