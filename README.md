# What is CHaskell?
CHaskell is a little educational project where I experiment with calling Haskell code from a Visual Studio C++ project and vice versa, in preparation for a larger project.

Online information on embedding Haskell into C++ applications was often outdated and contradictory, especially for Windows systems, so I made these projects and this repo as a ready made example with explanation in this readme, in case I ever forget.

I am writing this on Windows 10, for 64 Bit, with the following tools:

* Visual Studio 2015
* GHC 7.10.3 (Haskell Platform)
* Cabal 1.22.6.0
* Stack 1.0.4.3

This readme will mostly describe the compiling and building steps. The sample code is inside the *haskell* and *main* directories.

This question and answer on Stack Overflow was very helpful for me when I was struggling to figure this stuff out:
[How can I integrate a Haskell DLL into a C++/Qt Windows application?](http://stackoverflow.com/questions/31438420/how-can-i-integrate-a-haskell-dll-into-a-c-qt-windows-application)

This GHC documentation page is also helpful. Note it seemed a bit outdated at time of writing: [GHC Users Guide - Win32-dlls]
(https://downloads.haskell.org/~ghc/latest/docs/html/users_guide/win32-dlls.html)


## Strategy

The first step in making our Haskell code available to a C or C++ project is to compile our Haskell code to a proper windows .dll. Then we can link against that .dll from our C/C++ project.

Building this .dll fromt the Haskell source is the part I initially had the most trouble with. Haskell and Windows tends to be a troublesome combination for various reasons.

# Building the Cabal project

Beside our own functions, we also need to ensure that we link to the Haskell runtime in our dll.

This approach compiles all of the Haskell runtime and required packages all into the dll so we don't need to worry about the presence or absence of any local Haskell install on an end user's system. We can simply dump this dll into the program folder.

One issue here is that we need some way to initialize the Haskell runtime, before calling any Haskell functions, for that we need to add a little C header and compile it into our dll, along with the rest. Since this is C code, we need to compile this header to an object so we can link it into the .dll we will make later.

Since we will be compiling our Haskell code with GHC/Cabal, we do not need to provide their sources. We do need some configuration options in our *.cabal* project file so that it knows to link in our little c header containing the functions needed to start the Haskell runtime.

Finding the right .cabal configuration to include the c-file and all the rest into a .dll is a pain, but once we have it, building is easily done by running two commands from the project directory:

* `cabal exec -- ghc -optc-O -c cbits/StartEnd.c -o obj/StartEnd.o`
* `cabal build`

Or the equivalent using stack:

* `stack exec -- ghc -optc-O -c cbits/StartEnd.c -o obj/StartEnd.o`
* `stack build`

The first command is to create an object file from *Startend.c*.
The second command creates the actual dll with our Haskell code and also inserts the c code in the object file which is needed to start the haskell runtime. 
Cabal knows to include this into the final .dll due to the following options in our *.cabal* file in addition to the normal sources. Take note of these:

* `c-sources:           cbits/StartEnd.c`
* `ghc-options:         -O1 -shared obj/StartEnd.o -o bin/HaskellLib.dll -stubdir stub`

The build will create several important files:

* `/stub/Hello_stub.h`
* `/bin/HaskellLib.dll`
* `/bin/HaskellLib.a`

Hello_stub.h will contain type signatures for functions in our haskell modules. We will need to include this in our c/c++ project.  
The HaskellLib.dll file now contains both our newly written haskell module and the haskell runtime, so it results in a relatively large, standalone library, which we can distribute together with our main program.
This means users will not need to have the entire haskell toolchain installed.
The HaskellLib.a file contains symbol information needed by the c/c++ compiler to link against this .dll.

### Stack
It took me a while to figure out how to build correctly, using stack. My problem was that most of the build seemed to work just fine, but the final .dll and .dll.a files were not being created for some reason.
The issue turned out to be very simple. I was using the following line in my cabal file.  
`ghc-options:         -O1 -shared obj/StartEnd.o`  
This worked fine when using cabal normally, but was actually a bit ambiguous and cabal decided to name the object 'HSdll.dll'.
It seems like stack did not like that ambiguity and as such behaved differently from cabal. Once I explicitly specified the output filename, everything worked as expected even when building through stack.  
`ghc-options:         -O1 -shared obj/StartEnd.o -o HaskellLib.dll`
I find this difference in behaviour very strange as we are merely passing options to ghc which shouldn't really be all that relevant to either cabal or stack.
Anyway, it's good to know about this gotcha, and explicitly naming the output file is probably a wise idea anyway.

Apart from this little change. We do not need any special changes to build our project with stack. We can simply use the .yaml file that `stack init` generated for us.
Though I did take some time to organise the build artifacts into appropriate directories.

# Building the C or C++ project

Before building, it is important to specify the folders containing include files and libraries for ~~your local ghc installation~~ and of course your own haskell library (in cabal-project-dir/stub)

## Includes

*Project Configuration Properties -> C/C++ -> General -> Additional Include Directories*

~~needs includes to GHC headers (we only really need the type definitions in HsFFI.h) and to our own C headers inside the haskell project. I hate having to put absolute paths dependant on the local system inside project files (needing to fix your project file to include specifics for your local system after every pull and update gets tiresome quickly), so I've put the include path for GHC includes in an environment variable. On my system, the configuration looks like this:~~ We have added our own header for the FFI type definitions, so we no longer need to include GHC headers.  

We also need to include the stub headers generated by our earlier stack build.

* ~~`$(HASKELL_INCLUDE_PATH)` *(System environment variable I defined myself and pointing to `C:\HaskellPlatform\7.10.3\lib\include`)*~~ (This is no longer needed since we now have our own minimalist header)
* `..\..\haskell\stub`

## Libraries

It is also important to specify the library folder and library name for linking.

*Project Configuration Properties -> Linker -> General -> Additional Library Directories*

needs to point to the directory with object file we built in the previous step: `..\..\haskell\bin`

*Project Configuration Properties -> Linker -> Input -> Additional Dependencies*

needs to point to our final object: `HaskellLib.dll.a`

This file describes the symbols like functions which are contained in the .dll file, and is used to link against the .dll. Cabal and GHC create this file with an .a extension. This is equivalent to a .lib on Windows.

# Build and Run

Once all the includes and linking options are set correctly, we can finally build our C/C++ project. *(Ctrl-Shift-B)*

Before we run our new executable though, we need to ensure that the .dll we linked against can be found by our program. There are two options here:

* Copy the .dll from the Haskell project to the working directory of the executable. (this could even be a post-build command/hook in your project)
* Add the directory containing the .dll to our PATH variable.

The first option makes sense for distribution. For your own development system, adding to PATH may work better, since you don't need to bother with copying files around.
Some people might recommend copying the .dll to your windows system directory, as an easy way to get access to that .dll from anywhere without messing with a PATH variable, but that is just silly! Don't do that.

Now run the executable, and if everything went right, then you should see console output from both the C and Haskell side.

# Calling Back into C++ from Haskell

Since we are dynamic linking, getting bi-directional function calls working is a bit trickier. 
We can't just link our haskell lib against our main application because (1) it will not be compiled as a library but as an executable, and (2) if we tried, then we would quickly run into the cyclic dependency problem. Also known as the chicken-and-egg problem.

To sidestep these issues. We will adopt an approach commonly used by plugin architectures commonly found in extensible applications.
Upon initialization at runtime, we will call a function in our library and supply it with either a function pointer, or a struct containing multiple function pointers.
The library can then store these function pointers in some sort of global lookup table for later use by the library.

The following series of articles is a useful introduction to plugin architectures in c and c++ architectures: [Dr.Dobb's: Building Your Own Plugin Framework](http://www.drdobbs.com/cpp/building-your-own-plugin-framework-part/204202899)

#Final Notes

I find this project to be quite messy, with headers and object files created all over the place. There's got to be a better way.

The following Github issue suggests that soon there might be better support for building .dll files in the new Cabal 1.26 which should come out near the release of GHC 8:
[Add support for platform libraries #2540](https://github.com/haskell/cabal/pull/2540)
