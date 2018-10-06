*** NOTE ***

    If the compilation fails please try including -std=c++11 -l pthread in the flags.
    C++11 is required because mutex was used to make the mymalloc and myfree re-entrant and thread-safe.
	
	(Example Compilation Command: g++ -std=c++11 memutil.cpp -l pthread -o memutil)
	
	memutil needs to include "libmymem.cpp" in order to use the functions mymalloc and myfree


If no arguments are provided to memutil then, an error is displayed
Format:- ./memutil -n <integer> [-t <integer>]

The program produces no output if everything was successfull.
