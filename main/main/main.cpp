// cppmain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Hello_stub.h"

extern "C" {
	void HsStart();
	void HsEnd();
}

void helloFromC() {
	std::cout << "Hello from C++\n";
}

int main(int argc, char** argv) {
	HsStart();
	helloFromC();
	helloFromHaskell();

	// register the helloFromC function in the main application to the haskell plugin library.
	// normally, the plugin should store the function pointer for later use, but since these are example projects, 
	// for now we will immediately invoke the function.
	registerHelloFromC(helloFromC);

	HsEnd();
	std::cin.get();
	return 0;
}

