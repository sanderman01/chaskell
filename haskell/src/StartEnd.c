// StartEnd.c
#include <Rts.h>
#include <stdio.h>

void HsStart()
{
   printf("Starting Haskell runtime.\n");
   int argc = 1;
   char* argv[] = {"ghcDll", NULL}; // argv must end with NULL

   // Initialize Haskell runtime
   char** args = argv;
   hs_init(&argc, &args);
}

void HsEnd()
{
   printf("Ending Haskell runtime.\n");
   hs_exit();
}