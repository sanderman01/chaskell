// cppmain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Hello_stub.h"

extern "C" {
	void HsStart();
	void HsEnd();
}

int main(int argc, char** argv) {
	HsStart();
	std::cout << "Hello from C++\n";
	helloFromHaskell();
	HsEnd();
	std::cin.get();
	return 0;
}