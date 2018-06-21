newgene
=======

Build Instructions for Windows:

Part 1: Building NewGeneBackEnd.lib in Visual Studio 2017

1) Download Microsoft Visual Studio 2017

2) Download the Boost libraries from this web site: https://www.boost.org/users/download/. The boost_1_67_0.zip file should work.

3) Right click the zip file and click "Extract All". Note that it may take awhile for all of the files to be extracted.

4) Follow the instructions to build the full set of Boost libraries from this web site: https://www.boost.org/doc/libs/1_67_0/more/getting_started/windows.html. Section 5.1 should be sufficient. Note: the Boost libraries are buggy. It is possible that a file may be missing. If this is the case, you can try manually adding that file to the folder, re-downloading the most up-to-date version of Boost, or downloading the previous version of Boost.

5) Add the following environment variables to your Operating System (in Windows 10 go to Control Panel -> System and Security -> System -> Advanced System Settings -> Environment Variables): BOOST_ROOT and BOOST_LIB_MSVC12_X86. BOOST_ROOT should be set to the root Boost directory, and BOOST_LIB_MSVC12_X86 should be set to the directory containing the built Boost libraries.

6) Download the NewGene code from here: https://github.com/daniel347x/newgene.

7) In Visual Studio open NewGeneBackEnd.sln (located in newgene/NewGeneBackEnd).

8) Set the mode you wish to build from the top menu: either Debug or Release.

9) Click "Build -> Build Solution". This will build the file NewGeneBackEnd.lib.

Part 2: Building NewGene.exe in Qt Creator

1) Make sure the steps from Part 1 were successfully completed.

2) Download Qt Creator from the following web site: https://www.qt.io/download-qt-installer?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5

3) When installing Qt, make sure you select the MSVC 2015 32-bit compiler.

4) Once Qt is open, right click the Qt icon on the task bar, then right click the program and click "Run as administrator".

5) Click "Open Project", then open NewGene.pro (located in newgene/NewGeneUI/NewGene).

6) Make sure the build settings are set so that the MSVC 2015 32-bit compiler is selected.

7) Click the hammer in the lower-left corner to build NewGene.exe.

Part 3: Building NewGeneSetup.exe in DeployMaster