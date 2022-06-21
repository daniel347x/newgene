# newgene

## Build Instructions for Windows

Part 1: Building NewGeneBackEnd.lib in Visual Studio 2017

1) Download Microsoft Visual Studio 2017

2) Download the Boost libraries from this web site: https://www.boost.org/users/download/. The boost_1_67_0.zip file should work.

3) Right click the zip file and click "Extract All". Note that it may take awhile for all of the files to be extracted. Make sure that the file path that these are extracted into does not have spaces (ex. of good file path: C:/NewGene/...).

4) Follow the instructions to build the full set of Boost libraries from this web site: https://www.boost.org/doc/libs/1_67_0/more/getting_started/windows.html. Section 5.1 should be sufficient. Note: the Boost libraries are buggy. It is possible that a file may be missing. If this is the case, you can try manually adding that file to the folder, re-downloading the most up-to-date version of Boost, or downloading the previous version of Boost.
  i. Check that the built Boost libraries have vc-141 in their names. You may need to uninstall later versions of Visual Studio.

5) Add the following environment variables to your Operating System (in Windows 10 go to Control Panel -> System and Security -> System -> Advanced System Settings -> Environment Variables): BOOST_ROOT and BOOST_LIB_MSVC12_X86. BOOST_ROOT should be set to the root Boost directory, and BOOST_LIB_MSVC12_X86 should be set to the directory containing the built Boost libraries.

6) Download the NewGene code from here: https://github.com/daniel347x/newgene. Note: You will need to be added to the project in order to make any commits. This can be done upon request.

7) In newgene/NewGeneBackEnd/Boost_Pool, you will find 3 C++ header files. These files should replace the 3 correspondingly named files from your downloaded Boost directory. These custom files had to be created due to the specific memory management requirements of NewGene.

8) In Visual Studio open NewGeneBackEnd.sln (located in newgene/NewGeneBackEnd).

9) Set the mode you wish to build from the top menu (most likely Release).

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

1) In order to build a Windows installer in the same fashion as is currently available on www.newgenesoftware.org, DeployMaster will be necessary. This software can be downloaded from www.deploymaster.com. Note: the DeployMaster manual (https://www.deploymaster.com/manual/DeployMaster.pdf pg. 45-48) is very useful. You may need to download NewGene from the website to know which folders in the Files tab on DeployMaster the files should go.

2) As a template, the NewGene.deploy file that I used to build the latest version of NewGene is located in the root of this repository. However, it will need to be modified to each user's individual paths.

3) The most difficult parts in this process are knowing which DLLs to include in the installer, and setting up the initial dataset to tie into the installer. However, if you already have a working installation of NewGene on your computer, the files that will need to be included in the installer are simply the files in your own installation folder. Similarly, you can use the files in the NewGene folder in your "Documents" folder (assuming this path wasn't modified after your installation of NewGene) as the input and output dataset files to tie to the installer.

4) (Optional but recommended) It is a good idea to clean up the dataset files by vacuuming the .db file first using a SQLite IDE such as SQLite Expert Professional.

## Build Instructions for Mac

NOTE: NewGene was built using Xcode 7, and cannot be built using newer versions of Xcode without tightening up the code to meet the standards of more modern compilers. Unfortunately, Xcode 7 does not work on newer versions of macOS (after 10.13 I believe). Therefore, the only way to build NewGene for Mac if you have the most up-to-date macOS would be to use a cloud hosting service such as MacStadium, or update the code to be compliant with newer versions of Xcode.

1) Download Boost directory, as done for the Windows build. Don't forget to replace the three appropriate C++ header files with the corresponding files from newgene/NewGeneBackEnd/Boost_Pool 

2) Add the following lines to your ~/.bash_profile file:

- export BOOST_ROOT="local_boost_path"
- export BOOST_LIB="local_boost_path/stage/lib" #(or whatever local path you have for Boost libraries)

3) Download Xcode 7 and the latest version of Qt Creator for Mac that works on your operating system.

4) Clone this repository on your Mac.

5) Open NewGene.xcodeproj in Xcode.

6) There are a number of hard-coded paths in the build settings that will need to be updated in order for the build to work on your computer. Anywhere that says "/Users/daniel347x/boost_1_62_0" should be changed to you local Boost directory path.

7) Build the solution in Xcode. Make sure you have configured the settings to build the Release version.

8) Follow the instructions from the Windows portion of this manual to build the final solution in Qt Creator. Note that you may also have to add BOOST_ROOT and BOOST_LIB to the "Bulid Environment" (or "System Environment") portion of the project settings. The value for these variables should be the same as what was added to the bash_profile. Also make sure to build the Release version.

9) Navigate in Finder to the new build of NewGene.app, right-click on it and click "Show Package Contents". Now, download and install the latest version of NewGene from newgenesoftware.org and do the same. You will notice that the application that you built will be missing a number of dylibs and Frameworks from Qt. By navigating to the Qt directory on your computer, you will find these dylibs and Frameworks, which you should copy & paste into the package in the same manner that they are included in the build from the application of NewGene that you downloaded.

10) Run the macinstaller.sh script from the command line in order to build the NewGene installer for Mac. Note that there are a number of hard-coded paths in this script which you will have to change appropriately. Also, this script uses the command line tools "pkgbuild" and "productbuild", which are installed automatically with Xcode.

## Rundown on Interaction between Backend and UI in NewGene

c/o Dan

There are two main points about Qt and NewGene - one of which is related to just Qt itself, and the other of which is related to how I wrote the code to integrate Qt into the Visual Studio-centered code.

1) Qt is based on a 'signal-slot' architecture.  I recommend reading up on some Qt documentation for a runthrough.

The bottom line is that to do anything in Qt, you create 'signal handlers', called 'slots', in one place in the code, and elsewhere in the code, you 'send signals'.  You rarely call actual functions directly.  That's why you don't see 'ReceiveSignalSetRunStatus' called anywhere.  It's not.  (Well, actually, of course it is - but it's called from deep inside the Qt source code via a function pointer, never to be seen by mere mortals unless you have need to debug deep down into the Qt source code.)  The initialization routines for the objects in question set up the signal/slot connection (i.e., the above function acting as slot) to always be called whenever the given signal is sent.  And, a signal is sent from the NewGene source code somewhere that activates the slot.

The place to look, therefore, is for the initialization of the signals and slots, and then for where the signal is actually sent.

(Side comment: To get the signal/slot architecture working, Qt Creator automates a preprocessing stage, called the 'MOC compiler', which automatically generates a bunch of almost-not-readable C++ code from the C++ code that you edit at the GUI layer in Qt Creator.  This preprocessing stage is pretty heavy-hitting and so to avoid confusion when you see random, odd-looking source code files lying around you should be aware of this.)

2) The next tricky part is how I've tied together the Qt Creator GUI layer with the Visual Studio back-end algorithmic layer (a separate statically-linked library).

For this, I almost have to apologize.  Although the code is rock-solid and tight, that came with a pretty hefty sacrifice that I made in order to save a pretty serious amount of time and yet ensure that I could use Visual Studio for the algorithmic programming and only use Qt Creator for the GUI programming: code readability.

Frankly, at the time I wrote that code, C++ was just coming out of a decade of 'template hell' in which programmers (like myself) overused template programming - pretty much feasting off the fact that when you use templates heavily (and well) the code tends to be free of bugs, which is a very good thing.

However, like so many others, time has shown me that heavy template-based code is nearly unreadable.

So again, apologies that you have to deal with it.

But a quick rundown.  The connection between the GUI layer (in Qt Creator) and the back end code (in Visual Studio) has two major aspects.

(a) There is a base class (I forget the name - you should see pointers to these things all over the place), an instance of which is passed to all of the Visual Studio functions FROM the Qt Creator functions.  (Note: the Qt Creator functions have access to all the Visual Studio functions, it being a library, but not the other way around.)

Instances of this base class object contain no GUI code, no Qt objects, and no platform-specific code.  All Visual Studio sees is a pointer to instances of this base class.

The base class objects contain all necessary data to tell Visual Studio functions what to do.  Additionally, they also contain a set of virtual functions.  The magic here is at the heart of OO programming: The pointers passed to Visual Studio, though they are pointers to these base class objects, are ACTUALLY pointing to derived-class objects.  Whenever the Visual Studio functions call these virtual member functions, unbeknownst to the Visual Studio functions, it's really the derived-class override of the virtual functions that are called.  These derived-class overrides are defined in Qt Creator and do have access to Qt objects, the GUI layer and platform-specific code.

Bottom line: There are various virtual functions that, when called, trigger virtual functions that, in turn, access the Qt layer, which then emit signals, that then call slots via the signal-slot architecture, and it is these slots that actually pop up message boxes and dialog boxes.

(b) On top of all of the above is the 'template hell'.  In order to unify the many pieces of data that get passed to and fro into a single design (so that, for example, error handling and logging automatically happens regardless of the data that is passed), I tied many of the message-wrapping and data-wrapping classes into a template hierarchy (so that they share the same code, despite different actual message types and data types).

What this means is that while following through messages and data that get passed around (in particular - messages FROM the Qt layer TO the Visual Studio layer - but the reverse as well in some cases) you will run into template classes.  The template classes do not modify the overall architecture that is described above, BUT they add 2, 3 or even 4 layers of indirection so that to understand what ACTUAL class you're dealing with, you need to open up 2, 3 or 4 files at the same time, hold a bunch of stuff in your head and march through the files piecing together the layers of indirection until you finally envision the runtime class.

Yes, the above is a big hassle.  Yes, I apologize about the template hell.  My excuse is that an entire generation of programmers fell into a similar trap.  Templates do have their place, and I am happy with the fact that NewGene is rather bug-free (in large part due to the heavy use of templates), but if I had it to do over again, I would never in a million years use templates the way I did in this source code.

(Heads up that perhaps the most obnoxious and intractable of all the template code involves the way that user actions such as button clicks are passed through to the back end - there really is template hell here.  On the up side, it's ready to be heavily parallelized using this architecture in a future version which will dramatically improve performance for some datasets - unless the entire template-based code base is just dumped and rewritten.)

## Rundown on the Algorithm

c/o Dan

Please follow up with Paul and clarify with him the Unit of Analysis (UOA) - and note that it consists of one or more Decision Making Units (DMUs).  I.e., you could have a Country-Country-NGO-Senator UOA, which is a unit of analysis consisting of the Country DMU appearing twice, the NGO DMU appearing once, and the Senator DMU appearing once.  Note that the same DMU can appear more than once in a UOA.  The primary keys are nothing more than the set of DMUs in the UOA.

Note that the primary keys correspond to a set of COLUMNS that MUST be available to appear in the output.  (Note: Heretofore, "primary key" (in singular) should be taken to mean the set of primary keys for a given run, as defined above.)

Digging deeper into how the results associate specific DMU values to a particular row of output for a given primary key, we immediately see that different rows (for the same time slice) correspond to different DMU values for the same primary key (for example, one row might be USA-USA-Unicef-Sanders, and another row might be USA-Canada-Unicef-Cruz).  The primary keys are Country-Country-NGO-Senator but there are two rows with different sets of values.

To distinguish these two rows in general terms, we say that these two rows correspond to different branches.  The first branch is "USA-USA-Unicef-Sanders", and the second branch is "USA-Canada-Unicef-Cruz" in the above example.

Leaves are trickier.  Note that the above example was the simplest case given the scenario where the UOA is Country-Country-NGO-Senator - it is the simplest case because the above example assumes that the user chose 2 for the Country spinner, 1 for the NGO spinner, and 1 for the Senator spinner in the Kad selector section.

But let's make that example more complicated.  Suppose the user chooses 4 for the Country spinner.  In this case, note that for a single pair of values for the NGO and Senator DMUs (say, "Unicef-Sanders"), it is actually possible to have more than one row of output.

For example:

"USA-USA USA-Canada Unicef Sanders"
"USA-USA France-Russia Unicef Sanders"
"USA-USA USA-USA Unicef Sanders"
"France-Russia France-Russia Unicef Sanders"
"USA-Canada USA-Canada Unicef Sanders"
"USA-Canada France-Russia Unicef Sanders"

In the above example, we say that the (outer) multiplicity of the Country-Country part of the primary key is 2, while it is 1 for both NGO and Senator.

(Of note: the 'inner' multiplicity in this same example is 4 for Country, but the most important value of 'multiplicity' is the 'outer' multiplicity, and when you see 'multiplicity' in the source code it usually means outer multiplicity, but you have to look at the context to see which version of multiplicity is intended.)

Now we can return to leaves.  A useful way to describe the above scenario, where some of the DMUs have multiplicity 1 and others have multiplicity greater than 1 (in the above example, 2), we say that the set of rows in the result set with the same set of values for those DMUs with multiplicity 1 correspond to one branch, and the remaining values in the primary key across these rows, which vary, correspond to leaves.

Let's say that the output in the above example in fact has only the six rows noted above for the branch "Unicef Sanders".  In this case, we say that the leaves consist of the three pairs "USA-USA", "USA-Canada" and "France-Russia".

(Note: I have deliberately chosen the complex case where the outer multiplicity is DIFFERENT than the inner multiplicity.  I could just as easily have chosen the primary key to be COUNTRY-NGO-SENATOR (only one COUNTRY column) and then given an example with the Kad spinner set to 2 for Country and three leaves consisting of USA, Canada and Russia, with 6 rows of output corresponding to all possible combinations of these leaves.  In this case the outer multiplicity equals the inner multiplicity - 2.  But - because the more complex case is possible and more difficult to understand, I am using that as an example.  In this more complex case each pair should be considered a single, unbreakable entity - i.e., USA-USA is a single entity that always appears together as one "unit" of multiplicity, and there are three such pairs, or three leaves in the above example.)

To recap, the way the algorithm works is as follows.

1) Determine all branches that should appear in the output.

(Example: Unicef-Sanders is one branch, Unicef-Cruz is another branch, WWF-Cruz is a third branch, etc.)

2) For every branch, calculate the leaves available to construct output rows for the given time slice, given the dataset being used.

(Example above: For the branch Unicef-Sanders, there are three available leaves - USA-USA USA-Canada, and France-Russia.)

3) Determine all combinations of leaves given the outer multiplicity.

(Example above: For multiplicity 2, there are 6 combinations of 3 leaves, including 3 self-dyads, as seen above.)

4) Construct the output rows for the branch.

(Example above: There are 6 rows for this branch.  All 6 have Unicef-Sanders, and the 6 rows correspond to the 6 combinations of leaves noted.  For each row, the secondary fields must then be filled in from the so-called "leaf cache" (i.e., there are other columns of output besides the primary key columns)

5) Move on to the next branch and proceed with Step 2.

Note in point 4 an important part of the algorithm: the secondary columns - these require a leaf cache that you might see noted in the source code.
