# DLSS-Finder
A small application helping to unlock DLSS/DLSSG functionality in Streamline on AMD/Intel GPU cards

## Usage
Drop the dlss-finder.exe file into the game directory where the main game executable file is and launch it. Application will start minimized, silently perform the operations described below and close itself. 

## Functionality
DLSS Finder will try to find the nvngx_dlss.dll file in the game directory and copy it as nvngx.dll file. If it cannot find the original file, it will look for it in up to three directories (and their subdirectories) above. This however will be done only if dlss-finder.exe has been launched from [..]Binaries/Win64 directory (which is a default directory structure for Unreal Engine 4 and 5 games).

In case of any error, DLSS Finder will show an error message explaining the problem (you can start the app with /q commandline argument to hide the error messages).

In case of success, no message will appear (unless you start the app with /s commandline argument), and the application will close itself.

_The behavior described above is particulary suitable for various installer applications that might want to run dlss-finder.exe during or after the installation process_

## Compilation

This code can be easily compiled by opening the Visual Studio x64 command prompt, navigating to the "source" directory and launching the `nmake` command. Executable file will be created in "build" directory.

To clear the project, you can use `nmake clean` . It will delete the content of "build" directory.
