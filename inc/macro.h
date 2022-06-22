#ifndef VERSE_HEADER_MACRO
#define VERSE_HEADER_MACRO

// Limits
#define STRING_LENGTH_LINE    	2048
#define STRING_LENGTH_TOKEN   	256
#define STRING_LENGTH_VARDATA 	256
#define STRING_LENGTH_OFFSET  	128
#define STRING_LENGTH_REGISTER	92	// Why is this larger than the varname? Because register is sometimes substituted for the variable name + operator + offset
#define STRING_LENGTH_VARNAME 	64
#define STRING_LENGTH_NUMBER  	24
#define MAX_BYTESIZE_X64      	8
#define MAX_NESTLEVEL_CODE    	64
#define MAX_PASSCOUNT_CODE    	100
// Register ID
#define REGISTER_AX 	0
#define REGISTER_CX 	1
#define REGISTER_BX 	2
#define REGISTER_DX 	3
#define REGISTER_SP 	4
#define REGISTER_BP 	5
#define REGISTER_SI 	6
#define REGISTER_DI 	7
#define REGISTER_R8 	8
#define REGISTER_R9 	9
#define REGISTER_RA 	10
#define REGISTER_RB 	11
#define REGISTER_RC 	12
#define REGISTER_RD 	13
#define REGISTER_RE 	14
#define REGISTER_RF 	15

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#endif