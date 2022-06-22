#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../inc/macro.h"
#include "../inc/node.h"
#include "../inc/parse.h"

int extdest(struct offext *ext)
{
	if (ext != NULL)
	{
		if (ext->ext != NULL) extdest(ext->ext);
		free(ext);
	}
	return 0;
}

// Create node from heap
struct node *nodeinit(const char *string)
{
	struct node *n = calloc(sizeof(struct node), 1);
	
	n->ptrlv = 0;
	strncpy(n->string, string, STRING_LENGTH_VARDATA);
	
	return n;
}

// Free all nodes from heap
int nodedest(struct node *root)
{	
	if (root != NULL)
	{
		if (root->ext != NULL)  extdest(root->ext);
		if (root->west != NULL) nodedest(root->west);
		if (root->east != NULL) nodedest(root->east);
		if (root->spex != NULL) nodedest(root->spex);
		free(root);
	}

	return 0;
}

// Identify operator value
int nodelex(struct node *n)
{
	// Determine operator priority
	if (n == NULL) return 0;
	else switch (strlen(n->string))
	{
		case 1:
			switch (n->string[0])
			{
				// Arithmemtic operators
				case '*': return OP_MUL;
				case '%': return OP_MOD;
				case '/': return OP_DIV;
				case '+': return OP_ADD;
				case '-': return OP_SUB;
				
				// Bitwise operators
				case '|': return OP_BOR;
				case '$': return OP_BXOR;
				case '&': return OP_BAND;
				
				// Equality operators
				case '=': return OP_MOV;
				
				// Conditional operators
				case '>': return OP_GTHN;
				case '<': return OP_LTHN;
			}
			break;
		case 2:
			// Bitwise operators
			if (!strcmp(n->string, "<<")) return OP_SHL;
			if (!strcmp(n->string, ">>")) return OP_SHR;
			
			// Equality operators
			if (!strcmp(n->string, "*=")) return OP_MOVM;
			if (!strcmp(n->string, "/=")) return OP_MOVD;
			if (!strcmp(n->string, "+=")) return OP_MOVA;
			if (!strcmp(n->string, "-=")) return OP_MOVS;
			
			// Conditional operators
			if (!strcmp(n->string, ">=")) return OP_GEQU;
			if (!strcmp(n->string, "<=")) return OP_LEQU;
			if (!strcmp(n->string, "==")) return OP_EQU;
			if (!strcmp(n->string, "!=")) return OP_NEQU;
			if (!strcmp(n->string, "<>")) return OP_NEQU;
			
			// Logical oeprators
			if (!strcmp(n->string, "||")) return OP_OR;
			if (!strcmp(n->string, "!|")) return OP_XOR;
			if (!strcmp(n->string, "&&")) return OP_AND;
			
			break;
	}
	return 0;
}

// Check node if it contains a function extension
int nodeisfun(struct node *n)
{
	for (struct node *nspec = n->spex; nspec != NULL; nspec = nspec->spex)
	if (nspec->string[0] == '(')
		return 1;
	
	return 0;
}
