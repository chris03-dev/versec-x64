#ifndef VERSE_HEADER_CDATA
#define VERSE_HEADER_CDATA

#include <stdio.h>

#include "macro.h"
#include "vdata.h"

#define CPU_RV64 	0
#define CPU_I386 	1
#define CPU_X64  	2
#define CPU_ARM64	3

#define OS_ONYX 	0
#define OS_LINUX 	1
#define OS_MACOS 	2
#define OS_WIN32 	3

// Compiler data struct
struct compiledata
{
	// Flags
	unsigned char
	f_cpu,                          	// Set CPU architecture target
	f_abi,                          	// Set platform target
	f_infunc,                       	// Check if pointer is currently in function
	f_mlcom,                        	// Check if line is inside multi-line comment
	f_verbose,                      	// Show detailed program operations
	f_noparse,                      	// Do not convert assembly to executable
	f_condisls[MAX_NESTLEVEL_CODE]; 	// Check whether previous condition is the last condition
	
	// Counters
	unsigned int
	c_condmaj[MAX_NESTLEVEL_CODE],  	// Condition counter (major)
	c_condmin[MAX_NESTLEVEL_CODE],  	// Condition counter (minor)
	c_linenum,                      	// Line counter
	c_stacklv,                      	// Reserved stack counter for stack variables
	c_bspilllv,                     	// Reserved stack counter for byte spills
	c_bspill,                       	// Byte spill counter in process tree
	c_condlv,                       	// Condition nest counter (i.e. condition counter index)
	c_fconstnum,                    	// Float constant counter (i.e. __F1:)
	c_dconstnum,                    	// Double constant counter (i.e. __D1:)
	c_sconstnum;                    	// String constant counter (i.e. __S1:)
	
	// Variable lists
	struct vlist
	*l_f,   	// List of functions
	*l_v,   	// List of true variables
	*l_c,   	// List of constants (and VSO variables) (-vso)
	
	*l_i,   	// List of signed integers
	*l_u,   	// List of unsigned integers
	*l_p,   	// List of floats/doubles
	*l_s,   	// List of stack variables (don't even think about removing this)
	
	*l_o;   	// List of object templates
	
	void (*lololol)(int);
};

#endif