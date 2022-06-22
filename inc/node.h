#ifndef VERSE_HEADER_NODE
#define VERSE_HEADER_NODE

#include "macro.h"
#include "vdata.h"

// Arithmetic operators
#define OP_MUL 	0xaa
#define OP_MOD 	0xa9
#define OP_DIV 	0xa8
#define OP_ADD 	0xa1
#define OP_SUB 	0xa0

// Bitwise operators
#define OP_BOR 	0x94
#define OP_BXOR	0x93
#define OP_BAND	0x92
#define OP_SHL 	0x91
#define OP_SHR 	0x90

// Equality operators
#define OP_MOVM	0x84
#define OP_MOVD	0x83
#define OP_MOVA	0x82
#define OP_MOVS	0x81
#define OP_MOV 	0x80

// Conmparison operators
#define OP_GTHN 0x1d
#define OP_LTHN	0x1c
#define OP_GEQU	0x1b
#define OP_LEQU	0x1a
#define OP_EQU	0x19
#define OP_NEQU	0x18

// Logical operators
#define OP_OR 	0x12
#define OP_XOR 	0x11
#define OP_AND	0x10

#define OP_GROUP_ARITH	20
#define OP_GROUP_BITWS	18
#define OP_GROUP_EQUAL	16
#define OP_GROUP_COMPS	3
#define OP_GROUP_LOGIC	2

struct offext
{
	struct offext *ext;
	unsigned int offset;
};

struct node
{
	char string[STRING_LENGTH_VARDATA];
	struct offext *ext;
	
	struct node
	*root,
	*west,
	*east,
	*spex;
	
	unsigned int 
	ptrlv, 
	arrsz;
	
	unsigned char
	type,
	size,
	isarr,
	isnot,
	isneg,
	isref;
};

int extdest(struct offext *);
struct node *nodeinit(const char *);
int          nodedest(struct node *);
int           nodelex(struct node *);
int         nodeisfun(struct node *);

#endif