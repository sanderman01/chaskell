# What is CHaskell?
CHaskell is a little educational project where I experiment with calling Haskell code from a Visual Studio C++ project and vice versa, in preparation for a larger project.

Online information on embedding Haskell into C++ applications was often outdated and contradictory, so I made this project and repo as a ready made example with explanation in this readme, in case I ever forget.

I am writing this on Windows 10, for 64 Bit, with the following tools:

* Visual Studio 2015
* GHC 7.10.3 (Haskell Platform)
* Cabal 1.22.6.0.

This readme will mostly describe the compiling and building steps. The required sample code is inside the project.

The process described here will need adjustment if you intend to build on different platforms. Instead of haskell platform, you could probably also use stack.

This question and answer on Stack Overflow was very helpful:
[How can I integrate a Haskell DLL into a C++/Qt Windows application?](http://stackoverflow.com/questions/31438420/how-can-i-integrate-a-haskell-dll-into-a-c-qt-windows-application)

This GHC documentation page is also helpful. Note it seems a bit outdated: [GHC Users Guide - Win32-dlls]
(https://downloads.haskell.org/~ghc/latest/docs/html/users_guide/win32-dlls.html)


## Strategy

The first step in making our Haskell code available to a C or C++ project is to compile our Haskell code to a proper windows .dll. Then we can link against that .dll from our C/C++ project.

Building this .dll fromt the Haskell source is the part I initially had the most trouble with. Haskell and Windows tends to be a troublesome commbination for various reasons.

# Building the Cabal project

Beside our own functions, we also need to ensure that we link to the Haskell runtime in our dll.

This approach compiles all of the Haskell runtime and required packages all into the dll so we don't need to worry about the presence or absence of any local Haskell install on an end user's system. This means we can simply dump this dll into the program folder.

One issue here is that we need some way to initialize the Haskell runtime, before calling any Haskell functions, for that we need to add a little C header and compile it into our dll, along with the rest.

Since we will be compiling our Haskell code with GHC/Cabal, we do not need to provide their sources. We do need some configuration options in our .cabal project file so that it knows to link in our little c header containing the functions needed to start the Haskell runtime.

Finding the right .cabal configuration to include the c-file and all the rest into a .dll is a pain, but once we have it, building is easily done by running two commands from the project directory:

* *cabal exec -- ghc -optc-O -c src/StartEnd.c -o obj/StartEnd.o*
* *cabal build*

The first command is to create an object file from *Startend.c*.
The second command creates the actual dll with our Haskell code and also inserts the c code needed to start the haskell runtime. Why this process needs to be this convoluted, I do not know.

The build will create several important files:

* /dist/build/Hello_stub.h
* HSdll.dll
* HSdll.dll.a


See the .cabal file included in the Haskell project.

# Building the C or C++ project

Before building, it is important to specify the folders containing include files and libraries for your local ghc installation and of course your own haskell library (in cabal-project-dir/dist/build/)

## Includes

*Project Configuration Properties -> C/C++ -> General -> Additional Include Directories*

needs includes to GHC headers, and to our own C headers inside the haskell project. On my system, it the paths look like this:

* path\to\ghc\includes *(on my system it looks like this: C:\HaskellPlatform\7.10.3\lib\include)*
* ..\..\haskell\src
* ..\..\haskell\dist\build

## Libraries

It is also important to specify the library folder and library name for linking.

*Project Configuration Properties -> Linker -> General -> Additional Library Directories*

needs to point to the directory with object file we built in the previous step:

* ..\..\haskell

*Project Configuration Properties -> Linker -> Input -> Additional Dependencies*

needs to point to our final object:

* HSdll.dll.a

The this file describes the symbols like functions which are contained in the .dll file, and is used to link against the .dll. Cabal and GHC create this file with an .a extension. This is equivalent to a .lib on Windows.

# Build and Run

Once all the includes and linking options are set correctly, we can finally build our C/C++ project. *(Ctrl-Shift-B)*

Before we run our new executable though, we need to ensure that the .dll we linked against can be found by our program. There are two options here:

* Copy the .dll from the Haskell project to the working directory of the executable. (this could even be a post-build command in your project)
* Add the directory containing the .dll to our PATH variable.

The first option makes sense for distribution. For your own development system, adding to PATH may work better, since you don't need to bother with files being copied around.

Now run the executable, and if everything went right, then you should see console output from both the C and Haskell side.

#Final Notes

I find this project to be quite messy, with headers and object files created all over the place. There's got to be a better way.

The following Github issue suggests that soon there might be better support for building .dll files in the new Cabal 1.26 which should come out near the release of GHC 8:
[Add support for platform libraries #2540](https://github.com/haskell/cabal/pull/2540)