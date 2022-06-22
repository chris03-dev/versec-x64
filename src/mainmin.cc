#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/macro.h"
#include "../inc/cdata.h"
#include "../inc/parse.h"
#include "../inc/node.h"
#include "../inc/main.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0


// Find the difference of a number from nearest base 2 number
int shiftdiff(long long unsigned int n)
{
	// Difference values
	int i = 0, diff = 0, pdiff = 0;
	
	// Check if operation not needed
	if (n == 1) return 0;
	
	// Loop until difference is less than 0
	while ((diff > 0) || (i == 0))
	{
		pdiff = diff;
		diff = n - (2 << i);
		i++;
	}
	
	// Determine the closer value to n
	if (n + diff + pdiff < n) diff = pdiff;
	return diff;
}

// Find the nearest base 2 number
int shiftbase(long long unsigned int n)
{
	// Difference values
	int i = 0, diff = 0, pdiff = 0;
	
	// Check if operation not needed
	if (n == 1) return 0;
	
	// Loop until difference is less than 0
	while ((diff > 0) || (i == 0))
	{
		pdiff = diff;
		diff = n - (2 << i);
		i++;
	}
	
	// Determine the closer value to n
	if (n + diff + pdiff < n) i--;
	return i;
}

// Perform a byte spill to store a variable
int asmbspillx64_s(FILE *fout, struct compiledata *cdata, const char *reg, char *wbitext)
{
	// Check if mov bits extension string is null
	if (wbitext == NULL) wbitext = "";
	
	// Store register
	if (cdata->c_bspill > 0) fprintf(fout, "mov%s [rsp%+i], %s\n\n", wbitext, cdata->c_bspill, reg);
	else                     fprintf(fout, "mov%s [rsp], %s\n\n",    wbitext, reg);
	
	// Increment byte spill value
	cdata->c_bspill += MAX_BYTESIZE_X64;
	
	// Check if exceeding current reserved byte spill for stack
	if (cdata->c_bspill > cdata->c_bspilllv)
		cdata->c_bspilllv += MAX_BYTESIZE_X64;
	
	return 0;
}

// Perform a byte spill to load a variable
int asmbspillx64_l(FILE *fout, struct compiledata *cdata, const char *reg, char *wbitext)
{
	// Check if mov bits extension string is null
	if (wbitext == NULL) wbitext = "";
	
	// Check if byte spill is valid
	if (cdata->c_bspill == 0)
	{
		fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in a function where you called 'asmbspillx64_l'.\n\n", stderr);
		return 1;
	}
	else
	{
		// Restore byte spill value
		cdata->c_bspill -= MAX_BYTESIZE_X64;
		
		if (cdata->c_bspill > 0) fprintf(fout, "mov%s %s, [rsp%+i]\n\n", wbitext, reg, cdata->c_bspill);
		else                     fprintf(fout, "mov%s %s, [rsp]\n\n",    wbitext, reg);
	}
	
	return 0;
}

// Optimize (L1) process tree before creating assembly code
int nodeoptimize_1(FILE *fout, struct compiledata *cdata)
{
	return 0;
}

// Optimize (L2) process tree before creating assembly code
int nodeoptimize_2(FILE *fout, struct compiledata *cdata)
{
	return 0;
}

// Optimize (L3) process tree before creating assembly code
int nodeoptimize_3(FILE *fout, struct compiledata *cdata)
{
	return 0;
}

// Setup data in process tree before creating assembly code
int asmpreproc(FILE *fout, struct compiledata *cdata, struct node *root)
{
	// Recursion calls
	if (root->west != NULL) asmpreproc(fout, cdata, root->west);
	if (root->east != NULL) asmpreproc(fout, cdata, root->east);
	if (root->spex != NULL) asmpreproc(fout, cdata, root->spex);
	
	// Apply flags to root node
	if (nodelex(root) == 0)
	{
		// Check if not symbol exists
		if (root->string[0] == '!')
		{
			root->isnot = 1;
			
			for (unsigned int i = 0, l = strlen(root->string); i < l; i++)
				root->string[i] = root->string[i + 1];
		}
		
		// Check if negation symbol exists
		if (root->string[0] == '-')
		{
			root->isneg = 1;
			
			for (unsigned int i = 0, l = strlen(root->string); i < l; i++)
				root->string[i] = root->string[i + 1];
		}
		
		// Check if reference symbol exists
		if (root->string[0] == '@')
		{
			root->isref++;
			
			for (unsigned int i = 0, l = strlen(root->string); i < l; i++)
				root->string[i] = root->string[i + 1];
		}
	}
	
	// Clean up name before special extensions
	if ((strchr(root->string, '[') > root->string)
	||  (strchr(root->string, '(') > root->string)
	||  (strchr(root->string, '{') > root->string))
	{
		char name[STRING_LENGTH_VARNAME];
		strctok(name, root->string, NULL, "[](){}", 0);
		strcpy(root->string, name);
	}
	
	// Check if offset extensions exist
	if ((strchr(root->string, '~') != NULL)
	||  (strchr(root->string, ':') != NULL)
	||  (strchr(root->string, '.') != NULL))
	{
		struct offext 
		*focus = NULL,  	// Focus node for creating offset extension chains on nodes
		*ntmp = NULL;   	// Temporary offset extension pointer for holding offsets
		
		char
		name[STRING_LENGTH_VARNAME],
		*namext = root->string,
		s_off[STRING_LENGTH_OFFSET];
		
		// Process node string
		strcltok(name, root->string, NULL, "[](){}", "~:.", 0);
		namext = root->string;
		
		//printf("PRE:  %s\n", namext);
		
		namext += strlen(name);
		
		//printf("POST: %s\n", namext);
		
		for (unsigned int ix = 0, c_off = 0, l = strlen(namext); ix < l;)
		{
			switch (namext[ix])
			{
				case '~':
					// Create node
					ntmp = calloc(sizeof(struct offext), 1);
					
					// Set node offset
					ntmp->offset = 0;
					
					// Increment index
					ix++;
					break;
				
				case ':':
					// Create node
					ntmp = calloc(sizeof(struct offext), 1);
					
					// Get offset string
					strctok(s_off, namext, NULL, "~:.", c_off);
					
					// Set node offset
					if (isinteger(s_off)) ntmp->offset = stoi(s_off);
					else                  ntmp->offset = 0;
					
					// Increment index
					ix += strlen(s_off) + 1;
					c_off++;
					break;
					
				case '.':
					// Create node
					ntmp = calloc(sizeof(struct offext), 1);
					
					// Get offset string
					strctok(s_off, namext, NULL, "~:.", c_off);
					
					// This is a stub. You should include:
					// - Automatic re-casting to other types and sizes
					
					/*
					
					root->type = vdatafind(vdatafind(cdata->l_obj, "object")->list, s_off)->offset;
					root->size = 
					*/
					
					// Increment index
					ix += strlen(s_off) + 1;
					c_off++;
					break;
			}
			
			// Check if first run
			if (focus == NULL)
			{
				root->ext = ntmp;
				ntmp->ext = NULL;
				focus = ntmp;
				ntmp = NULL;
			}
			
			else if (ntmp != NULL)
			{
				focus->ext = ntmp;
				ntmp->ext = NULL;
				focus = ntmp;
				ntmp = NULL;
			}
			else break;
		}
		
		// Simplify string in node
		root->string[strlen(name)] = '\0';
	}
	
	// Check if node is parenthesized
	if ((root->string[0] == '(')
	&& ((root->root == NULL)
	||  ((root->root != NULL)
	&&  (root != root->root->spex))))
	{
		char name[STRING_LENGTH_VARNAME];
		
		// Check if nodes contain parenthesized equations
		strctok(name, root->string, "()", NULL, 0);
		strip(name);
		
		if (nodeproc(fout, cdata, root, name, 0) == NULL)
			return 1;
	}
	
	// Check if node is part of process tree
	if ((root->west != NULL) && (root->east != NULL))
	{
		// Flags if number-based
		unsigned int
		// Check if integer
		iswint = isinteger(root->west->string),
		iseint = isinteger(root->east->string),
		
		// Check if precise number
		iswprc = isprecise(root->west->string),
		iseprc = isprecise(root->east->string),
		
		iswnum = (iswint || iswprc),
		isenum = (iseint || iseprc),
		
		// Check if operator
		//iswop = (nodelex(root->west) != 0),
		iseop = (nodelex(root->east) != 0),
		
		// Check if last branch
		//iswoplb = ((iswop) && (nodelex(root->west->west) == 0) && (nodelex(root->west->east) == 0)),
		iseoplb = ((iseop) && (nodelex(root->east->west) == 0) && (nodelex(root->east->east) == 0));
		
		// Setup node level
		
		/*/ Check if right branch has conditions for swapping operands for optimization
		if (iseoplb
		&& (nodelex(root->east) != OP_DIV)
		&& (nodelex(root->east) != OP_SUB))
		{
			// Swap nodes on east branch
			struct node
			*n_w = root->east->west,
			*n_e = root->east->east;
			
			unsigned int
			isewnum = (isinteger(root->east->west->string) || isprecise(root->east->west->string)),
			iseenum = (isinteger(root->east->east->string) || isprecise(root->east->east->string));
			
			if ((!isewnum) && (iseenum))
			{
				root->east->west = n_e;
				root->east->east = n_w;
			}
		}
		//*/
		
		// Check if both nodes are numbers
		if (iswnum && isenum)
		{
			// Check for precise simplification of numbers
			if (iswprc || iseprc)
			{
				// Values
				double
				wvalue = stod(root->west->string),
				evalue = stod(root->east->string),
				value = 0;
				
				// String for new value
				char vstring[STRING_LENGTH_VARDATA];
				
				// Compute value from nodes
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL: value = wvalue * evalue; break;
					case OP_DIV: value = wvalue / evalue; break;
					case OP_ADD: value = wvalue + evalue; break;
					case OP_SUB: value = wvalue - evalue; break;
					
					// Equality operators
					case OP_MOV:
					case OP_MOVM:
					case OP_MOVD:
					case OP_MOVA:
					case OP_MOVS:
						fprintf(stderr, "\nError (Line %u): Whoever came up with the idea to use a constant value as the destination address should sip a bit of coffee.\n\n", cdata->c_linenum);
						return 1;
					
					// Conditional operators
					case OP_GTHN: value = wvalue > evalue;  break;
					case OP_LTHN: value = wvalue < evalue;  break;
					case OP_GEQU: value = wvalue >= evalue; break;
					case OP_LEQU: value = wvalue <= evalue; break;
					case OP_EQU:  value = wvalue == evalue; break;
					case OP_NEQU: value = wvalue != evalue; break;
				}
				
				// Apply new value to nodes
				snprintf(vstring, STRING_LENGTH_VARDATA, "%lf", value);
				strncpy(root->string, vstring, STRING_LENGTH_VARDATA);
				//root->string[STRING_LENGTH_VARDATA] = '\0';
				
				// Predefine a type and size
				root->size = 8;
				
				if (iswint) root->type = 'i';
				
				// Free memory of children nodes
				nodedest(root->west);
				nodedest(root->east);
				
				root->west = NULL;
				root->east = NULL;
			}
			
			// Check for integer simplification of numbers
			else if (iswint && iseint)
			{
				// Values
				long long int
				wvalue = stoi(root->west->string),
				evalue = stoi(root->east->string),
				value = 0;
				
				// String for new value
				char vstring[STRING_LENGTH_VARDATA];
				
				// Compute value from nodes
				switch (nodelex(root))
				{
					// Equality operators
					case OP_MOV:
					case OP_MOVM:
					case OP_MOVD:
					case OP_MOVA:
					case OP_MOVS:
						fprintf(stderr, "\nError (Line %u): Whoever came up with the idea to use a constant value as the destination address should sip a bit of coffee.\n\n", cdata->c_linenum);
						return 1;
					
					// Arithmetic operators
					case OP_MUL: value = wvalue * evalue; break;
					case OP_DIV: value = wvalue / evalue; break;
					case OP_ADD: value = wvalue + evalue; break;
					case OP_SUB: value = wvalue - evalue; break;
					
					// Conditional operators
					case OP_GTHN: value = wvalue > evalue;  break;
					case OP_LTHN: value = wvalue < evalue;  break;
					case OP_GEQU: value = wvalue >= evalue; break;
					case OP_LEQU: value = wvalue <= evalue; break;
					case OP_EQU:  value = wvalue == evalue; break;
					case OP_NEQU: value = wvalue != evalue; break;
				}
				
				// Apply new value to nodes
				snprintf(vstring, STRING_LENGTH_VARDATA, "%lli", value);
				strncpy(root->string, vstring, STRING_LENGTH_VARDATA);
				//root->string[STRING_LENGTH_VARDATA] = '\0';
				
				// Predefine a type and size
				if (value > 0xffffffff) root->size = 8;
				else                    root->size = 4;
				
				if (iswint) root->type = 'i';
				
				// Free memory of children nodes
				nodedest(root->west);
				nodedest(root->east);
				
				root->west = NULL;
				root->east = NULL;
			}
		}
		
		// Check for division-by-zero errors
		else if (isenum)
		{
			switch (nodelex(root))
			{
				case OP_DIV:
				case OP_MOVD:
					if (stoi(root->east->string) == 0)
					{
						fprintf(stderr, "\nError (Line %u): Division by zero is not permitted.\n\n", cdata->c_linenum);
						return 1;
					}
			}
		}
	}
	
	return 0;
}

// Create x86-64 (base: i386) assembly code from process tree
// TODO: Add 'true root' argument to support byte spills
int asmprocx64(FILE *fout, struct compiledata *cdata, struct node *troot, struct node *root)
{
	// Check if node has children
	if ((root->west != NULL) && (root->east != NULL))
	{
		// Variable name
		unsigned char
		// Flags
		isroot = (root->root == NULL),
		iswest = (!isroot && (root->root->west == root)),
		iseast = (!isroot && (root->root->east == root)),
		
		// Check if operator
		iswop = (nodelex(root->west) != 0),
		iseop = (nodelex(root->east) != 0),
		
		// Check if last branch from root of node
		islb = (!iswop && !iseop);
		//isrwlb = (!isroot && (root->root->west != NULL) && ((nodelex(root->root->west->west) == 0) || (nodelex(root->root->west->east) == 0))),
		//isrelb = (!isroot && (root->root->east != NULL) && ((nodelex(root->root->east->west) == 0) || (nodelex(root->root->east->east) == 0)));
		
		char 
		*wname = root->west->string,
		*ename = root->east->string,
		*eopwname = root->east->west->string,
		*eopename = root->east->east->string,
		reg_wmaxint[STRING_LENGTH_REGISTER],
		reg_emaxint[STRING_LENGTH_REGISTER],
		reg_wmaximm[STRING_LENGTH_REGISTER],
		reg_emaximm[STRING_LENGTH_REGISTER];
		
		int
		// Check if variables (branches of west and east nodes)
		iseopwvar = (iswop && (root->east->west != NULL)
		&& ((vdatafind(cdata->l_i, eopwname) != NULL)
		||  (vdatafind(cdata->l_u, eopwname) != NULL)
		||  (vdatafind(cdata->l_p, eopwname) != NULL)
		||  (vdatafind(cdata->l_f, eopwname) != NULL)
		||  (nodeisfun(root->east->west)))),
		
		iseopevar = (iseop && (root->east->east != NULL)
		&& ((vdatafind(cdata->l_i, eopename) != NULL)
		||  (vdatafind(cdata->l_u, eopename) != NULL)
		||  (vdatafind(cdata->l_p, eopename) != NULL)
		||  (vdatafind(cdata->l_f, eopename) != NULL)
		||  (nodeisfun(root->east->east)))),
		
		// Check if last branch within node
		//iswoplb = ((iswop) && (nodelex(root->west->west) == 0) && (nodelex(root->west->east) == 0)),
		iseoplb = ((iseop) && (nodelex(root->east->west) == 0) && (nodelex(root->east->east) == 0));
		
		// Flags
		int
		// Check if integer
		iswint = isinteger(wname),
		iseint = isinteger(ename),
		
		// Check if precise number
		iswprc = isprecise(wname),
		iseprc = isprecise(ename),
		
		// Variable type
		iswvf = ((vdatafind(cdata->l_f, wname) != NULL) || (nodeisfun(root->west))),
		iswvi = (vdatafind(cdata->l_i, wname) != NULL),
		iswvu = (vdatafind(cdata->l_u, wname) != NULL),
		iswvp = (vdatafind(cdata->l_p, wname) != NULL),
		iswvs = (vdatafind(cdata->l_s, wname) != NULL),
		
		isevf = ((vdatafind(cdata->l_f, ename) != NULL) || (nodeisfun(root->east))),
		isevi = (vdatafind(cdata->l_i, ename) != NULL),
		isevu = (vdatafind(cdata->l_u, ename) != NULL),
		isevp = (vdatafind(cdata->l_p, ename) != NULL),
		isevs = (vdatafind(cdata->l_s, ename) != NULL),
		
		// Check if variable
		iswvar = (iswvi || iswvu || iswvp || iswvf),
		isevar = (isevi || isevu || isevp || isevf),
		
		// Check if alternate registers should be used for each node
		altwreg = (!isroot && iseast && (iswvar || iseop) && !iseopwvar), //islb
		altereg = (iseop && (iseopwvar || !iseopevar));//iseop && !iswop && !isrwlb
		
		// Special extension node
		struct node *nspex;
		
		// Flags for special extensions
		int
		hadex,
		hadexp,
		isexa = 0,	// Array index extension
		isexf = 0,	// Function call extension
		isexc = 0;	// Function cast extension
		
		// Setup registers for max byte size registers
		if (altwreg) strcpy(reg_wmaxint, "r10");
		else         strcpy(reg_wmaxint, "rax");
		
		if (altereg) strcpy(reg_emaxint, "r10");
		else         strcpy(reg_emaxint, "rcx");
		
		// Setup some stuff for post-order node traversal
		// Q: Why did you put the code for initializing the max byte size immediate registers here?
		// A: After I let the recursion of the 'asmproc' function finish, only then I could get the variable type on the node.
		// Setup registers for max byte size immediate (integer or precise number)
		
		// Recursive call for west node
		asmprocx64(fout, cdata, root, root->west);
		
		// Setup registers for max byte size immediate registers
		if (iswvar)
		{
			if (iswvp)
			{
				if (altwreg) strcpy(reg_wmaximm, "xmm2");
				else         strcpy(reg_wmaximm, "xmm0");
			}
			else strcpy(reg_wmaximm, reg_wmaxint);
		}
		else if (iswop)
		{
			switch (root->west->type)
			{
				case 'i':
				case 'u':
					strcpy(reg_wmaximm, reg_wmaxint);
					break;
				case 'p':
					if (altwreg) strcpy(reg_wmaximm, "xmm2");
					else         strcpy(reg_wmaximm, "xmm0");
					break;
				default:
					puts("WOT");
					break;
			}
		}
		else if (iswint || iswvi || iswvu)
		{
			strcpy(reg_wmaximm, reg_wmaxint);
		}
		else if (iswprc || iswvp)
		{
			if (altwreg) strcpy(reg_wmaximm, "xmm2");
			else         strcpy(reg_wmaximm, "xmm0");
		}
		
		// Setup push ax
		if (isroot && iseop && ((!iseoplb && (nodelex(root)/8 == OP_GROUP_EQUAL)) || ((iseopwvar || !iseoplb) && !iswvp && !iswprc && (root->west->type != 'p') && (nodelex(root->east) == OP_DIV))))
		{
			// Store byte spill value
			if (reg_wmaximm[0] == 'x') asmbspillx64_s(fout, cdata, reg_wmaximm, "sd");
			else                       asmbspillx64_s(fout, cdata, reg_wmaximm, NULL);
			root->east->root = NULL;
		}
		
		// Recursive call for east node
		asmprocx64(fout, cdata, root, root->east);
		
		// Setup registers for max byte size immediate registers
		if (isevar)
		{
			if (isevp)
			{
				if (altereg) strcpy(reg_emaximm, "xmm2");
				else         strcpy(reg_emaximm, "xmm1");
			}
			else strcpy(reg_emaximm, reg_emaxint);
		}
		else if (iseop)
		{
			switch (root->east->type)
			{
				case 'i':
				case 'u':
					strcpy(reg_emaximm, reg_emaxint);
					break;
				case 'p':
					if (altereg) strcpy(reg_emaximm, "xmm2");
					else         strcpy(reg_emaximm, "xmm1");
					break;
				default:
					puts("WOT");
					break;
			}
		}
		else if (iseint || isevi || isevu)
		{
			strcpy(reg_emaximm, reg_emaxint);
		}
		else if (iseprc || isevp)
		{
			if (altereg) strcpy(reg_emaximm, "xmm2");
			else         strcpy(reg_emaximm, "xmm1");
		}
		
		// Setup pop rax
		if (isroot && iseop && ((!iseoplb && (nodelex(root)/8 == OP_GROUP_EQUAL)) || ((iseopwvar || !iseoplb) && !iswvp && !iswprc && (root->west->type != 'p') && (nodelex(root->east) == OP_DIV))))
		{
			// Load byte spill value
			if (reg_wmaximm[0] == 'x') asmbspillx64_l(fout, cdata, reg_wmaximm, "sd");
			else                       asmbspillx64_l(fout, cdata, reg_wmaximm, NULL);
			root->east->root = root->root;
		}
		
		// Variable type definition
		if ((root->west->type != 'i')
		&&  (root->west->type != 'u')
		&&  (root->west->type != 'p'))
		{
			// Check if integer
			if (iswint) root->west->type = 'i';
			if (iswprc) root->west->type = 'p';
			
			// Check if variable
			if (iswvar)
			{
				if (iswvi) root->west->type = 'i';
				if (iswvu) root->west->type = 'u';
				if (iswvp) root->west->type = 'p';
			}
		}
		
		if ((root->east->type != 'i')
		&&  (root->east->type != 'u')
		&&  (root->east->type != 'p'))
		{
			// Check if integer
			if (iseint) root->east->type = 'i';
			if (iseprc) root->east->type = 'p';
			
			// Check if variable
			if (isevar)
			{
				if (isevi) root->east->type = 'i';
				if (isevu) root->east->type = 'u';
				if (isevp) root->east->type = 'p';
			}
		}
		
		//fprintf(fout, "; %i %i\n", root->west->size, root->east->size);
		
		// Variable size
		if (root->west->size == 0)
		{
			// Check if function
			if (iswvf) root->west->size = 8;
			
			// Check if variable
			else if (iswvar)
			{
				if (iswvi) root->west->size = vdatafind(cdata->l_i, wname)->size;
				if (iswvu) root->west->size = vdatafind(cdata->l_u, wname)->size;
				if (iswvp) root->west->size = vdatafind(cdata->l_p, wname)->size;
			}
			
			// Check if precise
			else if ((root->east->type == 'p') || iswprc)
			{
				if (isevp || (iseop && root->east->type == 'p'))
				{
					if (isevp) root->west->size = vdatafind(cdata->l_p, ename)->size;
					else       root->west->size = root->east->size;
				}
				else switch (wname[strlen(wname) - 1])
				{
					case 'f': root->west->size = 4; break;
					case 'd': root->west->size = 8; break;
					default:  root->west->size = 8; break;
				}
			}
			
			// Check if integer
			else if (iswint)
			{
				long long unsigned int nsize = stoi(wname);
				if (nsize > 0xffffffff) root->west->size = 8;
				else                    root->west->size = 4;
			}
		}
		
		if (root->east->size == 0)
		{
			// Check if function
			if (isevf) root->east->size = 8;
			
			// Check if variable
			else if (isevar)
			{
				if (isevi) root->east->size = vdatafind(cdata->l_i, ename)->size;
				if (isevu) root->east->size = vdatafind(cdata->l_u, ename)->size;
				if (isevp) root->east->size = vdatafind(cdata->l_p, ename)->size;
			}
			
			// Check if precise
			else if ((root->west->type == 'p') || iseprc)
			{
				//printf("MCFLO %s %s %s\n", root->west->string, root->string, root->east->string);
				if (iswvp || (iswop && root->west->type == 'p'))
				{
					if (iswvp) root->east->size = vdatafind(cdata->l_p, wname)->size;
					else       root->east->size = root->west->size;
				}
				else switch (ename[strlen(ename) - 1])
				{
					case 'f': root->east->size = 4; break;
					case 'd': root->east->size = 8; break;
					default:  root->east->size = 8; break;
				}
			}
			
			// Check if integer
			else if (iseint)
			{
				long long unsigned int nsize = stoi(ename);
				if (nsize > 0xffffffff) root->east->size = 8;
				else                    root->east->size = 4;
			}
		}
		
		//fprintf(fout, "; %i %i\n", root->west->size, root->east->size);
		
		//if (root->west->spex != NULL) 
		//printf("spex NODE: %c%i %c%i (%i, %i)\n", root->west->type, root->west->size, root->east->type, root->east->size, iswvar || iswop, isevar || iseop);
		
		// Get the pointer levels, and array state
		if (iswvar)
		{
			if (iswvi) {root->west->ptrlv = vdatafind(cdata->l_i, wname)->ptrlv; root->west->isarr = vdatafind(cdata->l_i, wname)->isarr;}
			if (iswvu) {root->west->ptrlv = vdatafind(cdata->l_u, wname)->ptrlv; root->west->isarr = vdatafind(cdata->l_u, wname)->isarr;}
			if (iswvp) {root->west->ptrlv = vdatafind(cdata->l_p, wname)->ptrlv; root->west->isarr = vdatafind(cdata->l_p, wname)->isarr;}
		}
		if (isevar)
		{
			if (isevi) {root->east->ptrlv = vdatafind(cdata->l_i, ename)->ptrlv; root->east->isarr = vdatafind(cdata->l_i, ename)->isarr;}
			if (isevu) {root->east->ptrlv = vdatafind(cdata->l_u, ename)->ptrlv; root->east->isarr = vdatafind(cdata->l_u, ename)->isarr;}
			if (isevp) {root->east->ptrlv = vdatafind(cdata->l_p, ename)->ptrlv; root->east->isarr = vdatafind(cdata->l_p, ename)->isarr;}
		}
		
		// Strings for assembly code
		char
		reg_wptr[STRING_LENGTH_REGISTER] = "",  	// West pointer register    	(to hold address)
		reg_wimm[STRING_LENGTH_REGISTER] = "",  	// West immediate register  	(to hold value)
		reg_wsrc[STRING_LENGTH_REGISTER] = "",  	// West source register     	(to store value)
		wsizedef[7] = "",                       	// West size operand        	(i.e. dword, byte)
		wbitext[12] = "",                       	// West sign/zero extension 	(i.e. movsx, movzx)
		wopext[12] = "",                        	// West operation extension 	(i.e. addss, subsd)
		
		reg_eptr[STRING_LENGTH_REGISTER] = "",  	// East pointer register    	(to hold address)
		reg_eimm[STRING_LENGTH_REGISTER] = "",  	// East immediate register  	(to hold value)
		reg_esrc[STRING_LENGTH_REGISTER] = "",  	// East source register     	(to store value)
		esizedef[7] = "",                       	// East size operand        	(i.e. dword, byte)
		ebitext[12] = "",                       	// East sign/zero extension 	(i.e. movsx, movzx)
		eopext[12] = "";                        	// East operation extension 	(i.e. addss, subsd)
		
		// Setup special extension node
		nspex = root->west->spex;
		
		// Reset extension history flags
		hadex = 0;
		hadexp = 0;
		
		// Loop for setting up values while accomodating special extensions
		//do
		{
			// Reset special extension flags
			isexa = 0;
			isexf = 0;
			isexc = 0;
			
			// Setup special extension flags
			if (nspex != NULL)
			{
				isexa = (nspex->string[0] == '[');  	// Check if array index
				isexf = (nspex->string[0] == '(');  	// Check if function call
				isexc = (nspex->string[0] == '{');  	// Check if variable cast
				
				// Check if variable had extension from the past
				hadexp = hadex;
				if (hadex == 0) hadex = ((root->west->ext != NULL) || (nspex != root->west->spex));
			}
			
			// Setup assembly-related strings
			if (altwreg)
			{
				if (iswvf)
				{
					strcpy(reg_wptr, "r10");
					strcpy(reg_wimm, "r10");
					strcpy(reg_wsrc, "r10");
				}
				else if (iswvar || iswop)
				{
					// Set pointer register
					if (iswop) strcpy(reg_wptr, reg_wmaxint);
					else       strcpy(reg_wptr, "rdx");
					
					switch (root->west->type)
					{
						case 'i':
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(reg_wimm, "r10");  strcpy(reg_wsrc, "r10");  strcpy(wsizedef, "");       break;
								case 4: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10d"); strcpy(wsizedef, "dword "); break;
								case 2: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10w"); strcpy(wsizedef, "word ");  strcpy(wbitext, "sx"); break;
								case 1: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10b"); strcpy(wsizedef, "byte ");  strcpy(wbitext, "sx"); break;
							}
							break;
						case 'u':
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(reg_wimm, "r10");  strcpy(reg_wsrc, "r10");  strcpy(wsizedef, "");       break;
								case 4: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10d"); strcpy(wsizedef, "dword "); break;
								case 2: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10w"); strcpy(wsizedef, "word ");  strcpy(wbitext, "zx"); break;
								case 1: strcpy(reg_wimm, "r10d"); strcpy(reg_wsrc, "r10b"); strcpy(wsizedef, "byte ");  strcpy(wbitext, "zx"); break;
							}
							break;
						case 'p':
							strcpy(reg_wimm, "xmm2");
							strcpy(reg_wsrc, "xmm2");
							strcpy(wsizedef, "");
							
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(wsizedef, "qword "); strcpy(wbitext, "sd"); strcpy(wopext, "sd"); break;
								case 4: strcpy(wsizedef, "dword "); strcpy(wbitext, "ss"); strcpy(wopext, "ss"); break;
							}
							break;
					}
				}
				else if ((iseop && root->east->type == 'p') || iswprc || isevp)
				{
					// Set immediate and source register
					strcpy(reg_wimm, "xmm2");
					strcpy(reg_wsrc, "xmm2");
					
					switch (root->west->size)
					{
						case 8: strcpy(wsizedef, "qword "); strcpy(wbitext, "sd"); strcpy(wopext, "sd"); break;
						case 4: strcpy(wsizedef, "dword "); strcpy(wbitext, "ss"); strcpy(wopext, "ss"); break;
					}
					
					// Change node string to precise number constant
					FILE *fc;
					switch (root->west->size)
					{
						case 4: fc = fopen("fconst.txt", "a+"); break;
						case 8: fc = fopen("dconst.txt", "a+"); break;
					}
					
					// Assign value to constant
					if (fc == NULL)
					{
						fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
						return 1;
					}
					else
					{
						// Align bytes for SSE2
						fputs("align 16\n", fc);
						
						// Assign precise value to constant
						switch (root->west->size)
						{
							case 4:
								fprintf(fc, "__F%i dd %lf\n", cdata->c_fconstnum, stod(wname));
								snprintf(wname, STRING_LENGTH_VARNAME, "[__F%i]", cdata->c_fconstnum);
								cdata->c_fconstnum++;
								break;
							case 8:
								fprintf(fc, "__D%i dq %lf\n", cdata->c_dconstnum, stod(wname));
								snprintf(wname, STRING_LENGTH_VARNAME, "[__D%i]", cdata->c_dconstnum);
								cdata->c_dconstnum++;
								break;
						}
						
						// Close file
						fclose(fc);
					}
					
				}
				else if (iswint)
				{
					switch (root->west->size)
					{
						case 8: strcpy(reg_wimm, "r10");  break;
						case 4: strcpy(reg_wimm, "r10d"); break;
					}
				}
				else if (!iswop)
				{
					fprintf(stderr, "\nError (Line %u): Invalid number, variable or function '%s'.\n\n", cdata->c_linenum, wname);
					return 1;
				}
				
			}
			else
			{
				if (iswvf)
				{
					strcpy(reg_wptr, "rax");
					strcpy(reg_wimm, "rax");
					strcpy(reg_wsrc, "rax");
				}
				else if (iswvar || iswop)
				{
					// Set pointer register
					if (iswop) strcpy(reg_wptr, reg_wmaxint);
					else       strcpy(reg_wptr, "rdx");
					
					switch (root->west->type)
					{
						case 'i':
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(reg_wimm, "rax"); strcpy(reg_wsrc, "rax"); strcpy(wsizedef, "");       break;
								case 4: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "eax"); strcpy(wsizedef, "dword "); break;
								case 2: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "ax");  strcpy(wsizedef, "word ");  strcpy(wbitext, "sx"); break;
								case 1: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "al");  strcpy(wsizedef, "byte ");  strcpy(wbitext, "sx"); break;
							}
							break;
						case 'u':
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(reg_wimm, "rax"); strcpy(reg_wsrc, "rax"); strcpy(wsizedef, "");       break;
								case 4: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "eax"); strcpy(wsizedef, "dword "); break;
								case 2: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "ax");  strcpy(wsizedef, "word ");  strcpy(wbitext, "zx"); break;
								case 1: strcpy(reg_wimm, "eax"); strcpy(reg_wsrc, "al");  strcpy(wsizedef, "byte ");  strcpy(wbitext, "zx"); break;
							}
							break;
						case 'p':
							strcpy(reg_wimm, "xmm0");
							strcpy(reg_wsrc, "xmm0");
							strcpy(wsizedef, "");
							
							// Set immediate and source register
							switch (root->west->size)
							{
								case 8: strcpy(wsizedef, "qword "); strcpy(wbitext, "sd"); strcpy(wopext, "sd"); break;
								case 4: strcpy(wsizedef, "dword "); strcpy(wbitext, "ss"); strcpy(wopext, "ss"); break;
							}
							break;
					}
				}
				else if ((iseop && root->east->type == 'p') || iswprc || isevp)
				{
					// Set immediate and source register
					strcpy(reg_wimm, "xmm0");
					strcpy(reg_wsrc, "xmm0");
					
					switch (root->west->size)
					{
						case 8: strcpy(wsizedef, "qword "); strcpy(wbitext, "sd"); strcpy(wopext, "sd"); break;
						case 4: strcpy(wsizedef, "dword "); strcpy(wbitext, "ss"); strcpy(wopext, "ss"); break;
					}
					
					// Change node string to precise number constant
					FILE *fc;
					switch (root->west->size)
					{
						case 4: fc = fopen("fconst.txt", "a+"); break;
						case 8: fc = fopen("dconst.txt", "a+"); break;
					}
					
					// Assign value to constant
					if (fc == NULL)
					{
						fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
						return 1;
					}
					else
					{
						// Align bytes for SSE2
						fputs("align 16\n", fc);
						
						// Assign precise value to constant
						switch (root->west->size)
						{
							case 4:
								fprintf(fc, "__F%i dd %lf\n", cdata->c_fconstnum, stod(wname));
								snprintf(wname, STRING_LENGTH_VARNAME, "[__F%i]", cdata->c_fconstnum);
								cdata->c_fconstnum++;
								break;
							case 8:
								fprintf(fc, "__D%i dq %lf\n", cdata->c_dconstnum, stod(wname));
								snprintf(wname, STRING_LENGTH_VARNAME, "[__D%i]", cdata->c_dconstnum);
								cdata->c_dconstnum++;
								break;
						}
						
						// Close file
						fclose(fc);
					}
					
				}
				else if (iswint)
				{
					switch (root->west->size)
					{
						case 8: strcpy(reg_wimm, "rax"); break;
						case 4: strcpy(reg_wimm, "eax"); break;
					}
				}
				else if (!iswop)
				{
					fprintf(stderr, "\nError (Line %u): Invalid number, variable or function '%s'.\n\n", cdata->c_linenum, wname);
					return 1;
				}
			}
			
			// Setup function call
			if (isexf)
			{
				// Setup function arguments
				if (nspex != root->west->spex) asmbspillx64_s(fout, cdata, reg_wimm, wbitext);
				if (nspex != NULL)             callprocx64(fout, cdata, nspex->root);
				if (nspex != root->west->spex) asmbspillx64_l(fout, cdata, reg_wimm, wbitext);
				
				// Update size and type based on function return
				if (!iswop) iswvar = 1;/*
				root->west->type = 'i';
				root->west->size = 4;*/
			}
			
			// Setup values
			if (iswvar || iswop)
			{
				if (root->west->ext != NULL)
				{
					struct offext *n;   	// Extension node pointer
					unsigned int c = 0; 	// Pointer level counter
					
					// Check if variable is pointer
					for (n = root->west->ext; (c < root->west->ptrlv) && (n->ext != NULL); ++c)
					{
						unsigned int offset = n->offset;
						
						// First run in loop
						if ((c == 0) && !hadexp)
						{
							// Check if reference
							if (root->west->isref)
							{
								if (iswvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_wptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, wname)->offset);
								else
								{
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_wptr, wname, n->offset * MAX_BYTESIZE_X64);
									else       	    fprintf(fout, "mov %s, [%s]\n",    reg_wptr, wname);
								}
								
								// Increment offset extension
								n = n->ext;
							}
							else
							{
								if (iswvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_wptr, -vdatafind(cdata->l_s, wname)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_wptr, wname);
							}
						}
						
						// Subsequent runs in loop
						else
						{
							if (offset > 0) fprintf(fout, "mov %s, [%s+%i]\n", reg_wptr, reg_wptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_wptr, reg_wptr);
							
							// Increment offset extension
							n = n->ext;
						}
					}
					
					// Update pointer level
					root->west->ptrlv -= c;
					
					// Check if loop has not run before
					if ((c == 0) && !hadexp)
					{
						// Setup offset variable (for optimization)
						unsigned int offset = n->offset;
						
						// Check if function
						if (root->west->isref)
						{
							if (offset > 0)
							{
								if (iswvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_wptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, wname)->offset);
								else       fprintf(fout, "mov %s, [%s%+i]\n",  reg_wptr, wname, offset * MAX_BYTESIZE_X64);
							}
							else
							{
								if (iswvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_wptr, -vdatafind(cdata->l_s, wname)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_wptr, wname);
							}
						}
						
						// Check if operator
						else if (iswop && (root->west->ptrlv == 0))
						{
							if (nodelex(root)/8 == OP_GROUP_EQUAL)
							{
								if (offset > 0) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s%+i", reg_wimm, offset * root->west->size);
							}
							else
							{
								if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", wbitext, reg_wimm, wsizedef, reg_wimm, offset * root->west->size);
								else            fprintf(fout, "mov%s %s, %s[%s]\n",    wbitext, reg_wimm, wsizedef, reg_wimm);
							}
						}
						
						// Check if pointer/function
						else if (iswvf || (root->west->ptrlv > 0))
						{
							root->west->ptrlv--;
							
							// Remove conversion of sizes
							if (root->west->ptrlv > 0) 
							{
								strcpy(wbitext, "");
								strcpy(wsizedef, "");
								strcpy(reg_wimm, reg_wmaxint);
							}
							
							// Check if variable
							if (iswvar)
							{
								if (iswvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_wptr, -vdatafind(cdata->l_s, wname)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_wptr, wname);
							}
							
							if (nodelex(root)/8 == OP_GROUP_EQUAL)
							{
								if (offset > 0)
								{
									char reg_temp[STRING_LENGTH_REGISTER];
									strcpy(reg_temp, reg_wptr);
									
									if (root->west->ptrlv > 0) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s%+i", reg_temp, MAX_BYTESIZE_X64 * root->west->size);
									else                       snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s%+i", reg_temp, offset * root->west->size);
								}
							}
							else
							{
								if (root->west->ptrlv > 0)
								{
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_wimm, reg_wptr, offset * MAX_BYTESIZE_X64);
									else            fprintf(fout, "mov %s, [%s]\n",    reg_wimm, reg_wptr);
								}
								else
								{
									if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", wbitext, reg_wimm, wsizedef, reg_wptr, offset * root->west->size);
									else            fprintf(fout, "mov%s %s, %s[%s]\n",    wbitext, reg_wimm, wsizedef, reg_wptr);
								}
							}
						}
						
						// Check if array
						else if (root->west->isarr)
						{
							if (nodelex(root)/8 == OP_GROUP_EQUAL)
							{
								if (offset > 0)
								{
									if (iswvs) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "rbp%+i", (offset * root->west->size) - vdatafind(cdata->l_s, wname)->offset);
									else       snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s%+i", wname, offset * root->west->size);
								}
								else
								{
									if (iswvs) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "rbp%+i", -vdatafind(cdata->l_s, wname)->offset);
									else       snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s", wname);
								}
							}
							else
							{
								if (offset > 0)
								{
									if (iswvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", wbitext, reg_wimm, wsizedef, (offset * root->west->size) - vdatafind(cdata->l_s, wname)->offset);
									else       fprintf(fout, "mov%s %s, %s[%s%+i]\n",  wbitext, reg_wimm, wsizedef, wname, (offset * root->west->size));
								}
								else
								{
									if (iswvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", wbitext, reg_wimm, wsizedef, vdatafind(cdata->l_s, wname)->offset);
									else       fprintf(fout, "mov%s %s, %s[%s]\n",     wbitext, reg_wimm, wsizedef, wname);
								}
							}
						}
						
						else
						{
							fprintf(stderr, "\nError (Line %u): '%s' is neither an array, nor a pointer.\n\n", cdata->c_linenum, wname);
							return 1;
						}
					}
					
					else
					{
						unsigned int offset = (n != NULL) ? n->offset : 0;
						
						// Check if function
						if (iswvf)
						{
							if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_wimm, reg_wptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_wimm, reg_wptr);
						}
						
						// Check if no longer a pointer
						else if (root->west->ptrlv == 0)
						{
							if (nodelex(root)/8 == OP_GROUP_EQUAL)
							{
								if (offset > 0)
								{
									char reg_temp[STRING_LENGTH_REGISTER];
									strcpy(reg_temp, reg_wptr);
									
									snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s%+i", reg_temp, offset * root->west->size);
								}
							}
							
							else
							{
								if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", wbitext, reg_wimm, wsizedef, reg_wptr, (offset * root->west->size));
								else            fprintf(fout, "mov%s %s, %s[%s]\n",    wbitext, reg_wimm, wsizedef, reg_wptr);
							}
						}
						
						// Check if still a pointer
						else
						{
							// Force source register to hold pointer
							strcpy(reg_wsrc, reg_wmaxint);
							strcpy(reg_wimm, reg_wmaxint);
							
							puts("STILL A PTR");
							
							// This is a stub. Feel free to add anything later.
						}
					}
				}
				else if (iswop && (root->west->ptrlv > 0))
				{
					strcpy(reg_wimm, reg_wmaxint);
				}
				else if (isexa)
				{
					
				}
				else if (isexf)
				{
					if (root->west->isref)
					{
						if (iswvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_wimm, -vdatafind(cdata->l_s, wname)->offset, reg_wimm);
						else       fprintf(fout, "call %s\n",                   wname);
					}
					else
					{
						if (iswvs) fprintf(fout, "call qword [rbp%+i]\n", -vdatafind(cdata->l_s, wname)->offset);
						else       fprintf(fout, "call qword [%s]\n",     wname);
					}
				}
				else if (isexc)
				{
					
				}
				else if (iswvar)
				{
					if (root->west->isref)
					{
						// Force source register to hold pointer
						strcpy(reg_wsrc, reg_wmaxint);
						strcpy(reg_wimm, reg_wmaxint);
						
						if (iswvs) fprintf(fout, "lea %s, [rbp+%i]\n", reg_wimm, -vdatafind(cdata->l_s, wname)->offset);
						else       fprintf(fout, "mov %s, %s\n",       reg_wimm, wname);
					}
					else if ((root->west->ptrlv > 0) || root->west->isarr)
					{
						// Force source register to hold pointer
						strcpy(reg_wsrc, reg_wmaxint);
						strcpy(reg_wimm, reg_wmaxint);
						
						if (nodelex(root)/8 == OP_GROUP_EQUAL)
						{
							// Remove conversion of sizes
							if (nodelex(root) != OP_MOV)
							{
								strcpy(wbitext, "");
								strcpy(wsizedef, "");
							}
							
							if (iswvs) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "rbp%+i", -vdatafind(cdata->l_s, wname)->offset);
							else       snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s", wname);
						}
						else
						{
							if (iswvs) fprintf(fout, "mov %s, [rbp-%i]\n", reg_wimm, vdatafind(cdata->l_s, wname)->offset);
							else       fprintf(fout, "mov %s, [%s]\n",     reg_wimm, wname);
						}
					}
					else
					{
						// Use XOR operation to remove data dependency
						if (iswvp && (nspex == NULL)) fprintf(fout, "pxor %s, %s\n", reg_wimm, reg_wimm);
						
						if (nodelex(root)/8 == OP_GROUP_EQUAL)
						{
							if (iswvs) snprintf(reg_wptr, STRING_LENGTH_REGISTER, "rbp%+i", -vdatafind(cdata->l_s, wname)->offset);
							else       snprintf(reg_wptr, STRING_LENGTH_REGISTER, "%s", wname);
						}
						else
						{
							if (iswvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", wbitext, reg_wimm, wsizedef, -vdatafind(cdata->l_s, wname)->offset);
							else       fprintf(fout, "mov%s %s, %s[%s]\n",     wbitext, reg_wimm, wsizedef, wname);
						}
					}
				}
			}
		}
		//while ((root->west->ext != NULL) || (nspex != NULL));
		
		// Setup special extension node
		nspex = root->east->spex;
		
		// Reset extension history flags
		hadex = 0;
		hadexp = 0;
		
		//do
		{
			// Reset special extension flags
			isexa = 0;
			isexf = 0;
			isexc = 0;
			
			// Setup special extension flags
			if (nspex != NULL)
			{
				isexa = (nspex->string[0] == '[');  	// Check if array index
				isexf = (nspex->string[0] == '(');  	// Check if function call
				isexc = (nspex->string[0] == '{');  	// Check if variable cast
				
				// Check if variable had extension from the past
				hadexp = hadex;
				if (hadex == 0) hadex = ((root->east->ext != NULL) || (nspex != root->east->spex));
			}
			
			// Setup assembly-related strings
			if (altereg)
			{
				if (isevf)
				{
					strcpy(reg_eptr, "r10");
					strcpy(reg_eimm, "r10");
					strcpy(reg_esrc, "r10");
				}
				else if (isevar || iseop)
				{
					// Set pointer register
					strcpy(reg_eptr, "r10");
					
					switch (root->east->type)
					{
						case 'i':
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(reg_eimm, "r10");  strcpy(reg_esrc, "r10");  strcpy(esizedef, "");       break;
								case 4: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10d"); strcpy(esizedef, "dword "); break;
								case 2: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10w"); strcpy(esizedef, "word ");  strcpy(ebitext, "sx"); break;
								case 1: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10b"); strcpy(esizedef, "byte ");  strcpy(ebitext, "sx"); break;
							}
							break;
						case 'u':
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(reg_eimm, "r10");  strcpy(reg_esrc, "r10");  strcpy(esizedef, "");       break;
								case 4: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10d"); strcpy(esizedef, "dword "); break;
								case 2: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10w"); strcpy(esizedef, "word ");  strcpy(ebitext, "zx"); break;
								case 1: strcpy(reg_eimm, "r10d"); strcpy(reg_esrc, "r10b"); strcpy(esizedef, "byte ");  strcpy(ebitext, "zx"); break;
							}
							break;
						case 'p':
							strcpy(reg_eimm, "xmm2");
							strcpy(reg_esrc, "xmm2");
							strcpy(esizedef, "");
							
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(esizedef, "qword "); strcpy(ebitext, "sd"); strcpy(eopext, "sd"); break;
								case 4: strcpy(esizedef, "dword "); strcpy(ebitext, "ss"); strcpy(eopext, "ss"); break;
							}
					}
				}
				else if ((iswop && root->west->type == 'p') || iseprc || iswvp)
				{
					// Set immediate and source register
					strcpy(reg_eimm, "xmm2");
					strcpy(reg_esrc, "xmm2");
					
					switch (root->east->size)
					{
						case 8: strcpy(esizedef, "qword "); strcpy(ebitext, "sd"); strcpy(eopext, "sd"); break;
						case 4: strcpy(esizedef, "dword "); strcpy(ebitext, "ss"); strcpy(eopext, "ss"); break;
					}
					
					// Change node string to precise number constant
					FILE *fc;
					switch (root->east->size)
					{
						case 4: fc = fopen("fconst.txt", "a+"); break;
						case 8: fc = fopen("dconst.txt", "a+"); break;
					}
					
					// Assign value to constant
					if (fc == NULL)
					{
						fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
						return 1;
					}
					else
					{
						// Align bytes for SSE2
						fputs("align 16\n", fc);
						
						// Assign precise value to constant
						switch (root->east->size)
						{
							case 4:
								fprintf(fc, "__F%i dd %lf\n", cdata->c_fconstnum, stod(ename));
								snprintf(ename, STRING_LENGTH_VARNAME, "[__F%i]", cdata->c_fconstnum);
								cdata->c_fconstnum++;
								break;
							case 8:
								fprintf(fc, "__D%i dq %lf\n", cdata->c_dconstnum, stod(ename));
								snprintf(ename, STRING_LENGTH_VARNAME, "[__D%i]", cdata->c_dconstnum);
								cdata->c_dconstnum++;
								break;
						}
						
						// Close file
						fclose(fc);
					}
					
				}
				else if (iseint)
				{
					switch (root->east->size)
					{
						case 8: strcpy(reg_eimm, "r10");  break;
						case 4: strcpy(reg_eimm, "r10d"); break;
					}
				}
				else if (!iseop)
				{
					fprintf(stderr, "\nError (Line %u): Invalid number, variable or function '%s'.\n\n", cdata->c_linenum, ename);
					return 1;
				}
			}
			else
			{
				if (isevf)
				{
					strcpy(reg_eptr, "rcx");
					strcpy(reg_eimm, "rcx");
					strcpy(reg_esrc, "rcx");
				}
				else if (isevar || iseop)
				{
					// Set pointer register
					strcpy(reg_eptr, "rcx");
					
					switch (root->east->type)
					{
						case 'i':
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(reg_eimm, "rcx"); strcpy(reg_esrc, "rcx"); strcpy(esizedef, "");       break;
								case 4: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "dword "); break;
								case 2: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "word ");  strcpy(ebitext, "sx"); break;
								case 1: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "byte ");  strcpy(ebitext, "sx"); break;
							}
							break;
						case 'u':
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(reg_eimm, "rcx"); strcpy(reg_esrc, "rcx"); strcpy(esizedef, "");       break;
								case 4: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "dword "); break;
								case 2: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "word ");  strcpy(ebitext, "zx"); break;
								case 1: strcpy(reg_eimm, "ecx"); strcpy(reg_esrc, "ecx"); strcpy(esizedef, "byte ");  strcpy(ebitext, "zx"); break;
							}
							break;
						case 'p':
							strcpy(reg_eimm, "xmm1");
							strcpy(reg_esrc, "xmm1");
							strcpy(esizedef, "");
							
							// Set immediate and source register
							switch (root->east->size)
							{
								case 8: strcpy(esizedef, "qword "); strcpy(ebitext, "sd"); strcpy(eopext, "sd"); break;
								case 4: strcpy(esizedef, "dword "); strcpy(ebitext, "ss"); strcpy(eopext, "ss"); break;
							}
					}
				}
				else if ((iswop && root->west->type == 'p') || iseprc || iswvp)
				{
					// Set immediate and source register
					strcpy(reg_eimm, "xmm1");
					strcpy(reg_esrc, "xmm1");
					
					switch (root->east->size)
					{
						case 8: strcpy(esizedef, "qword "); strcpy(ebitext, "sd"); strcpy(eopext, "sd"); break;
						case 4: strcpy(esizedef, "dword "); strcpy(ebitext, "ss"); strcpy(eopext, "ss"); break;
					}
					
					// Change node string to precise number constant
					FILE *fc;
					switch (root->east->size)
					{
						case 4: fc = fopen("fconst.txt", "a+"); break;
						case 8: fc = fopen("dconst.txt", "a+"); break;
					}
					
					// Assign value to constant
					if (fc == NULL)
					{
						fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
						return 1;
					}
					else
					{
						// Align bytes for SSE2
						fputs("align 16\n", fc);
						
						// Assign precise value to constant
						switch (root->east->size)
						{
							case 4:
								fprintf(fc, "__F%i dd %lf\n", cdata->c_fconstnum, stod(ename));
								snprintf(ename, STRING_LENGTH_VARNAME, "[__F%i]", cdata->c_fconstnum);
								cdata->c_fconstnum++;
								break;
							case 8:
								fprintf(fc, "__D%i dq %lf\n", cdata->c_dconstnum, stod(ename));
								snprintf(ename, STRING_LENGTH_VARNAME, "[__D%i]", cdata->c_dconstnum);
								cdata->c_dconstnum++;
								break;
						}
						
						// Close file
						fclose(fc);
					}
					
				}
				else if (iseint)
				{
					switch (root->east->size)
					{
						case 8: strcpy(reg_eimm, "rcx"); break;
						case 4: strcpy(reg_eimm, "ecx"); break;
					}
				}
				else if (!iseop)
				{
					fprintf(stderr, "\nError (Line %u): Invalid number, variable or function '%s'.\n\n", cdata->c_linenum, ename);
					return 1;
				}
			}
			
			// Setup function call
			if (isexf)
			{
				// Setup function arguments
				if (nspex != root->east->spex) asmbspillx64_s(fout, cdata, reg_eimm, NULL);
				if (nspex != NULL)             callprocx64(fout, cdata, nspex->root);
				if (nspex != root->east->spex) asmbspillx64_l(fout, cdata, reg_eimm, NULL);
				
				// Update size and type based on function return
				if (!iseop) isevar = 1;/*
				root->east->type = 'i';
				root->east->size = 4;*/
			}
			
			// Setup values
			if (isevar || iseop)
			{
				if (root->east->ext != NULL)
				{
					struct offext *n;   	// Extension node pointer
					unsigned int c = 0; 	// Pointer level counter
					
					// Check if variable is pointer
					for (n = root->east->ext; (c < root->east->ptrlv) && (n->ext != NULL); ++c)
					{
						unsigned int offset = n->offset;
						
						// First run in loop
						if ((c == 0) && !hadexp)
						{
							// Check if reference
							if (root->east->isref)
							{
								if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, ename)->offset);
								else
								{
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_eptr, ename, n->offset * MAX_BYTESIZE_X64);
									else       	    fprintf(fout, "mov %s, [%s]\n",    reg_eptr, ename);
								}
								
								// Increment offset extension
								n = n->ext;
							}
							else
							{
								if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eptr, -vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_eptr, ename);
							}
						}
						
						// Subsequent runs in loop
						else
						{
							if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_eptr, reg_eptr, n->offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_eptr, reg_eptr);
							
							// Increment offset extension
							n = n->ext;
						}
					}
					
					// Update pointer level
					root->east->ptrlv -= c;
					
					// Check if loop has not run before
					if ((c == 0) && !hadexp)
					{
						// Setup offset variable (for optimization)
						unsigned int offset = n->offset;
						
						// Check if reference
						if (root->east->isref)
						{
							if (offset > 0)
							{
								if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov %s, [%s%+i]\n",  reg_eptr, ename, offset * MAX_BYTESIZE_X64);
							}
							else
							{
								if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eptr, -vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_eptr, ename);
							}
						}
						
						// Check if operator
						else if (iseop && (root->east->ptrlv == 0))
						{
							if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", ebitext, reg_eimm, esizedef, reg_eimm, offset * root->east->size);
							else            fprintf(fout, "mov%s %s, %s[%s]\n", ebitext, reg_eimm, esizedef, reg_eimm);
						}
						
						// Check if pointer/function
						else if (isevf || (root->east->ptrlv > 0))
						{
							root->east->ptrlv--;
							
							// Remove conversion of sizes
							if (root->east->ptrlv > 0) 
							{
								strcpy(ebitext, "");
								strcpy(esizedef, "");
								strcpy(reg_eimm, reg_emaxint);
							}
							
							// Check if variable
							if (isevar)
							{
								if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eptr, -vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov %s, [%s]\n",     reg_eptr, ename);
							}
							
							if (root->east->ptrlv > 0)
							{
								if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_eimm, reg_eptr, MAX_BYTESIZE_X64 * root->east->size);
								else            fprintf(fout, "mov %s, [%s]\n",    reg_eimm, reg_eptr);
							}
							else
							{
								if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", ebitext, reg_eimm, esizedef, reg_eptr, offset * root->east->size);
								else            fprintf(fout, "mov%s %s, %s[%s]\n",    ebitext, reg_eimm, esizedef, reg_eptr);
							}
						}
						
						// Check if array
						else if (root->east->isarr)
						{
							if (offset > 0)
							{
								if (isevs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", ebitext, reg_eimm, esizedef, (offset * root->east->size) - vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov%s %s, %s[%s%+i]\n",  ebitext, reg_eimm, esizedef, ename, (offset * root->east->size));
							}
							else
							{
								if (isevs) fprintf(fout, "mov%s %s, %s[rbp-%i]\n", ebitext, reg_eimm, esizedef, vdatafind(cdata->l_s, ename)->offset);
								else       fprintf(fout, "mov%s %s, %s[%s]\n",     ebitext, reg_eimm, esizedef, ename);
							}
						}
						
						else
						{
							fprintf(stderr, "\nError (Line %u): '%s' is neither an array, nor a pointer.\n\n", cdata->c_linenum, ename);
							return 1;
						}
					}
					
					else
					{
						unsigned int offset = (n != NULL) ? n->offset : 0;
						
						// Check if function
						if (isevf)
						{
							if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_eimm, reg_eptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_eimm, reg_eptr);
						}
						
						// Check if no longer a pointer
						if (root->east->ptrlv == 0)
						{
							if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", ebitext, reg_eimm, esizedef, reg_eptr, (offset * root->east->size));
							else            fprintf(fout, "mov%s %s, %s[%s]\n",    ebitext, reg_eimm, esizedef, reg_eptr);
						}
						
						// Check if still a pointer
						else
						{
							// Force source register to hold pointer
							strcpy(reg_esrc, reg_emaxint);
							strcpy(reg_eimm, reg_emaxint);
							
							puts("STILL A PTR");
							
							// This is a stub. Feel free to add anything later.
						}
					}
				}
				else if (iseop && (root->east->ptrlv > 0))
				{
					strcpy(reg_eimm, reg_emaxint);
				}
				else if (isexa)
				{
					
				}
				else if (isexf)
				{
					if (root->east->isref)
					{
						if (isevs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_eimm, -vdatafind(cdata->l_s, ename)->offset, reg_eimm);
						else       fprintf(fout, "call %s\n",                   ename);
					}
					else
					{
						if (isevs) fprintf(fout, "call qword [rbp%+i]\n", -vdatafind(cdata->l_s, ename)->offset);
						else       fprintf(fout, "call qword [%s]\n",     ename);
					}
				}
				else if (isexc)
				{
					
				}
				else if (isevar)
				{
					if (root->east->isref)
					{
						// Force source register to hold pointer
						strcpy(reg_esrc, reg_emaxint);
						strcpy(reg_eimm, reg_emaxint);
						
						if (isevs) fprintf(fout, "lea %s, [rbp%+i]\n", reg_eimm, -vdatafind(cdata->l_s, ename)->offset);
						else       fprintf(fout, "mov %s, %s\n",       reg_eimm, ename);
					}
					else if ((root->east->ptrlv > 0) || root->east->isarr)
					{
						// Force source register to hold pointer
						strcpy(reg_esrc, reg_emaxint);
						strcpy(reg_eimm, reg_emaxint);
						
						if (isevs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_eimm, -vdatafind(cdata->l_s, ename)->offset);
						else       fprintf(fout, "mov %s, [%s]\n",     reg_eimm, ename);
					}
					else
					{
						// Use XOR operation to remove data dependency
						if (isevp && (nspex == NULL)) fprintf(fout, "pxor %s, %s\n", reg_eimm, reg_eimm);
						
						if (isevs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", ebitext, reg_eimm, esizedef, -vdatafind(cdata->l_s, ename)->offset);
						else       fprintf(fout, "mov%s %s, %s[%s]\n",     ebitext, reg_eimm, esizedef, ename);
					}
				}
			}
		}
		//while ((root->east->ext != NULL) || (nspex != NULL));
		
		// Check if both variables are still pointers
		if ((nodelex(root) != OP_MOV) && ((root->west->ptrlv > 0) && (root->east->ptrlv > 0)))
			fprintf(stderr, "\nWarning (Line %u): Performing a non-assignment operation with multiple pointers is not recommended.\n\n", cdata->c_linenum);
		
		// Resolve immediate register types for root node
		if (iswvi || isevi) root->type = 'i';
		else                root->type = 'u';
		
		if (iswvp || isevp
		|| (iswop && root->west->type == 'p')
		|| (iseop && root->east->type == 'p'))
		{
			// Set root node to precise number
			root->type = 'p';
			
			// Setup immediate register types of west and east nodes
			if (root->west->ptrlv == 0)
			if (iswvp || (iswop && root->west->type == 'p'))
			{
				if ((isevar || iseop) && (root->east->type != 'p'))
				{
					if (altereg) {fputs("pxor xmm2, xmm2\n", fout); fprintf(fout, "cvtsi2%s xmm2, %s\n", wbitext, reg_eimm); strcpy(reg_eimm, "xmm2"); strcpy(reg_esrc, "xmm2");}
					else         {fputs("pxor xmm1, xmm1\n", fout); fprintf(fout, "cvtsi2%s xmm1, %s\n", wbitext, reg_eimm); strcpy(reg_eimm, "xmm1"); strcpy(reg_esrc, "xmm1");}
					
					strcpy(eopext, wopext);
					strcpy(ebitext, wbitext);
				}
			}
			
			if (root->east->ptrlv == 0)
			if (isevp || (iseop && root->east->type == 'p'))
			{
				if ((iswvar || iswop) && (root->west->type != 'p'))
				{
					printf("BYE %i %i %i\n", altwreg, !isroot, nodelex(root)/8 != OP_GROUP_EQUAL);
					
					if (altwreg) {fputs("pxor xmm2, xmm2\n", fout); fprintf(fout, "cvtsi2%s xmm2, %s\n", ebitext, reg_wimm); strcpy(reg_wimm, "xmm2"); strcpy(reg_wsrc, "xmm2");}
					else         {fputs("pxor xmm0, xmm0\n", fout); fprintf(fout, "cvtsi2%s xmm0, %s\n", ebitext, reg_wimm); strcpy(reg_wimm, "xmm0"); strcpy(reg_wsrc, "xmm0");}
					
					strcpy(wopext, eopext); 
					strcpy(wbitext, ebitext);
				}
			}
		}
		else root->type = root->west->type;
		
		//fprintf(stderr, "; LOL '%4s' %4s '%4s' \t%i, %i (%i, %i) (%s, %s)\n", wname, root->string, ename, root->west->size, root->east->size, root->west->ptrlv, root->east->ptrlv, wbitext, ebitext);
		
		// Resolve immediate register sizes for root node
		if ((((root->west->size != root->east->size)
		&&  ((root->west->size == 8) || (root->east->size == 8)))
		||  ((root->west->ptrlv > 0) || (root->east->ptrlv > 0))))
		{
			//printf("WOW %c%ip%i %c%ip%i (%s)\n", root->west->type, root->west->size, root->west->ptrlv, root->east->type, root->east->size, root->east->ptrlv, root->string);
			
			// Check if precise number
			if (root->type == 'p')
			{
				root->size = root->west->size;
				
				// Check if both nodes were originally precise numbers
				if ((root->west->type == 'p')
				&&  (root->east->type == 'p'))
				{
					// Check if west node is qword sized or a pointer
					if (((root->west->size < 8) && (root->east->size == 8))
					|| ((root->west->ptrlv == 0) && (root->east->ptrlv > 0)))
					{
						if (root->east->ptrlv > 0) root->ptrlv = root->east->ptrlv;
						fprintf(fout, "cvt%s2%s %s, %s\n", wbitext, ebitext, reg_wimm, reg_wimm);
					} 
					
					// Check if east node is qword sized or a pointer
					else
					if (((root->east->size < 8) && (root->west->size == 8))
					|| ((root->east->ptrlv == 0) && (root->west->ptrlv > 0)))
					{
						if (root->west->ptrlv > 0) root->ptrlv = root->west->ptrlv;
						fprintf(fout, "cvt%s2%s %s, %s\n", ebitext, wbitext, reg_eimm, reg_eimm);
					}
				}
			}
			else
			{
				// Check if neither are pointers
				if ((root->west->ptrlv == 0) && (root->east->ptrlv == 0)) root->size = 8;
				else                                                      root->size = root->west->size;
				
				// Check if west node is qword sized or a pointer
				if (((root->west->size < 8)  && (root->east->size == 8))
				|| ((root->west->ptrlv == 0) && (root->east->ptrlv > 0)))
				{
					strcpy(reg_wsrc, reg_wmaxint);
					
					if (root->east->ptrlv > 0) {strcpy(reg_wimm, reg_wmaxint); root->ptrlv = root->east->ptrlv;}
					else                       {strcpy(reg_wimm, reg_wmaximm);}
					
					strcpy(wsizedef, "");
				} 
				
				// Check if east node is qword sized or a pointer
				else
				if (((root->east->size < 8)  && (root->west->size == 8))
				|| ((root->east->ptrlv == 0) && (root->west->ptrlv > 0)))
				{
					strcpy(reg_esrc, reg_emaxint);
					
					if (root->west->ptrlv > 0) {strcpy(reg_eimm, reg_emaxint); root->ptrlv = root->west->ptrlv;}
					else                       {strcpy(reg_eimm, reg_emaximm);}
					
					strcpy(esizedef, "");
				}
			}
			
			//printf("OWO %c%ip%i\n", root->type, root->size, root->ptrlv);
		}
		else root->size = root->west->size;
		
		if ((iswvar || iswop) && (isevar || iseop))
		switch (root->type)
		{
			case 'i':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						fprintf(fout, "imul %s, %s\n", reg_wimm, reg_eimm);
						break;
					case OP_MOD:
						fprintf(fout, "cdq\nidiv %s\nmov %s, edx\n", reg_eimm, reg_wimm);
						break;
					case OP_DIV:
						fprintf(fout, "cdq\nidiv %s\n", reg_eimm);
						break;
					case OP_ADD:
						fprintf(fout, "add %s, %s\n", reg_wimm, reg_eimm);
						break;
					case OP_SUB:
						fprintf(fout, "sub %s, %s\n", reg_wimm, reg_eimm);
						break;
					
					// Shift operators
					case OP_SHL:
						if (altereg) fputs("mov ecx, r10d", fout);
						fprintf(fout, "sal %s, cl\n", reg_wimm); break;
						break;
					case OP_SHR:
						if (altereg) fputs("mov ecx, r10d", fout);
						fprintf(fout, "sar %s, cl\n", reg_wimm);
						break;
					
					// Equality operators
					case OP_MOV:
						fprintf(fout, "mov [%s], %s\n", reg_wptr, reg_esrc);
						break;
					case OP_MOVM:
						fprintf(fout, "imul %s, [%s]\nmov [%s], %s\n", reg_eimm, reg_wptr, reg_wptr, reg_esrc);
						break;
					case OP_MOVD:
						fprintf(fout, "mov%s %s, %s[%s]\ncdq\nidiv %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVA:
						fprintf(fout, "mov%s %s, %s[%s]\nadd %s, %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_wimm, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVS:
						fprintf(fout, "mov%s %s, %s[%s]\nsub %s, %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_wimm, reg_eimm, reg_wptr, reg_wsrc);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "cmp %s, %s\nsetg al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_LTHN:
						fprintf(fout, "cmp %s, %s\nsetl al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_GEQU:
						fprintf(fout, "cmp %s, %s\nsetge al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_LEQU:
						fprintf(fout, "cmp %s, %s\nsetle al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_EQU:
						fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_NEQU:
						fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
				}
				break;
				
			case 'u':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						fprintf(fout, "imul %s, %s\n", reg_wimm, reg_eimm);
						break;
					case OP_MOD:
						fprintf(fout, "xor edx, edx\ndiv %s\nmov %s, edx\n", reg_eimm, reg_wimm);
						break;
					case OP_DIV:
						fprintf(fout, "xor edx, edx\ndiv %s\n", reg_eimm);
						break;
					case OP_ADD:
						fprintf(fout, "add %s, %s\n", reg_wimm, reg_eimm);
						break;
					case OP_SUB:
						fprintf(fout, "sub %s, %s\n", reg_wimm, reg_eimm);
						break;
					
					// Shift operators
					case OP_SHL:
						if (altereg) fputs("mov ecx, r10d", fout);
						fprintf(fout, "shl %s, cl\n", reg_wimm);
						break;
					case OP_SHR:
						if (altereg) fputs("mov ecx, r10d", fout);
						fprintf(fout, "shr %s, cl\n", reg_wimm);
						break;
					
					// Equality operators
					case OP_MOV:
						fprintf(fout, "mov [%s], %s\n", reg_wptr, reg_esrc);
						break;
					case OP_MOVM:
						fprintf(fout, "imul %s, [%s]\nmov [%s], %s\n", reg_eimm, reg_wptr, reg_wptr, reg_esrc);
						break;
					case OP_MOVD:
						fprintf(fout, "mov%s %s, %s[%s]\ncdq\nidiv %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVA:
						fprintf(fout, "mov%s %s, %s[%s]\nadd %s, %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_wimm, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVS:
						fprintf(fout, "mov%s %s, %s[%s]\nsub %s, %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_wimm, reg_eimm, reg_wptr, reg_wsrc);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "cmp %s, %s\nseta al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_LTHN:
						fprintf(fout, "cmp %s, %s\nsetb al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_GEQU:
						fprintf(fout, "cmp %s, %s\nsetae al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_LEQU:
						fprintf(fout, "cmp %s, %s\nsetbe al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_EQU:
						fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
					case OP_NEQU:
						fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, reg_eimm);
						break;
				}
				break;
				
			case 'p':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						fprintf(fout, "mul%s %s, %s\n", wopext, reg_wimm, reg_eimm);
						break;
					case OP_MOD:
						fprintf(stderr, "\nError (Line %u): Finding the remainder of precise numbers cannot be performed.\n\n", cdata->c_linenum);
						return 1;
					case OP_DIV:
						fprintf(fout, "div%s %s, %s\n", wopext, reg_wimm, reg_eimm);
						break;
					case OP_ADD:
						fprintf(fout, "add%s %s, %s\n", wopext, reg_wimm, reg_eimm);
						break;
					case OP_SUB:
						fprintf(fout, "sub%s %s, %s\n", wopext, reg_wimm, reg_eimm);
						break;
					
					// Shift operators
					case OP_SHL:
					case OP_SHR:
						fprintf(stderr, "\nError (Line %u): Shifting bits of precise numbers cannot be performed.\n\n", cdata->c_linenum);
						return 1;
					
					// Equality operators
					case OP_MOV:
						fprintf(fout, "mov%s [%s], %s\n", wbitext, reg_wptr, reg_esrc);
						break;
					case OP_MOVM:
						fprintf(fout, "mul%s %s, %s[%s]\nmov%s [%s], %s\n", wopext, reg_eimm, wsizedef, reg_wptr, wbitext, reg_wptr, reg_esrc);
						break;
					case OP_MOVD:
						fprintf(fout, "div%s %s, %s[%s]\nmov%s [%s], %s\n", wopext, reg_eimm, wsizedef, reg_wptr, wbitext, reg_wptr, reg_esrc);
						break;
					case OP_MOVA:
						fprintf(fout, "add%s %s, %s[%s]\nmov%s [%s], %s\n", wopext, reg_eimm, wsizedef, reg_wptr, wbitext, reg_wptr, reg_esrc);
						break;
					case OP_MOVS:
						fprintf(fout, "sub%s %s, %s[%s]\nmov%s [%s], %s\n", wopext, reg_eimm, wsizedef, reg_wptr, wbitext, reg_wptr, reg_esrc);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "ucomi%s %s, %s\nseta al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, reg_eimm);
						break;
					case OP_LTHN:
						fprintf(fout, "ucomi%s %s, %s\nsetb al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, reg_eimm);
						break;
					case OP_GEQU:
						fprintf(fout, "ucomi%s %s, %s\nsetae al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, reg_eimm);
						break;
					case OP_LEQU:
						fprintf(fout, "ucomi%s %s, %s\nsetbe al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, reg_eimm);
						break;
					case OP_EQU:
						fprintf(fout, "ucomi%s %s, %s\nsete al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, reg_eimm);
						break;
					case OP_NEQU:
						fprintf(fout, "ucomi%s %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, reg_eimm);
						break;
				}
				break;
		}
		
		else if (iswvar || iswop)
		switch (root->type)
		{
			case 'i':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						if ((shiftdiff(stoi(ename)) >= -3) && (shiftdiff(stoi(ename)) <= 3))
						{
							fprintf(fout, "mov %s, %s\nsal %s, %i\n", reg_eimm, reg_wimm, reg_wimm, shiftbase(stoi(ename)));
							
							if (shiftdiff(stoi(ename)) > 0)
							for (int i = 0; i <  shiftdiff(stoi(ename)); ++i)
								fprintf(fout, "add %s, %s\n", reg_wimm, reg_eimm);
							
							if (shiftdiff(stoi(ename)) < 0)
							for (int i = 0; i < -shiftdiff(stoi(ename)); ++i)
								fprintf(fout, "sub %s, %s\n", reg_wimm, reg_eimm);
						}
						else fprintf(fout, "imul %s, %s\n", reg_wimm, ename);
						break;
					case OP_MOD:
						if (shiftdiff(stoi(ename)) == 0) fprintf(fout, "mov %s, 0\n", reg_wimm);
						else                              fprintf(fout, "mov %s, %s\ncdq\nidiv %s\nmov %s, edx\n", reg_eimm, ename, reg_eimm, reg_wimm);
						break;
					case OP_DIV:
						if (shiftdiff(stoi(ename)) == 0) fprintf(fout, "sar %s, %i\n", reg_wimm, shiftbase(stoi(ename)));
						else                              fprintf(fout, "mov %s, %s\ncdq\nidiv %s\n", reg_eimm, ename, reg_eimm);
						break;
					case OP_ADD:
						if (stoi(ename) == 1) fprintf(fout, "inc %s\n", reg_wimm);
						else                   fprintf(fout, "add %s, %s\n", reg_wimm, ename);
						break;
					case OP_SUB:
						if (stoi(ename) == 1) fprintf(fout, "dec %s\n", reg_wimm);
						else                   fprintf(fout, "sub %s, %s\n", reg_wimm, ename);
						break;
					
					// Shift operators
					case OP_SHL:
						fprintf(fout, "sal %s, %s\n", reg_wimm, ename);
						break;
					case OP_SHR:
						fprintf(fout, "sar %s, %s\n", reg_wimm, ename);
						break;
					
					// Equality operators
					case OP_MOV:
						if (root->west->size == 8) fprintf(fout, "mov %s, %s\nmov [%s], %s\n", reg_wimm, ename, reg_wptr, reg_wsrc);
						else                       fprintf(fout, "mov [%s], %s%s\n", reg_wptr, wsizedef, ename);
						break;
					case OP_MOVM:
						if (root->west->size == 8) fprintf(fout, "imul %s, [%s], %s\nmov [%s], %s\n",   reg_wimm, reg_wptr, ename, reg_wptr, reg_wsrc);
						else                       fprintf(fout, "imul %s, [%s], %s%s\nmov [%s], %s\n", reg_wimm, reg_wptr, wsizedef, ename, reg_wptr, reg_wsrc);
						break;
					case OP_MOVD:
						fprintf(fout, "mov%s %s, %s[%s]\nmov %s, %s\ncdq\nidiv %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_eimm, ename, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVA:
						if (stoi(ename) > 1) fprintf(fout, "add [%s], %s%s\n", reg_wptr, wsizedef, ename);
						else                  fprintf(fout, "inc [%s]\n", reg_wptr);
						break;
					case OP_MOVS:
						if (stoi(ename) > 1) fprintf(fout, "sub [%s], %s%s\n", reg_wptr, wsizedef, ename);
						else                  fprintf(fout, "dec [%s]\n", reg_wptr);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "cmp %s, %s\nsetg al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_LTHN:
						fprintf(fout, "cmp %s, %s\nsetl al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_GEQU:
						fprintf(fout, "cmp %s, %s\nsetge al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_LEQU:
						fprintf(fout, "cmp %s, %s\nsetle al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_EQU:
						fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_NEQU:
						fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					
					// Logical operators
					
				}
				break;
				
			case 'u':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						if ((shiftdiff(stoi(ename)) >= -3) && (shiftdiff(stoi(ename)) <= 3))
						{
							fprintf(fout, "mov %s, %s\nshl %s, %i\n", reg_eimm, reg_wimm, reg_wimm, shiftbase(stoi(ename)));
							
							if (shiftdiff(stoi(ename)) > 0)
							for (int i = 0; i <  shiftdiff(stoi(ename)); ++i)
								fprintf(fout, "add %s, %s\n", reg_wimm, reg_eimm);
							
							if (shiftdiff(stoi(ename)) < 0)
							for (int i = 0; i < -shiftdiff(stoi(ename)); ++i)
								fprintf(fout, "sub %s, %s\n", reg_wimm, reg_eimm);
						}
						else fprintf(fout, "imul %s, %s\n", reg_wimm, ename);
						break;
					case OP_MOD:
						if (shiftdiff(stoi(ename)) == 0) fprintf(fout, "mov %s, 0\n", reg_wimm);
						else                              fprintf(fout, "mov %s, %s\nxor edx, edx\ndiv %s\nmov %s, edx\n", reg_eimm, ename, reg_eimm, reg_wimm);
						break;
					case OP_DIV:
						if (shiftdiff(stoi(ename)) == 0) fprintf(fout, "shr %s, %i\n", reg_wimm, shiftbase(stoi(ename)));
						else                              fprintf(fout, "mov %s, %s\nxor edx, edx\ndiv %s\n", reg_eimm, ename, reg_eimm);
						break;
					case OP_ADD:
						if (stoi(ename) == 1) fprintf(fout, "inc %s\n", reg_wimm);
						else                   fprintf(fout, "add %s, %s\n", reg_wimm, ename);
						break;
					case OP_SUB:
						if (stoi(ename) == 1) fprintf(fout, "dec %s\n", reg_wimm);
						else                   fprintf(fout, "sub %s, %s\n", reg_wimm, ename);
						break;
					
					// Shift operators
					case OP_SHL:
						fprintf(fout, "shl %s, %s\n", reg_wimm, ename);
						break;
					case OP_SHR:
						fprintf(fout, "shr %s, %s\n", reg_wimm, ename);
						break;
					
					// Equality operators
					case OP_MOV:
						if (root->west->size == 8) fprintf(fout, "mov %s, %s\nmov [%s], %s\n", reg_wimm, ename, reg_wptr, reg_wsrc);
						else                       fprintf(fout, "mov [%s], %s%s\n", reg_wptr, wsizedef, ename);
						break;
					case OP_MOVM:
						if (root->west->size == 8) fprintf(fout, "imul %s, [%s], %s\nmov [%s], %s\n",   reg_wimm, reg_wptr, ename, reg_wptr, reg_wsrc);
						else                       fprintf(fout, "imul %s, [%s], %s%s\nmov [%s], %s\n", reg_wimm, reg_wptr, wsizedef, ename, reg_wptr, reg_wsrc);
						break;
					case OP_MOVD:
						fprintf(fout, "mov%s %s, %s[%s]\nmov %s, %s\ncdq\nidiv %s\nmov [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, reg_eimm, ename, reg_eimm, reg_wptr, reg_wsrc);
						break;
					case OP_MOVA:
						if (stoi(ename) > 1) fprintf(fout, "add [%s], %s%s\n", reg_wptr, wsizedef, ename);
						else                  fprintf(fout, "inc [%s]\n", reg_wptr);
						break;
					case OP_MOVS:
						if (stoi(ename) > 1) fprintf(fout, "sub [%s], %s%s\n", reg_wptr, wsizedef, ename);
						else                  fprintf(fout, "dec [%s]\n", reg_wptr);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "cmp %s, %s\nseta al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_LTHN:
						fprintf(fout, "cmp %s, %s\nsetb al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_GEQU:
						fprintf(fout, "cmp %s, %s\nsetae al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_LEQU:
						fprintf(fout, "cmp %s, %s\nsetbe al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_EQU:
						fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					case OP_NEQU:
						fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
						break;
					
					// Logical operators
					
				}
				break;
				
			case 'p':
				switch (nodelex(root))
				{
					// Arithmetic operators
					case OP_MUL:
						fprintf(fout, "mul%s %s, %s\n", wopext, reg_wimm, ename);
						break;
					case OP_MOD:
						fprintf(stderr, "\nError (Line %u): Finding the remainder of precise numbers cannot be performed.\n\n", cdata->c_linenum);
						return 1;
					case OP_DIV:
						fprintf(fout, "div%s %s, %s\n", wopext, reg_wimm, ename);
						break;
					case OP_ADD:
						fprintf(fout, "add%s %s, %s\n", wopext, reg_wimm, ename);
						break;
					case OP_SUB:
						fprintf(fout, "sub%s %s, %s\n", wopext, reg_wimm, ename);
						break;
					
					// Shift operators
					case OP_SHL:
					case OP_SHR:
						fprintf(stderr, "\nError (Line %u): Shifting bits of precise numbers cannot be performed.\n\n", cdata->c_linenum);
						return 1;
					
					// Equality operators
					case OP_MOV:
						fprintf(fout, "mov%s %s, %s%s\nmov%s [%s], %s\n", wbitext, reg_wimm, wsizedef, ename, wbitext, reg_wptr, reg_wsrc);
						break;
					case OP_MOVM:
						fprintf(fout, "mul%s %s, [%s], %s%s\nmov [%s], %s\n", wbitext, reg_wimm, reg_wptr, wsizedef, ename, reg_wptr, reg_wsrc);
						break;
					case OP_MOVD:
						fprintf(fout, "mov%s %s, %s[%s]\nmov%s %s, %s%s\ncdq\ndiv%s %s, %s\nmov%s [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, wbitext, reg_eimm, esizedef, ename, wopext, reg_wimm, reg_eimm, wbitext, reg_wptr, reg_wsrc);
						break;
					case OP_MOVA:
						fprintf(fout, "mov%s %s, %s[%s]\nadd%s %s, %s\nmov%s [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, wopext, reg_wimm, ename, wbitext, reg_wptr, reg_wsrc);
						break;
					case OP_MOVS:
						fprintf(fout, "mov%s %s, %s[%s]\nsub%s %s, %s\nmov%s [%s], %s\n", wbitext, reg_wimm, wsizedef, reg_wptr, wopext, reg_wimm, ename, wbitext, reg_wptr, reg_wsrc);
						break;
					
					// Conditional operators
					case OP_GTHN:
						fprintf(fout, "ucomi%s %s, %s%s\nseta al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, wsizedef, ename);
						break;
					case OP_LTHN:
						fprintf(fout, "ucomi%s %s, %s%s\nsetb al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, wsizedef, ename);
						break;
					case OP_GEQU:
						fprintf(fout, "ucomi%s %s, %s%s\nsetae al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, wsizedef, ename);
						break;
					case OP_LEQU:
						fprintf(fout, "ucomi%s %s, %s%s\nsetbe al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, wsizedef, ename);
						break;
					case OP_EQU:
						fprintf(fout, "ucomi%s %s, %s%s\nsete al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, wsizedef, ename);
						break;
					case OP_NEQU:
						fprintf(fout, "ucomi%s %s, %s%s\nsetne al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, wsizedef, ename);
						break;
					
					// Logical operators
					
				}
				break;
		}
		
		else if (isevar || iseop)
		{
			switch (root->type)
			{
				case 'i':
					switch (nodelex(root))
					{
						// Arithmetic operators
						case OP_MUL:
							if ((shiftdiff(stoi(wname)) >= -3) && (shiftdiff(stoi(wname)) <= 3))
							{
								fprintf(fout, "mov %s, %s\nsal %s, %i\n", reg_wimm, reg_eimm, reg_eimm, shiftbase(stoi(wname)));
								
								if (shiftdiff(stoi(wname)) > 0)
								for (int i = 0; i <  shiftdiff(stoi(wname)); ++i)
									fprintf(fout, "add %s, %s\n", reg_eimm, reg_wimm);
								
								if (shiftdiff(stoi(wname)) < 0)
								for (int i = 0; i < -shiftdiff(stoi(wname)); ++i)
									fprintf(fout, "sub %s, %s\n", reg_eimm, reg_wimm);
							}
							else fprintf(fout, "imul %s, %s\n", reg_eimm, wname);
							break;
						case OP_DIV:
							fprintf(fout, "mov %s, %s\ncdq\nidiv %s\n", reg_wimm, wname, reg_eimm);
							break;
						case OP_ADD:
							if (stoi(ename) == 1) fprintf(fout, "inc %s\n", reg_eimm);
							else                   fprintf(fout, "add %s, %s\n", reg_eimm, wname);
							break;
						case OP_SUB:
							if (stoi(ename) == 1) fprintf(fout, "dec %s\n", reg_eimm);
							else                   fprintf(fout, "sub %s, %s\n", reg_eimm, wname);
							break;
						
						// Equality operators
						case OP_MOV:
						case OP_MOVM:
						case OP_MOVD:
						case OP_MOVA:
						case OP_MOVS:
							fprintf(stderr, "\nError (Line %u): Whoever came up with this brilliant idea to use a constant value as the destination address should sip a bit of coffee.\n\n", cdata->c_linenum);
							return 1;
						
						// Conditional operators
						case OP_GTHN:
							fprintf(fout, "cmp %s, %s\nsetg al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_LTHN:
							fprintf(fout, "cmp %s, %s\nsetl al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_GEQU:
							fprintf(fout, "cmp %s, %s\nsetge al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_LEQU:
							fprintf(fout, "cmp %s, %s\nsetle al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_EQU:
							fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_NEQU:
							fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
					}
					break;
					
				case 'u':
					switch (nodelex(root))
					{
						// Arithmetic operators
						case OP_MUL:
							if ((shiftdiff(stoi(wname)) >= -3) && (shiftdiff(stoi(wname)) <= 3))
							{
								fprintf(fout, "mov %s, %s\nshl %s, %i\n", reg_wimm, reg_eimm, reg_eimm, shiftbase(stoi(wname)));
								
								if (shiftdiff(stoi(wname)) > 0)
								for (int i = 0; i <  shiftdiff(stoi(wname)); ++i)
									fprintf(fout, "add %s, %s\n", reg_eimm, reg_wimm);
								
								if (shiftdiff(stoi(wname)) < 0)
								for (int i = 0; i < -shiftdiff(stoi(wname)); ++i)
									fprintf(fout, "sub %s, %s\n", reg_eimm, reg_wimm);
							}
							else fprintf(fout, "imul %s, %s\n", reg_eimm, wname);
							break;
						case OP_DIV:
							fprintf(fout, "mov %s, %s\nxor edx, edx\nidiv %s\n", reg_wimm, wname, reg_eimm);
							break;
						case OP_ADD:
							if (stoi(ename) == 1) fprintf(fout, "inc %s\n", reg_eimm);
							else                   fprintf(fout, "add %s, %s\n", reg_eimm, wname);
							break;
						case OP_SUB:
							if (stoi(ename) == 1) fprintf(fout, "dec %s\n", reg_eimm);
							else                   fprintf(fout, "sub %s, %s\n", reg_eimm, wname);
							break;
						
						// Equality operators
						case OP_MOV:
						case OP_MOVM:
						case OP_MOVD:
						case OP_MOVA:
						case OP_MOVS:
							fprintf(stderr, "\nError (Line %u): Whoever came up with this brilliant idea to use a constant value as the destination address should sip a bit of coffee.\n\n", cdata->c_linenum);
							return 1;
						
						// Conditional operators
						case OP_GTHN:
							fprintf(fout, "cmp %s, %s\nseta al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_LTHN:
							fprintf(fout, "cmp %s, %s\nsetb al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_GEQU:
							fprintf(fout, "cmp %s, %s\nsetae al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_LEQU:
							fprintf(fout, "cmp %s, %s\nsetbe al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_EQU:
							fprintf(fout, "cmp %s, %s\nsete al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
						case OP_NEQU:
							fprintf(fout, "cmp %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", reg_wimm, ename);
							break;
					}
					break;
					
				case 'p':
					switch (nodelex(root))
					{
						// Arithmetic operators
						case OP_MUL:
							fprintf(fout, "mul%s %s, %s\n", eopext, reg_eimm, wname);
							break;
						case OP_MOD:
							fprintf(stderr, "\nError (Line %u): Finding the remainder of precise numbers cannot be performed.\n\n", cdata->c_linenum);
							return 1;
						case OP_DIV:
							fprintf(fout, "mov%s %s, %s\ndiv%s %s, %s\n", eopext, reg_wimm, wname, eopext, reg_wimm, reg_eimm);
							break;
						case OP_ADD:
							fprintf(fout, "add%s %s, %s\n", eopext, reg_eimm, wname);
							break;
						case OP_SUB:
							fprintf(fout, "mov%s %s, %s\nsub%s %s, %s\n", wbitext, reg_wimm, wname, eopext, reg_wimm, reg_eimm);
							break;
						
						// Shift operators
						case OP_SHL:
						case OP_SHR:
							fprintf(stderr, "\nError (Line %u): Shifting bits of precise numbers cannot be performed.\n\n", cdata->c_linenum);
							return 1;
						
						// Equality operators
						case OP_MOV:
						case OP_MOVM:
						case OP_MOVD:
						case OP_MOVA:
						case OP_MOVS:
							fprintf(stderr, "\nError (Line %u): Whoever came up with this brilliant idea to use a constant value as the destination address should sip a bit of coffee.\n\n", cdata->c_linenum);
							return 1;
						
						// Conditional operators
						case OP_GTHN:
							fprintf(fout, "ucomi%s %s, %s\nseta al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, ename);
							break;
						case OP_LTHN:
							fprintf(fout, "ucomi%s %s, %s\nsete al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, ename);
							break;
						case OP_GEQU:
							fprintf(fout, "ucomi%s %s, %s\nsetae al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, ename);
							break;
						case OP_LEQU:
							fprintf(fout, "ucomi%s %s, %s\nsetbe al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, ename);
							break;
						case OP_EQU:
							fprintf(fout, "ucomi%s %s, %s\nsete al\nand al, 1\nmovzx eax, al\n",  wopext, reg_wimm, ename);
							break;
						case OP_NEQU:
							fprintf(fout, "ucomi%s %s, %s\nsetne al\nand al, 1\nmovzx eax, al\n", wopext, reg_wimm, ename);
							break;
					}
					break;
			}
			
			if (iseop) fprintf(fout, "mov%s %s, %s\n", wbitext, reg_wimm, reg_eimm);
		}
		else fprintf(stderr, "\nWarning (Line %u): Neither '%s' nor '%s' can be identified as a variable or operator. No assembly code shall be generated for this operation.\n\n", cdata->c_linenum, wname, ename);
		
		// Check if not actually root
		if (isroot && (troot != root))
			fprintf(fout, "\nmov%s %s, %s\n", wbitext, reg_eimm, reg_wimm);
		
		// Change type if conditional or logical operator
		
		if ((nodelex(root) / 8) <= OP_GROUP_COMPS)
		{
			root->type = 'i';
			root->size = 4;
		}
		
		return 0;
	}
	
	// Check if root
	else if (root->root == NULL)
	{
		// Variable name
		char 
		*name = root->string,
		//spexstr[STRING_LENGTH_VARDATA],
		reg_maxint[STRING_LENGTH_REGISTER];
		
		// Setup register for max byte size
		strcpy(reg_maxint, "rax");
		
		int
		isint = isinteger(name),	// Check if integer
		isprc = isprecise(name),	// Check if precise number
		
		// Variable type
		isvf = ((vdatafind(cdata->l_f, name) != NULL) || (nodeisfun(root))),
		isvi = (vdatafind(cdata->l_i, name) != NULL),
		isvu = (vdatafind(cdata->l_u, name) != NULL),
		isvp = (vdatafind(cdata->l_p, name) != NULL),
		isvs = (vdatafind(cdata->l_s, name) != NULL),
		
		// Check if variable
		isvar = (isvf || isvi || isvu || isvp);
		
		// Special extension node
		struct node *nspex;
		
		// Flags for special extensions
		int
		hadex = 0,
		hadexp = 0,
		isexa = 0,
		isexf = 0,
		isexc = 0;
		
		// Setup variable type
		if ((root->type != 'i')
		&&  (root->type != 'u')
		&&  (root->type != 'p'))
		{
			// Check if integer
			if (isint) root->type = 'i'; else
			if (isprc) root->type = 'p';
			
			// Check if variable
			else
			{
				if (vdatafind(cdata->l_i, name) != NULL) root->type = 'i';
				if (vdatafind(cdata->l_u, name) != NULL) root->type = 'u';
				if (vdatafind(cdata->l_p, name) != NULL) root->type = 'p';
			}
		}
		
		// Variable size
		if (root->size == 0)
		{
			// Check if integer
			if (isint)
			{
				long long unsigned int nsize = stoi(root->string);
				if (nsize > 0xffffffff) root->size = 8;
				else                    root->size = 4;
			}
			
			// Check if precise
			else if (isprc)
			{
				switch (root->string[strlen(root->string) - 1])
				{
					case 'f': root->size = 4; break;
					case 'd': root->size = 8; break;
					default:  root->size = 8; break;
				}
			}
			
			// Check if variable
			else if (isvar)
			{
				if (vdatafind(cdata->l_i, name) != NULL) root->size = vdatafind(cdata->l_i, name)->size;
				if (vdatafind(cdata->l_u, name) != NULL) root->size = vdatafind(cdata->l_u, name)->size;
				if (vdatafind(cdata->l_p, name) != NULL) root->size = vdatafind(cdata->l_p, name)->size;
			}
		}
		
		// Strings for assembly code
		char
		reg_ptr[STRING_LENGTH_REGISTER] = "",  	// West pointer register    	(to hold address)
		reg_imm[STRING_LENGTH_REGISTER] = "",  	// West immediate register  	(to hold value)
		reg_src[STRING_LENGTH_REGISTER] = "",  	// West source register     	(to store value)
		sizedef[7] = "",                       	// West size operand        	(i.e. dword, byte)
		bitext[12] = "",                       	// West 'mov' extension     	(i.e. movsx, movzx)
		opext[12] = "";                        	// West operation extension 	(i.e. addss, subsd)
		
		// Get the pointer levels, and array state
		if (isvar)
		{
			if (isvi) {root->ptrlv = vdatafind(cdata->l_i, name)->ptrlv; root->isarr = vdatafind(cdata->l_i, name)->isarr;}
			if (isvu) {root->ptrlv = vdatafind(cdata->l_u, name)->ptrlv; root->isarr = vdatafind(cdata->l_u, name)->isarr;}
			if (isvp) {root->ptrlv = vdatafind(cdata->l_p, name)->ptrlv; root->isarr = vdatafind(cdata->l_p, name)->isarr;}
		}
		
		// Setup special extension node
		nspex = root->spex;
		
		// Reset extension history flags
		hadex = 0;
		hadexp = 0;
		
		// Loop for setting up values while accomodating special extensions
		do
		{
			// Reset special extension flags
			isexa = 0;
			isexf = 0;
			isexc = 0;
			
			// Setup special extension flags
			if (nspex != NULL)
			{
				isexa = (nspex->string[0] == '[');  	// Check if array index
				isexf = (nspex->string[0] == '(');  	// Check if function call
				isexc = (nspex->string[0] == '{');  	// Check if variable cast
				
				// Check if variable had extension from the past
				hadexp = hadex;
				if (hadex == 0) hadex = ((root->ext != NULL) || (nspex != NULL));
			}
			
			// Setup assembly-related strings
			if (isvf)
			{
				strcpy(reg_ptr, reg_maxint);
				strcpy(reg_imm, reg_maxint);
			}
			else if (isvar)
			{
				// Set pointer register
				strcpy(reg_ptr, "rdx");
				
				switch (root->type)
				{
					case 'i':
						// Set immediate and source register
						switch (root->size)
						{
							case 8: strcpy(reg_imm, "rax"); strcpy(reg_src, "rax"); strcpy(sizedef, "");       break;
							case 4: strcpy(reg_imm, "eax"); strcpy(reg_src, "eax"); strcpy(sizedef, "dword "); break;
							case 2: strcpy(reg_imm, "eax"); strcpy(reg_src, "ax");  strcpy(sizedef, "word ");  strcpy(bitext, "sx"); break;
							case 1: strcpy(reg_imm, "eax"); strcpy(reg_src, "al");  strcpy(sizedef, "byte ");  strcpy(bitext, "sx"); break;
						}
						break;
					case 'u':
						// Set immediate and source register
						switch (root->size)
						{
							case 8: strcpy(reg_imm, "rax"); strcpy(reg_src, "rax"); strcpy(sizedef, "");       break;
							case 4: strcpy(reg_imm, "eax"); strcpy(reg_src, "eax"); strcpy(sizedef, "dword "); break;
							case 2: strcpy(reg_imm, "eax"); strcpy(reg_src, "ax");  strcpy(sizedef, "word ");  strcpy(bitext, "zx"); break;
							case 1: strcpy(reg_imm, "eax"); strcpy(reg_src, "al");  strcpy(sizedef, "byte ");  strcpy(bitext, "zx"); break;
						}
						break;
					case 'p':
						strcpy(bitext, "abs");
						strcpy(reg_imm, "xmm0");
						strcpy(reg_src, "xmm0");
						strcpy(sizedef, "");
						
						// Set immediate and source register
						switch (root->size)
						{
							case 8: strcpy(bitext, "sd"); strcpy(opext, "sd"); break;
							case 4: strcpy(bitext, "ss"); strcpy(opext, "ss"); break;
						}
						break;
				}
			}
			else if (isprc)
			{
				// Set immediate and source register
				strcpy(reg_imm, "xmm0");
				strcpy(reg_src, "xmm0");
				
				switch (root->size)
				{
					case 8: strcpy(bitext, "sd"); strcpy(opext, "sd"); break;
					case 4: strcpy(bitext, "ss"); strcpy(opext, "ss"); break;
				}
				
				// Change node string to precise number constant
				FILE *fc;
				switch (root->size)
				{
					case 8: fc = fopen("dconst.txt", "a+"); break;
					case 4: fc = fopen("fconst.txt", "a+"); break;
					default:
						fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
						return 1;
				}
				
				// Assign value to constant
				if (fc == NULL)
				{
					fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
					return 1;
				}
				else
				{
					// Align bytes for SSE2
					fputs("align 16\n", fc);
					
					switch (root->size)
					{
						case 8:
							fprintf(fc, "__D%i dq %s\n", cdata->c_dconstnum, root->string);
							snprintf(root->string, STRING_LENGTH_VARDATA, "[__D%i]", cdata->c_dconstnum);
							cdata->c_dconstnum++;
							break;
						case 4:
							fprintf(fc, "__F%i dd %s\n", cdata->c_fconstnum, root->string);
							snprintf(root->string, STRING_LENGTH_VARDATA, "[__F%i]", cdata->c_fconstnum);
							cdata->c_fconstnum++;
							break;
					}
				}
				
				fclose(fc);
			}
			else if (isint)
			{
				switch (root->size)
				{
					case 8: strcpy(reg_imm, "rax"); break;
					case 4: strcpy(reg_imm, "eax"); break;
				}
			}
			else
			{
				fprintf(stderr, "\nError (Line %u): '%s' is not a valid number, variable, or function.\n\n", cdata->c_linenum, name);
				return 1;
			}
			
			// Setup function call
			if (isexf)
			{
				if (nspex != root->spex) asmbspillx64_s(fout, cdata, reg_imm, bitext);
				if (nspex != NULL)       callprocx64(fout, cdata, nspex->root);
				if (nspex != root->spex) asmbspillx64_l(fout, cdata, reg_imm, bitext);
				
				isvar = 1;
				/*root->type = 'i';
				root->size = 4;*/
			}
			
			// Setup values
			if (isvar)
			{
				if (1) {
				if (hadexp)
				{
					if (root->ext != NULL)
					{
						struct offext *n;   	// Extension node pointer
						unsigned int c = 0; 	// Pointer level counter
						
						// Check if variable is pointer
						for (n = root->ext; (isvf || c < root->ptrlv) && (n->ext != NULL); ++c)
						{
							// Setup offset variable (for optimization)
							unsigned int offset = n->offset;
							
							// First/subsequent runs in loop
							if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_ptr, reg_ptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_ptr, reg_ptr);
							
							// Increment offset extension
							n = n->ext;
						}
						
						// Update pointer level
						root->ptrlv -= c;
						
						// Check if function call or casted
						if (isexf)
						{
							// Setup offest variable (for optimization)
							unsigned int offset = n->offset;
							
							// Check if reference
							if (root->isref)
							{
								if (offset > 0) fprintf(fout, "add %s, %i\ncall %s\n", reg_ptr, offset * MAX_BYTESIZE_X64, reg_ptr);
								else            fprintf(fout, "call %s\n",             reg_ptr);
							}
							else
							{
								if (offset > 0) fprintf(fout, "call qword [%s%+i]\n", reg_ptr, offset * MAX_BYTESIZE_X64);
								else            fprintf(fout, "call qword [%s]\n",    reg_ptr);
							}
						}
						else
						{
							// Setup offset variable (for optimization)
							unsigned int offset = n->offset;
							
							// Check if function
							if (isvf)
							{
								if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_imm, reg_ptr, offset * MAX_BYTESIZE_X64);
								else            fprintf(fout, "mov %s, [%s]\n",    reg_imm, reg_ptr);
							}
							
							// Check if no longer a pointer
							else if (root->ptrlv == 0)
							{
								if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", bitext, reg_imm, sizedef, reg_ptr, (offset * root->size));
								else            fprintf(fout, "mov%s %s, %s[%s]\n",    bitext, reg_imm, sizedef, reg_ptr);
							}
							
							// Check if still a pointer
							else
							{
								strcpy(reg_src, reg_maxint);
								strcpy(reg_imm, reg_maxint);
								
								puts("STILL A PTR");
								
								// This is a stub. Feel free to add anything later.
							}
						}
					}
					else if (isexa)
					{
						
					}
					else if (isexf)
					{
						if (root->isref) fprintf(fout, "call %s\n",         reg_ptr);
						else             fprintf(fout, "call qword [%s]\n", reg_ptr);
					}
					else if (isexc)
					{
						
					}
				}
				else
				{
					if (root->ext != NULL)
					{
						struct offext *n;   	// Extension node pointer
						unsigned int c = 0; 	// Pointer level counter
						
						// Check if variable is pointer
						for (n = root->ext; (isvf || c < root->ptrlv) && (n->ext != NULL); ++c)
						{
							// Setup offset variable (for optimization)
							unsigned int offset = n->offset;
							
							// First run in loop
							if (c == 0)
							{
								// Check if reference
								if (root->isref)
								{
									if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset);
									else
									{
										if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_ptr, name, n->offset * MAX_BYTESIZE_X64);
										else       	    fprintf(fout, "mov %s, [%s]\n",    reg_ptr, name);
									}
									
									// Increment offset extension
									n = n->ext;
								}
								else
								{
									if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
								}
							}
							
							// Subsequent runs in loop
							else
							{
								if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_ptr, reg_ptr, offset * MAX_BYTESIZE_X64);
								else            fprintf(fout, "mov %s, [%s]\n",    reg_ptr, reg_ptr);
								
								// Increment offset extension
								n = n->ext;
							}
						}
						
						// Update pointer level
						root->ptrlv -= c;
						
						// Check if function call or casted
						if (isexf)
						{
							// Setup offest variable (for optimization)
							unsigned int offset = n->offset;
							
							// Check if loop has not run before
							if (c == 0)
							{
								// Check if reference
								if (root->isref)
								{
									if (offset > 0)
									{
										if (isvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset, reg_ptr);
										else      fprintf(fout, "lea %s, [%s%+i]\ncall %s\n",  reg_ptr, name, offset * MAX_BYTESIZE_X64, reg_ptr);
									}
									else
									{
										if (isvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset, reg_ptr);
										else      fprintf(fout, "call %s\n",                   name);
									}
								}
								else
								{
									if (offset > 0)
									{
										if (isvs) fprintf(fout, "call qword [rbp%+i]\n", (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "call qword [%s%+i]\n",  name, offset * MAX_BYTESIZE_X64);
									}
									else
									{
										if (isvs) fprintf(fout, "call qword [rbp%+i]\n", -vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "call qword [%s]\n",     name);
									}
								}
							}
							else
							{
								if (offset > 0) fprintf(fout, "call qword [%s%+i]\n", reg_ptr, offset * MAX_BYTESIZE_X64);
								else            fprintf(fout, "call qword [%s]\n",    reg_ptr);
							}
						}
						else
						{
							// Setup offset variable (for optimization)
							unsigned int offset = n->offset;
							
							// Check if loop has not run before
							if (c == 0)
							{
								// Check if reference
								if (root->isref)
								{
									if (offset > 0)
									{
										if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "mov %s, [%s%+i]\n",  reg_ptr, name, offset * MAX_BYTESIZE_X64);
									}
									else
									{
										if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
									}
								}
								
								// Check if pointer/function
								else if (isvf || (root->ptrlv > 0))
								{
									root->ptrlv--;
									
									if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
									
									if (root->ptrlv > 0)
									{
										strcpy(reg_imm, reg_maxint);
										
										if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_imm, reg_ptr, MAX_BYTESIZE_X64 * root->size);
										else            fprintf(fout, "mov %s, [%s]\n",    reg_imm, reg_ptr);
									}
									else
									{
										if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", bitext, reg_imm, sizedef, reg_ptr, offset * root->size);
										else            fprintf(fout, "mov%s %s, %s[%s]\n",    bitext, reg_imm, sizedef, reg_ptr);
									}
								}
								
								// Check if array
								else if (root->isarr)
								{
									if (offset > 0)
									{
										if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, (offset * root->size) - vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "mov%s %s, %s[%s%+i]\n",  bitext, reg_imm, sizedef, name, (offset * root->size));
									}
									else
									{
										if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, -vdatafind(cdata->l_s, name)->offset);
										else      fprintf(fout, "mov%s %s, %s[%s]\n",     bitext, reg_imm, sizedef, name);
									}
								}
								
								else
								{
									fprintf(stderr, "\nError (Line %u): '%s' is neither an array, nor a pointer.\n\n", cdata->c_linenum, name);
									return 1;
								}
							}
							
							else
							{
								// Check if function
								if (isvf)
								{
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_imm, reg_ptr, offset * MAX_BYTESIZE_X64);
									else            fprintf(fout, "mov %s, [%s]\n",    reg_imm, reg_ptr);
								}
								
								// Check if no longer a pointer
								else if (root->ptrlv == 0)
								{
									if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", bitext, reg_imm, sizedef, reg_ptr, (offset * root->size));
									else            fprintf(fout, "mov%s %s, %s[%s]\n",    bitext, reg_imm, sizedef, reg_ptr);
								}
								
								// Check if still a pointer
								else
								{
									strcpy(reg_src, reg_maxint);
									strcpy(reg_imm, reg_maxint);
									
									puts("STILL A PTR");
									
									// This is a stub. Feel free to add anything later.
								}
							}
						}
					}
					else if (isexa)
					{
						
					}
					else if (isexf)
					{
						// Check if reference
						if (root->isref)
						{
							if (isvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset, reg_ptr);
							else      fprintf(fout, "call %s\n",                   name);
						}
						else
						{
							if (isvs) fprintf(fout, "call qword [rbp%+i]\n", -vdatafind(cdata->l_s, name)->offset);
							else      fprintf(fout, "call qword [%s]\n",     name);
						}
					}
					else if (isexc)
					{
						
					}
					else
					{
						// Check if reference
						if (root->isref)
						{
							// Force source register to hold pointer
							strcpy(reg_src, reg_maxint);
							strcpy(reg_imm, reg_maxint);
							
							if (isvs) fprintf(fout, "lea %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
							else      fprintf(fout, "mov %s, %s\n",       reg_imm, name);
						}
						
						// Check if pointer
						else if ((root->ptrlv > 0) || root->isarr)
						{
							// Force source register to hold pointer
							strcpy(reg_src, reg_maxint);
							strcpy(reg_imm, reg_maxint);
							
							if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
							else      fprintf(fout, "mov %s, [%s]\n",     reg_imm, name);
						}
						
						// Just a variable
						else
						{
							if (isvf)
							{
								if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
								else      fprintf(fout, "mov %s, [%s]\n",     reg_imm, name);
							}
							else
							{
								// Use XOR operation to remove data dependency
								if (isvp && (nspex == NULL)) fprintf(fout, "pxor %s, %s\n", reg_imm, reg_imm);
								
								if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, -vdatafind(cdata->l_s, name)->offset);
								else      fprintf(fout, "mov%s %s, %s[%s]\n",     bitext, reg_imm, sizedef, name);
							}
						}
					}
				}
				}
				
				if (0) {
				if (root->ext != NULL)
				{
					struct offext *n;   	// Extension node pointer
					unsigned int c = 0; 	// Pointer level counter
					
					// Check if variable is pointer
					for (n = root->ext; (isvf || c < root->ptrlv) && (n->ext != NULL); ++c)
					{
						// Setup offset variable (for optimization)
						unsigned int offset = n->offset;
						
						// First run in loop
						if ((c == 0) && !hadexp)
						{
							// Check if reference
							if (root->isref)
							{
								if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset);
								else
								{
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_ptr, name, n->offset * MAX_BYTESIZE_X64);
									else       	    fprintf(fout, "mov %s, [%s]\n",    reg_ptr, name);
								}
								
								// Increment offset extension
								n = n->ext;
							}
							else
							{
								if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
								else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
							}
						}
						
						// Subsequent runs in loop
						else
						{
							if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_ptr, reg_ptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "mov %s, [%s]\n",    reg_ptr, reg_ptr);
							
							// Increment offset extension
							n = n->ext;
						}
					}
					
					// Update pointer level
					root->ptrlv -= c;
					
					// Check if function call or casted
					if (isexf)
					{
						// Setup offest variable (for optimization)
						unsigned int offset = n->offset;
						
						// Check if loop has not run before
						if ((c == 0) && !hadexp)
						{
							// Check if reference
							if (root->isref)
							{
								if (isvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset, reg_ptr);
								else
								{
									if (offset > 0) fprintf(fout, "lea %s, [%s%+i]\ncall %s\n", reg_ptr, name, offset * MAX_BYTESIZE_X64, reg_ptr);
									else            fprintf(fout, "call %s\n",                  name);
								}
							}
							else
							{
								if (isvs) fprintf(fout, "call qword [rbp-%i]\n", vdatafind(cdata->l_s, name)->offset);
								else
								{
									if (offset > 0) fprintf(fout, "call qword [%s%+i]\n", name, offset * MAX_BYTESIZE_X64);
									else            fprintf(fout, "call qword [%s]\n",    name);
								}
							}
						}
						else
						{
							if (offset > 0) fprintf(fout, "call qword [%s%+i]\n", reg_ptr, offset * MAX_BYTESIZE_X64);
							else            fprintf(fout, "call qword [%s]\n",    reg_ptr);
						}
					}
					else
					{
						// Setup offset variable (for optimization)
						unsigned int offset = n->offset;
						
						// Check if loop has not run before
						if ((c == 0) && !hadexp)
						{
							// Check if reference
							if (root->isref)
							{
								if (offset > 0)
								{
									if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, (offset * MAX_BYTESIZE_X64) - vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov %s, [%s%+i]\n",  reg_ptr, name, offset * MAX_BYTESIZE_X64);
								}
								else
								{
									if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
								}
							}
							
							// Check if pointer/function
							else if (isvf || (root->ptrlv > 0))
							{
								root->ptrlv--;
								
								if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_ptr, -vdatafind(cdata->l_s, name)->offset);
								else      fprintf(fout, "mov %s, [%s]\n",     reg_ptr, name);
								
								if (root->ptrlv > 0)
								{
									strcpy(reg_imm, reg_maxint);
									
									if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_imm, reg_ptr, MAX_BYTESIZE_X64 * root->size);
									else            fprintf(fout, "mov %s, [%s]\n",    reg_imm, reg_ptr);
								}
								else
								{
									if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", bitext, reg_imm, sizedef, reg_ptr, offset * root->size);
									else            fprintf(fout, "mov%s %s, %s[%s]\n",    bitext, reg_imm, sizedef, reg_ptr);
								}
							}
							
							// Check if array
							else if (root->isarr)
							{
								if (offset > 0)
								{
									if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, (offset * root->size) - vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov%s %s, %s[%s%+i]\n",  bitext, reg_imm, sizedef, name, (offset * root->size));
								}
								else
								{
									if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, -vdatafind(cdata->l_s, name)->offset);
									else      fprintf(fout, "mov%s %s, %s[%s]\n",     bitext, reg_imm, sizedef, name);
								}
							}
							
							else
							{
								fprintf(stderr, "\nError (Line %u): '%s' is neither an array, nor a pointer.\n\n", cdata->c_linenum, name);
								return 1;
							}
						}
						
						else
						{
							// Check if function
							if (isvf)
							{
								if (offset > 0) fprintf(fout, "mov %s, [%s%+i]\n", reg_imm, reg_ptr, offset * MAX_BYTESIZE_X64);
								else            fprintf(fout, "mov %s, [%s]\n",    reg_imm, reg_ptr);
							}
							
							// Check if no longer a pointer
							else if (root->ptrlv == 0)
							{
								if (offset > 0) fprintf(fout, "mov%s %s, %s[%s%+i]\n", bitext, reg_imm, sizedef, reg_ptr, (offset * root->size));
								else            fprintf(fout, "mov%s %s, %s[%s]\n",    bitext, reg_imm, sizedef, reg_ptr);
							}
							
							// Check if still a pointer
							else
							{
								strcpy(reg_src, reg_maxint);
								strcpy(reg_imm, reg_maxint);
								
								puts("STILL A PTR");
								
								// This is a stub. Feel free to add anything later.
							}
						}
						
					}
				}
				else if (isexa)
				{
					
				}
				else if (isexf)
				{
					if (root->isref)
					{
						if (isvs) fprintf(fout, "lea %s, [rbp%+i]\ncall %s\n", reg_imm, -vdatafind(cdata->l_s, name)->offset, reg_imm);
						else      fprintf(fout, "call %s\n",                   name);
					}
					else
					{
						if (isvs) fprintf(fout, "call qword [rbp%+i]\n", -vdatafind(cdata->l_s, name)->offset);
						else      fprintf(fout, "call qword [%s]\n",     name);
					}
				}
				else if (isexc)
				{
					
				}
				else
				{
					if (root->isref)
					{
						// Force source register to hold pointer
						strcpy(reg_src, reg_maxint);
						strcpy(reg_imm, reg_maxint);
						
						if (isvs) fprintf(fout, "lea %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
						else      fprintf(fout, "mov %s, %s\n",       reg_imm, name);
					}
					else if ((root->ptrlv > 0) || root->isarr)
					{
						// Force source register to hold pointer
						strcpy(reg_src, reg_maxint);
						strcpy(reg_imm, reg_maxint);
						
						// Check if first/no special extension
						if (nspex == root->spex)
						{
							if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
							else      fprintf(fout, "mov %s, [%s]\n",     reg_imm, name);
						}
					}
					else
					{
						// Check if function
						if (isvf)
						{
							if (nspex == root->spex)
							{
								if (isvs) fprintf(fout, "mov %s, [rbp%+i]\n", reg_imm, -vdatafind(cdata->l_s, name)->offset);
								else      fprintf(fout, "mov %s, [%s]\n",     reg_imm, name);
							}
						}
						else
						{
							// Use XOR operation to remove data dependency
							if (isvp && (nspex == NULL)) fprintf(fout, "pxor %s, %s\n", reg_imm, reg_imm);
							
							if (isvs) fprintf(fout, "mov%s %s, %s[rbp%+i]\n", bitext, reg_imm, sizedef, -vdatafind(cdata->l_s, name)->offset);
							else      fprintf(fout, "mov%s %s, %s[%s]\n",     bitext, reg_imm, sizedef, name);
						}
					}
				}
				}
				//*/
			}
			else if (isint || isprc)
			{
				// Use XOR operation to remove data dependency
				if (isprc) fprintf(fout, "pxor %s, %s\n", reg_imm, reg_imm);
				fprintf(fout, "mov%s %s, %s\n", bitext, reg_imm, root->string);
			}
			
			// Reset function flag (REMOVE SOON)
			isvf = 0;
			
			// Setup aftermath from special extensions
			if (nspex != NULL)
			{
				// Function call
				// If no longer function, set "name" to "reg_ptr"
				
				// Variable cast
				if (isexc)
				{
					// Set variable type
					switch (nspex->string[1])
					{
						case 'i': root->type = 'i'; break;
						case 'u': root->type = 'u'; break;
						case 'p': root->type = 'p'; break;
					}
					
					// Set variable size
					switch (nspex->string[2])
					{
						case '1': root->size = 1; break;
						case '2': root->size = 2; break;
						case '4': root->size = 4; break;
						case '8': root->size = 8; break;
					}
				}
				
				// Invalid special extension
				if (!isexa && !isexf && !isexc)
				{
					fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'asmprocx64'.\n\n", stderr);
					return 1;
				}
				
				// Setup next set of offset extensions
				if ((nspex == root->spex) && (root->ext != NULL)) extdest(root->ext);
				
				// Inherit from special extension to node
				root->isref = nspex->isref;
				root->ext = nspex->ext;
				
				// Move on to next special extension
				if (nspex->spex != NULL)
				{
					// Check if function
					isvf = nodeisfun(nspex);
				}
				else if (root->ext != NULL)
				{
					// Move to proper pointer
					fputs("mov rdx, rax\n", fout);
				}
				
				// Setup next run in loop
				nspex = nspex->spex;
			}
			
			// Free the final set of special extensions
			else root->ext = NULL;
		}
		while ((root->ext != NULL) || (nspex != NULL));
		
		return 0;
	}
	
	// Do nothing
	else return 0;
}

// Create arm 64-bit assembly code from process tree
// This is a stub, since I have no experience with arm64 assembly,
// but it could possibly be worked on with some contributors.
int asmprocarm64(FILE *fout, struct compiledata *cdata, struct node *troot, struct node *root)
{
	return 0;
}

// Create function call assembly code
int callprocx64(FILE *fout, struct compiledata *cdata, struct node *root)
{
	char argsl[STRING_LENGTH_VARDATA];	// Argument execution
	unsigned int
	argc = 0,   	// Argument count
	argci = 0,  	// Argument integer count
	argcp = 0,  	// Argument precise count
	argcs = 0;  	// Argument stack count
	
	// Determine number of arguments
	while (strctok(argsl, root->spex->string, NULL, ",", argc))
		argc++;
	
	// Decrement stack pointer
	switch (cdata->f_abi)
	{
		// Linux/System-V
		case OS_LINUX:
			if (argci > 5)
				fprintf(fout, "sub rsp, %i\n", (argc - 3) * MAX_BYTESIZE_X64);
			break;
		
		// Windows
		case OS_WIN32:
			fprintf(fout, "sub rsp, %i\n", (4 + argc - 1) * MAX_BYTESIZE_X64);
			break;
	}
	
	// Assign argument values
	for (unsigned int i = 0; i < argc; ++i)
	{
		strctok(argsl, root->spex->string, NULL, "(,)", i);
		strip(argsl);
		
		// Stop processing arguments if empty argument
		if (strlen(argsl) == 0)
		{
			// Check if user-made error
			if (i > 0) fprintf(stderr, "\nWarning (Line %u): Empty argument #%u in function call '%s'. Argument value is set to zero.\n\n", cdata->c_linenum, i + 1, root->string);
			else break;
			
			// Set argument value to 0
			fputs("; 0\nxor eax, eax\n", fout);
		}
		else if (nodeproc(fout, cdata, NULL, argsl, 1) == NULL)
		{
			fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'callproc'.\n\n", stderr);
			return 1;
		}
		
		// Output arguments to function
		switch (cdata->f_abi)
		{
			// Linux/System-V
			case OS_LINUX:
				switch (i)
				{
					case 0: fputs("mov rdi, rax\n", fout); break;
					case 1: fputs("mov rsi, rax\n", fout); break;
					case 2: fputs((argc == i + 1) ? "mov rdx, rax\n" : "push rax\n", fout); break;
					case 3: fputs((argc == i + 1) ? "mov rcx, rax\n" : "push rax\n", fout); break;
					case 4: fputs("mov r8, rax\n", fout); break;
					case 5: fputs("mov r9, rax\n", fout); break;
					default:
						fprintf(fout, "mov [rsp+%i], rax\n", (i - 3) * MAX_BYTESIZE_X64);
						break;
				}
				break;
			
			// Windows
			case OS_WIN32:
				switch (i)
				{
					case 0: fputs((argc == i + 1) ? "mov rcx, rax\n" : "push rax\n", fout); break;
					case 1: fputs((argc == i + 1) ? "mov rdx, rax\n" : "push rax\n", fout); break;
					case 2: fputs("mov r8, rax\n", fout); break;
					case 3: fputs("mov r9, rax\n", fout); break;
					default:
						fprintf(fout, "mov [rsp+%i], rax\n", (i - 1) * MAX_BYTESIZE_X64);
						break;
				}
				break;
		}
	}
	
	// Restore rcx and rdx register values
	switch (cdata->f_abi)
	{
		// Linux/System-V
		case OS_LINUX:
			if (argc > 4) fputs("pop rcx\n", fout);
			if (argc > 3) fputs("pop rdx\n", fout);
			break;
		
		// Windows
		case OS_WIN32:
			if (argc > 2) fputs("pop rdx\n", fout);
			if (argc > 1) fputs("pop rcx\n", fout);
			break;
	}
	
	return 0;
}

// Create process tree for assembly code
struct node* nodeproc(FILE *fout, struct compiledata *cdata, struct node *dest, char *args, int autofree)
{
	char
	token[3][STRING_LENGTH_VARDATA] = {""},	// Tokens for function/variable names, and operator
	specl[2][STRING_LENGTH_VARDATA] = {""};	// Function arguments (if any)
	
	struct node 
	*nbase[2]  = {NULL},
	*nfocus[2] = {NULL};
	
	unsigned int c_tok = 0;  	// Token counter
	
	// Nodes
	struct node
	*root  = NULL,  	// Node on top of everything else
	*focus = NULL,  	// Last node worked on in loop
	*droot = NULL;  	// Root of destination node
	
	// Check if null args
	if (args == NULL) return (struct node *) 1;
	
	// Show equation in assembly
	if (dest == NULL) fprintf(fout, "; %s\n", stritok(args, NULL, " \t", 0));
	
	if (strlen(args))
	for (unsigned int i = 0; strcltok(token[c_tok], args, "~:.", "[](){}''\"\"", " \t", i); ++i, ++c_tok)
	{
		switch (c_tok)
		{
			// Increment token counter
			case 0:
				// Cleanup special extension string
				strcpy(specl[0], "");
				
				// Check for special extensions
				while (strcltok(specl[0], args, "~:.", "[](){}''\"\"", " \t", i + 1))
				{
					struct node *n;
					switch (specl[0][0])
					{
						case '[':	// Advanced array index
						case '(':	// Function call
						case '{':	// Variable cast
							// Create special extension
							n = nodeinit(specl[0]);
							
							// Assign special extension
							if (nbase[0] == NULL)
							{
								nbase[0] = n;
								nfocus[0] = n;
							}
							else
							{
								n->root = nfocus[0];
								nfocus[0]->spex = n;
								nfocus[0]->spex->root = n->root;
								nfocus[0] = n;
							}
							
							// Increment index of token
							i++;
							break;
						default:
							goto lb_aspec0;
							break;
					}
				}
				lb_aspec0:
				break;
			
			case 1:
				break;
			
			// Process nodes
			case 2:
				// Cleanup special extension string
				strcpy(specl[1], "");
				
				// Check for special extensions
				while (strcltok(specl[1], args, "~:.", "[](){}''\"\"", " \t", i + 1))
				{
					struct node *n;
					switch (specl[1][0])
					{
						case '[':	// Advanced array index
						case '(':	// Function call
						case '{':	// Variable cast
							// Create special extension
							n = nodeinit(specl[1]);
							
							// Assign special extension
							if (nbase[1] == NULL)
							{
								nbase[1] = n;
								nfocus[1] = n;
							}
							else
							{
								n->root = nfocus[1];
								nfocus[1]->spex = n;
								nfocus[1]->spex->root = n->root;
								nfocus[1] = n;
							}
							
							// Increment index of token
							i++;
							break;
						default:
							goto lb_aspec1;
							break;
					}
				}
				lb_aspec1:
				
				// Setup process tree
				if (focus == NULL)
				{
					// Setup first 3 nodes
					struct node
					*n0 = nodeinit(token[0]),
					*n1 = nodeinit(token[1]),
					*n2 = nodeinit(token[2]);
					
					// Check if operator is valid
					if (nodelex(n1) == 0)
					{
						fprintf(stderr, "\nError (Line %u): Invalid operator '%s'.\n\n", cdata->c_linenum, token[1]);
						return NULL;
					}
					
					// Setup important nodes
					root = n1;
					focus = n1;
					
					// Setup node structure
					n1->west = n0;
					n1->east = n2;
					
					n0->root = n1;
					n2->root = n1;
					
					// Connect to destination node (if it exists)
					if (dest != NULL)
					{
						droot = dest->root;
						root->ext = dest->ext;
						dest->ext = NULL;
						
						//printf("OFFSET: %i %p\n", (dest->ext != NULL) ? dest->ext->offset : 0, (dest->ext != NULL) ? dest->ext->ext : NULL);
						
						if (droot != NULL)
						{
							root->root = droot;
							if (droot->west == dest)
							{
								nodedest(dest);
								droot->west = root;
							}
							if (droot->east == dest)
							{
								nodedest(dest);
								droot->east = root;
							}
						}
					}
				}
				
				// Compare nodes
				else
				{
					// Setup nodes
					struct node
					*n1 = nodeinit(token[1]),
					*n2 = nodeinit(token[2]),
					*comp = focus;
					
					// Check if operator is valid
					if (nodelex(n1) == 0)
					{
						fprintf(stderr, "\nError (Line %u): Invalid operator '%s'.\n\n", cdata->c_linenum, token[1]);
						return NULL;
					}
					
					// Compare focus node operator importance with previous nodes' operators
					// Get closer to the process tree root
					while ((comp != droot) && ((nodelex(n1) / 8) <= (nodelex(comp) / 8)))
					{
						comp = comp->root;
					}
					
					// Setup basic connection between n1 and n2
					n1->root = comp;
					n1->east = n2;
					n2->root = n1;
					
					// Integrate nodes n1 and n2 to process tree
					if (comp != droot)
					{
						n1->west = comp->east;
						comp->east->root = n1;
						comp->east = n1;
					}
					// Set node n1 as root node
					else if (droot != NULL)
					{
						n1->root = droot;
						n1->west = root;
						
						if (droot->west == root) droot->west = n1;
						if (droot->east == root) droot->east = n1;
						
						// Transfer root extension node
						if (root->ext != NULL)
						{
							n1->ext = root->ext;
							root->ext = NULL;
						}
						
						// Transfer root special extension node
						if (root->spex != NULL)
						{
							n1->spex = root->spex;
							n1->spex->root = n1;
							root->spex = NULL;
						}
						
						root->root = n1;
						root = n1;
					}
					// Set node n1 as root node
					else
					{
						n1->west = root;
						
						// Transfer root extension node
						if (root->ext != NULL)
						{
							n1->ext = root->ext;
							root->ext = NULL;
						}
						
						// Transfer root special extension node
						if (root->spex != NULL)
						{
							n1->spex = root->spex;
							n1->spex->root = n1;
							root->spex = NULL;
						}
						
						root->root = n1;
						root = n1;
					}
					
					// Set new focus
					focus = n1;
				}
				
				// Assign argument lists to function-call nodes
				if (nbase[0] != NULL) 
				{
					focus->west->spex = nbase[0];
					focus->west->spex->root = focus->west;
					nbase[0] = NULL;
					nfocus[0] = NULL;
				}
				
				if (nbase[1] != NULL)
				{
					focus->east->spex = nbase[1];
					focus->east->spex->root = focus->east;
					nbase[1] = NULL;
					nfocus[1] = NULL;
				}
				
				//fprintf(stderr, "NODE: '%s%s %s %s%s'\n", focus->west->string, specl[0], focus->string, focus->east->string, specl[1]);
				//fprintf(stderr, "NODE: '%s %s %s'\n", focus->west->string, focus->string, focus->east->string);
				
				// Reset variables
				c_tok = 0;
				break;
			default:
				fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'nodeproc'.\n\n", stderr);
				return NULL;
		}
	}
	
	// Create assembly code
	if (root != NULL)
	{
		//fprintf(stderr, "ROOT1: '%s %s %s'\n", wname, root->string, ename);
		
		// Setup optimizations, offset extensions
		if (asmpreproc(fout, cdata, root))
		{
			fputs("Node preprocessing has failed.\n", stderr);
			return NULL;
		}
		
		// Write assembly code
		if (droot == NULL)
		{
			if (
			#ifdef __x86_64__
			asmprocx64(fout, cdata, root, root)
			#elif defined __arm__
			asmprocarm64(fout, cdata, root, root)
			#endif
			)
			{
				fputs("Assembly generation has failed.\n", stderr);
				return NULL;
			}
		}
	}
	else
	{
		char string[STRING_LENGTH_VARDATA];
		strcltok(string, args, "~:.", "[]()''\"\"", " \t", 0);
		
		root = nodeinit(string);
		
		// Assign argument lists to function-call node
		if (nbase[0] != NULL)
		{
			root->spex = nbase[0];
			root->spex->root = root;
		}
		
		// Check if parenthesized
		if (string[0] == '(')
		{
			if (asmpreproc(fout, cdata, root))
			{
				fputs("\nError: Node preprocessing has failed.\n\n", stderr);
				return NULL;
			}
		}
		// Just set the value to it
		else
		{
			if (asmpreproc(fout, cdata, root))
			{
				fputs("\nError: Node preprocessing has failed.\n\n", stderr);
				return NULL;
			}
			
			if (
			#ifdef __x86_64__
			asmprocx64(fout, cdata, root, root)
			#elif defined __arm__
			asmprocarm64(fout, root, root, cdata)
			#endif
			)
			{
				fputs("Assembly generation has failed.\n", stderr);
				return NULL;
			}
		}
	}
	
	// Destroy process tree
	if (dest == NULL) fputc('\n', fout);
	if (autofree && (root != NULL)) nodedest(root); 
	
	return root;
}

// Change between main code file and function file for file pointer
int fmodproc(struct compiledata *cdata, FILE *fout, int infunc)
{
	FILE *ftmp;     	// File pointer for function code file
	char line[STRING_LENGTH_LINE]; 	
	
	if (infunc == 0)
	{
		// Remove stack variables from lists
		for (struct vdata *v = cdata->l_s->start; v != NULL; v = v->next)
		{
			vdatadest(cdata->l_i, v->string);
			vdatadest(cdata->l_u, v->string);
			vdatadest(cdata->l_p, v->string);
		}
		
		vdatadestall(cdata->l_s);
	}
	
	// Check if 'inside-function' status changed
	if (infunc != cdata->f_infunc)
	{
		switch (infunc)
		{
			// Change file pointer to output to function file
			case 1:
				fout = freopen("codef.txt", "a+", fout);
				break;
				
			// Change file pointer to main code file, also append function text to main code
			case 0:
				// Setup file pointers
				fout = freopen("codex.txt", "a+", fout);
				ftmp =   fopen("codef.txt", "r+");
				
				// Write stack decrement code
				if (cdata->c_stacklv > 0)
				{
					#ifdef __x86_64__
					fprintf(fout, "push rbp\nmov rbp, rsp\nsub rsp, %i\n", cdata->c_stacklv + cdata->c_bspilllv);
					#endif
					#ifdef __arm__
					fprintf(fout, "sub sp, sp, #%i\n", cdata->c_stacklv);
					#endif
					
					// Reset stack-related data
					cdata->c_stacklv = 0;
				}
				
				while (fgets(line, STRING_LENGTH_LINE, ftmp))
				{
					// Process line
					strip(line);
					
					// Check if line is not empty
					if (strlen(line)) 
						fprintf(fout, "%s\n", line);
					else fputc('\n', fout);
				}
				
				fclose(ftmp);
				remove("codef.txt");
				
				break;
			default:
				fputs("\nError: You may have broken something in the source code.\nDebug 'main.c', in function 'fmodproc'.\n\n", stderr);
				return 1;
		}
		
		// Update 'inside-function' status
		cdata->f_infunc = infunc;
	}
	return 0;
}

// Check for Conditionals
int condproc(struct compiledata *cdata, FILE *fout, char *args, char argl[STRING_LENGTH_LINE/32][STRING_LENGTH_LINE/8])
{
	// If, Else-If, Else, Fi (End-If)
	if (!strcmp(argl[0], "if"))
	{
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		fprintf(fout, "cmp eax, 0\nje .c%i_%i\n\n", cdata->c_condmaj[cdata->c_condlv], cdata->c_condmin[cdata->c_condlv]);
		
		// Check for limit of nesting statements
		if (cdata->c_condlv < MAX_NESTLEVEL)
		{
			// Increment condition level
			cdata->c_condlv++;
			cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv - 1] + 1;
		}
		else
		{
			fprintf(stderr, "\nError (Line %u): Exceeded conditional nesting limit (maximum is %u).\n\n", cdata->c_linenum, MAX_NESTLEVEL);
			return 1;
		}
	}
	if (!strcmp(argl[0], "lf"))
	{
		// Write jump label (related to previous comparison code)
		fprintf(fout, "\njmp .c%ie\n.c%i_%i:\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
		
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		// Write condition
		cdata->c_condmin[cdata->c_condlv - 1]++;
		fprintf(fout, "cmp eax, 0\nje .c%i_%i\n\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
	}
	if (!strcmp(argl[0], "ls"))
	{
		// Write jump label (related to previous comparison code)
		fprintf(fout, "\njmp .c%ie\n.c%i_%i:\n\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
		
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		// Set 'else' flag on compiler data
		cdata->f_condisls[cdata->c_condlv - 1] = 1;
	}
	if (!strcmp(argl[0], "fi"))
	{
		// Check if previous condition of same scope is 'ls' (else)
		if (cdata->f_condisls[cdata->c_condlv - 1])
		{
			// Write jump label (end of condition)
			fprintf(fout, "\n.c%ie:\n\n", cdata->c_condmaj[cdata->c_condlv - 1]);
			cdata->f_condisls[cdata->c_condlv - 1] = 0;
		}
		else
		{
			fprintf(fout, ".c%i_%i:\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
			fprintf(fout, ".c%ie:\n\n", cdata->c_condmaj[cdata->c_condlv - 1]);
		}
		
		cdata->c_condlv--;                                                        	// Decrement condition level
		cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv + 1];	// Increment condition counter
		cdata->c_condmin[cdata->c_condlv] = 0;                                    	// Reset minor condition counter
	}
	
	// Compare, Case, No-Case, End, End-Compare
	if (!strcmp(argl[0], "cmp"))
	{
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		// Check for limit of nesting statements
		if (cdata->c_condlv < MAX_NESTLEVEL)
		{
			// Increment condition level
			cdata->c_condlv++;
			cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv - 1] + 1;
		}
		else
		{
			fprintf(stderr, "\nError (Line %u): Exceeded conditional nesting limit (maximum is %u).\n\n", cdata->c_linenum, MAX_NESTLEVEL);
			return 1;
		}
	}
	if (!strcmp(argl[0], "case"))
	{
		// Write jump label (related to previous comparison code)
		fprintf(fout, ".c%i_%i:\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
		
		// Write condition
		cdata->c_condmin[cdata->c_condlv - 1]++;
		fprintf(fout, "cmp rax, %s\njne .c%i_%i\n", argl[1], cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
	}
	if (!strcmp(argl[0], "nocase"))
	{
		// Write jump label (related to previous comparison code)
		fprintf(fout, ".c%i_%i:\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1]);
		
		// Set 'else' flag on compiler data
		cdata->f_condisls[cdata->c_condlv - 1] = 1;
	}
	if (!strcmp(argl[0], "end"))
	{
		// Check if previous condition of same scope is 'nocase'
		if (!cdata->f_condisls[cdata->c_condlv - 1])
			// Write jump code
			fprintf(fout, "\njmp .c%ie\n", cdata->c_condmaj[cdata->c_condlv - 1]);
	}
	if (!strcmp(argl[0], "ecmp"))
	{
		// Check if previous condition of same scope is 'nocase'
		if (cdata->f_condisls[cdata->c_condlv - 1])
		{
			// Write jump label (end of condition)
			fprintf(fout, "\n.c%ie:\n\n", cdata->c_condmaj[cdata->c_condlv - 1]);
			cdata->f_condisls[cdata->c_condlv - 1] = 0;
		}
		else fprintf(fout, "\n.c%i_%i:\n.c%ie:\n\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmin[cdata->c_condlv - 1], cdata->c_condmaj[cdata->c_condlv - 1]);
		
		cdata->c_condlv--;                                                        	// Decrement condition level
		cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv + 1];	// Increment condition counter
		cdata->c_condmin[cdata->c_condlv] = 0;                                    	// Reset minor condition counter
	}
	
	// Do-While
	if (!strcmp(argl[0], "do"))
	{
		// Write jump label (related to previous comparison code)
		fprintf(fout, ".c%i_l:\n", cdata->c_condmaj[cdata->c_condlv]);
		
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		// Check for limit of nesting statements
		if (cdata->c_condlv < MAX_NESTLEVEL)
		{
			// Increment condition level
			cdata->c_condlv++;
			cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv - 1] + 1;
		}
		else
		{
			fprintf(stderr, "\nError (Line %u): Exceeded conditional nesting limit (maximum is %u).\n\n", cdata->c_linenum, MAX_NESTLEVEL);
			return 1;
		}
	}
	if (!strcmp(argl[0], "wl"))
	{
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		// Write condition
		fprintf(fout, "cmp eax, 1\njge .c%i_l\n\n", cdata->c_condmaj[cdata->c_condlv - 1]);
		
		cdata->c_condlv--;                                                        	// Decrement condition level
		cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv + 1];	// Increment condition counter
		cdata->c_condmin[cdata->c_condlv] = 0;                                    	// Reset minor condition counter
	}
	
	// If-While
	if (!strcmp(argl[0], "ifwl"))
	{
		// Write jump label (related to start of loop)
		fprintf(fout, "\n.c%i_l:\n", cdata->c_condmaj[cdata->c_condlv]);
		
		// Execution code
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 1), 1) == NULL)
			return 1;
		
		fprintf(fout, "cmp eax, 0\nje .c%ie\n\n", cdata->c_condmaj[cdata->c_condlv]);
		
		// Check for limit of nesting statements
		if (cdata->c_condlv < MAX_NESTLEVEL)
		{
			// Increment condition level
			cdata->c_condlv++;
			cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv - 1] + 1;
		}
		else
		{
			fprintf(stderr, "\nError (Line %u): Exceeded conditional nesting limit (maximum is %u).\n\n", cdata->c_linenum, MAX_NESTLEVEL);
			return 1;
		}
	}
	if (!strcmp(argl[0], "lwfi"))
	{
		// Write condition
		fprintf(fout, "\njmp .c%i_l\n.c%ie:\n\n", cdata->c_condmaj[cdata->c_condlv - 1], cdata->c_condmaj[cdata->c_condlv - 1]);
		
		cdata->c_condlv--;                                                        	// Decrement condition level
		cdata->c_condmaj[cdata->c_condlv] = cdata->c_condmaj[cdata->c_condlv + 1];	// Increment condition counter
		cdata->c_condmin[cdata->c_condlv] = 0;                                    	// Reset minor condition counter
	}
	
	// Function
	if (cdata->f_infunc == 0) 
	{
		if (!strcmp(argl[0], "fn"))
		{
			fprintf(fout, "%s:\n", argl[1]);
			
			// Assign as function in list
			vdatainit(cdata->l_f, argl[1], 0, 0, 0, 0, 0);
			
			// Change file code pointer
			fmodproc(cdata, fout, 1);
		}
		if (!strcmp(argl[0], "nt"))
		{
			fprintf(fout, "entry %s\n%s:\n", argl[1], argl[1]);
			
			// Assign as function in list
			vdatainit(cdata->l_f, argl[1], 0, 0, 0, 0, 0);
			
			// Change file code pointer
			fmodproc(cdata, fout, 1);
		}
	}
	if (cdata->f_infunc == 1)
	{
		if (!strcmp(argl[0], "ret"))
		{
			// Execution code
			if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t,", 1), 1) == NULL)
				return 1;
			
			else if (cdata->c_condlv > 0)
			{
				// Insert jump to end of function
				fputs("jmp .__endfn\n", fout);
			}
			else
			{
				// Insert label to end of function
				fputs(".__endfn:\n", fout);
				
				// Write stack increment code
				if (cdata->c_stacklv > 0)
				{
					// x86-64 assembly code
					#ifdef __x86_64__
					fprintf(fout, "add rsp, %i\npop rbp\n", cdata->c_stacklv + cdata->c_bspilllv);
					#endif
					
					// ARM64 assembly code
					// This is a stub, since I have no idea about arm64 assembly, but
					// it could possibly be worked on with some other contributors.
					#ifdef __arm__
					fprintf(fout, "add sp, sp, #%i\n", cdata->c_stacklv);
					#endif
					
					fprintf(fout, "ret\n");
				}
				
				// Change file code pointer
				fmodproc(cdata, fout, 0);
				
				// Reset all condition counters
				cdata->c_condmaj[0] = 0;
				cdata->c_condmin[0] = 0;
				cdata->c_condlv = 0;
			}
		}
		if (!strcmp(argl[0], "exit"))
		{
			// Execution code
			if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t,", 1), 1) == NULL)
				return 1;
			
			else if (cdata->c_condlv > 0)
			{
				// Insert jump to end of function
				fputs("jmp .__endfn\n", fout);
			}
			else
			{
				// Insert label to end of function
				fputs(".__endfn:\n", fout);
				
				// Write stack increment code
				if (cdata->c_stacklv > 0)
				{
					// x86-64 assembly code
					#ifdef __x86_64__
						
						if (cdata->c_stacklv > 0) fprintf(fout, "add rsp, %i\npop rbp\n", cdata->c_stacklv + cdata->c_bspilllv);
						
						if (stritok(args, "''\"\"", " \t,", 1)[0] != '\0')
						switch (cdata->f_abi)
						{
							case OS_LINUX: fputs("mov rdi, rax\nmov rax, 60\nsyscall\n\n", fout);        break;
							case OS_WIN32: fputs("mov rcx, rax\ncall [ExitProcess]\n\n", fout);          break;
							case OS_MACOS: fputs("mov rdi, rax\nmov rax, 0x2000001\nsyscall\n\n", fout); break;
							default:
								fputs("\nError: You may have broken something in the compiler's source code.\nDebug 'main.c', in function 'condproc'.\n\n", stderr);
								return 1;
						}
						
					#endif
					
					// ARM64 assembly code
					// This is a stub, since I have no idea about arm64 assembly, but
					// it could possibly be worked on with some other contributors.
					#ifdef __arm__
					fprintf(fout, "add sp, sp, #%i\n\n", cdata->c_stacklv);
					#endif
				}
				
				// Change file code pointer
				fmodproc(cdata, fout, 0);
				
				// Reset all condition counters
				cdata->c_condmaj[0] = 0;
				cdata->c_condmin[0] = 0;
				cdata->c_condlv = 0;
			}
		}
	}
	
	// Setup variable name/number
	char name[STRING_LENGTH_VARNAME];
	strctok(name, argl[0], NULL, "@~:.[](){}", 0);
	
	// Expression
	if (isinteger(name) || isprecise(name)
	|| (vdatafind(cdata->l_i, name) != NULL)
	|| (vdatafind(cdata->l_u, name) != NULL)
	|| (vdatafind(cdata->l_p, name) != NULL)
	|| (vdatafind(cdata->l_f, name) != NULL))
	{
		// Check if inside function
		if (cdata->f_infunc == 0)
		{
			fprintf(stderr, "\nError (Line %u): Expression is outside a function.\n\n", cdata->c_linenum);
			return 1;
		}
		
		if (nodeproc(fout, cdata, NULL, stritok(args, "''\"\"", " \t", 0), 1) == NULL)
			return 1;
	}
	if (!strcmp(argl[0], "lb"))
	{
		// Create location label
		fprintf(fout, "%s:\n", argl[1]);
	}
	if (!strcmp(argl[0], "gt"))
	{
		// Jump to location
		fprintf(fout, "jmp %s\n", argl[1]);
	}
	if (!strcmp(argl[0], "asm"))
	{
		fprintf(fout, "%s\n", stritok(args, "''\"\"", " \t,", 1));
	}
	
	return 0;
}

// Parse data in variable
int vdataproc(FILE *fout, struct vlist *l_type, char *args, char *name, int type, int cvsize, int dupsz)
{
	int arrsz = 0;
	char 
	line[STRING_LENGTH_LINE]     = "",
	token[STRING_LENGTH_VARDATA] = "";
	
	// Loop to print data stuff to file
	for (unsigned int i = 0; strcltok(token, args, NULL, "''\"\"", ",", i); ++i)
	{
		if (i > 0) strcat(line, ",");
		
		strip(token);
		
		// Variables for string loop, because switch-case statement 
		// won't allow defining variables within its cases
		
		unsigned int
		ix, 	// Index for string
		esv,	// Value of escape character
		len;	// String length
		
		char base_num[4] = "";   	// Reserved base x string
		
		switch (token[0])
		{
			// String
			case '"':
				// Output characters in loop
				for (ix = 0, len = strlen(token); ix < len; ++ix)
				{
					// Reset variables
					strcpy(base_num, "");
					esv = 0;
					
					// Check for escape characters
					if ((ix > 0) && (token[ix - 1] == '\\'))
					{
						// Determine value of escape chars
						switch (token[ix])
						{
							case 'a': esv = 7;  break;
							case 'b': esv = 8;  break;
							case 't': esv = 9;  break;
							case 'n': esv = 10; break;
							case 'r': esv = 13; break;
							case 'x':
								strcpy(base_num, "0x");
								strncat(base_num + 2, token + ix + 1, 2);
								ix += 2;
								break;
							default:
								sprintf(line + strlen(line), "\",'%c',\"", token[ix]);
								break;
						}
						
						// Check for variables' status
						if (strlen(base_num) > 0) sprintf(line + strlen(line), "\",%llu,\"", (unsigned long long int) stoi(base_num)); else
						if (esv > 0)              sprintf(line + strlen(line), "\",%u,\"", esv);
					}
					else if (token[ix] != '\\')
						sprintf(line + strlen(line), "%c", token[ix]);
				}
				
				// Update array size
				arrsz += ix - 1;
				break;
				
			// Character
			case '\'':
				sprintf(line + strlen(line), "%llu", (unsigned long long int) stoi(token));
				arrsz++;
				break;
			
			// Variable/number
			default:
				strcat(line, token);
				arrsz++;
				break;
		}
	}
	
	// Set flag for array in vdata
	if (arrsz > 1)
	{
		vdatafind(l_type, name)->isarr = 1;
	}
	
	if (dupsz > 1) fprintf(fout, "%-16s d%c %-4i dup (%s)\n", name, cvsize, dupsz, line);
	else           fprintf(fout, "%-16s d%c %s\n", name, cvsize, line);
	
	putc('\n', fout);
	
	return 0;
}

// Define variable
int vdefproc(struct compiledata *cdata, FILE *fdata, FILE *fcode, char *args, char argl[STRING_LENGTH_LINE/32][STRING_LENGTH_LINE/8])
{
	int 
	type = 0, 
	size = 0;
	
	// Check if token is eligible for reading
	if (strlen(argl[0]) >= 2)
	{
		// Get type of variable
		switch (argl[0][0])
		{
			case 'i':
			case 'u':
			case 'p':
				type = (int) argl[0][0];
				break;
		}
		
		// Get size of variable
		switch (argl[0][1])
		{
			case '1': size = 1; break;
			case '2': size = 2; break;
			case '4': size = 4; break;
			case '8': size = 8; break;
		}
		
		// Check if valid declaration
		if ((type > 0) && (size > 0))
		{
			unsigned int 
			stage = 0,		// End of stage
			arrsz = 1,		// Array size
			ptrlv = 0;		// Pointer level
			
			// Get pointer level within variable declaration
			if (strlen(argl[0]) >= 4)
			{
				if (argl[0][2] != '~')
				{
					fprintf(stderr, "\nError (Line %u): Invalid declaration of pointer.\n\n", cdata->c_linenum);
					return 1;
				}
				else
				{
					char s_ptrlv[STRING_LENGTH_NUMBER];
					strctok(s_ptrlv, argl[0], NULL, "~", ~1);
					
					// Check if integer
					if (isinteger(s_ptrlv)) ptrlv = stoi(s_ptrlv);
					else
					{
						fprintf(stderr, "\nError (Line %u): Invalid declaration of pointer.\n\n", cdata->c_linenum);
						return 1;
					}
				}
			}
			
			// Token string
			char token[STRING_LENGTH_VARNAME];
			
			// Lists for type and size
			struct vlist *l_type = NULL;
			
			for (unsigned int i = 1; strctok(token, args, "''\"\"", " \t", i); ++i)
			{
				char cvsize;        	// Character for assembly code
				unsigned int vsize; 	// Variable size
				
				// Array stage
				if (stage == 0)
				{
					// Update array size
					if (isinteger(token)) arrsz *= atoi(token);
					
					// End of stage
					else stage++;
				}
				
				// Variable assignment stage
				if (stage == 1)
				{
					// Setup based on vartype
					switch (type)
					{
						case 'i': l_type = cdata->l_i; break;
						case 'u': l_type = cdata->l_u; break;
						case 'p': l_type = cdata->l_p; break;
					}
					
					// Setup based on varsize
					// Check if pointer
					if (ptrlv > 0)
					{
						vsize = 8;
						cvsize = 'q';
					}
					// Check if regular variable
					else switch (size)
					{
						case 1: vsize = 1; cvsize = 'b'; break;
						case 2: vsize = 2; cvsize = 'w'; break;
						case 4: vsize = 4; cvsize = 'd'; break;
						case 8: vsize = 8; cvsize = 'q'; break;
					}
					
					// Assign as variables in lists of appropriate type and size
					switch (token[0])
					{
						// Prevent underscore and dot variable name declaration
						case '_':
							fprintf(stderr, "\nError (Line %u): Invalid variable name '%s'. Declaring variables with names beginning with an underscore '_' is reserved to the compiler.\n\n", cdata->c_linenum, token);
							return 1;
						case '.':
							fprintf(stderr, "\nError (Line %u): Invalid variable name '%s'. Declaring variables with names beginning with a dot '.' is reserved to the compiler.\n\n", cdata->c_linenum, token);
							return 1;
						default:
							vdatainit(l_type, token, ptrlv, 0, type, size, arrsz);
							break;
					}
					
					// Output to file
					// Check if inside function
					if (cdata->f_infunc)
					{
						cdata->c_stacklv += (vsize * arrsz) + (4 - (cdata->c_stacklv % 4));
						fprintf(fcode, "; %-32s = rbp - %i\n", token, cdata->c_stacklv);
					}
					else
					{
						if (stritok(args, "''\"\"", " \t", i + 1) != NULL)
							vdataproc(fdata, l_type, stritok(args, "''\"\"", " \t", i + 1), token, type, cvsize, arrsz);
						else
						{
							if (arrsz > 1) fprintf(fdata, "%-16s r%c %-4i\n", token, cvsize, arrsz);
							else           fprintf(fdata, "%-16s r%c 1\n", token, cvsize);
						}
					}
					
					// Assign variables as vdata in lists of stack variables
					if (cdata->f_infunc)
						vdatainit(cdata->l_s, token, ptrlv, cdata->c_stacklv, type, size, arrsz);
					
					stage++;
				}
				
				if (stage == 2)
					break;
			}
			
			// Show assigned variable data (VERBOSE)
			if (cdata->f_verbose)
			{
				printf("Assigned '%s' as ", token);
				
				switch (type)
				{
					case 'i': fputs("signed ", stdout);   break;
					case 'u': fputs("unsigned ", stdout); break;
					case 'p': fputs("precise ", stdout);  break;
				}
				switch (size)
				{
					case 1: fputs("byte", stdout);  break;
					case 2: fputs("word", stdout);  break;
					case 4: fputs("dword", stdout); break;
					case 8: fputs("qword", stdout); break;
				}
				
				if (ptrlv > 1) printf(" %i-pointer chain.\n\n", ptrlv); else
				if (ptrlv > 0) puts(" pointer.\n");
				else puts(".\n");
			}
		}
	}
	
	return 0;
}

// Open input file, write to output file
int fileproc(struct compiledata *cdata, FILE *fdata, FILE *fcode, char *args)
{
	FILE *fptr = fopen(args, "r+");
	
	// Read line and process into output file
	if (fptr == NULL)
	{
		fprintf(stderr, "\nError: File '%s' either cannot be read, or does not exist.\n\n", args);
		return 1;
	}
	else
	{
		char line[STRING_LENGTH_LINE];
		char argl[STRING_LENGTH_LINE/32][STRING_LENGTH_LINE/8];
		
		printf("Set '%s' as input file.\n", args);
		
		// Read lines of input file
		while (fgets(line, STRING_LENGTH_LINE, fptr))
		{
			cdata->c_linenum++;
			
			// Process line
			strip(line);
			strcom(line, cdata);
			strip(line);
			
			// Extend line if backslash at end of line
			while (line[strlen(line) - 1] == '\\')
			{
				if (fgets(line + strlen(line) - 1, STRING_LENGTH_LINE - strlen(line) - 1, fptr))
				{
					cdata->c_linenum++;
			
					// Process line
					strip(line);
					strcom(line, cdata);
					strip(line);
				}
				else break;
			}
			
			if (strlen(line))
			{
				// (VERBOSE) Show line
				if (cdata->f_verbose) printf("LINE %04u: '%s'\n", cdata->c_linenum, line);
				
				// Loop to find tokens
				for (unsigned int i = 0; (i < STRING_LENGTH_LINE/32) && (strctok(argl[i], line, "''\"\"", " \t,", i)); ++i);
				
				// Parse the line
				if ((vdefproc(cdata, fdata, fcode, line, argl)) 	// Variable definition
				||  (condproc(cdata, fcode, line, argl)))       	// Conditional statements
					return 1;
			}
		}
		
		puts("Read finished.");
		
		fclose(fptr);
		return 0;
	}
}

// Combine data and code files into output file
int fcompile(struct compiledata *cdata, FILE *fout, FILE *fdata, FILE *fcode)
{
	char line[STRING_LENGTH_LINE];
	
	// Write backspace to files so last line is not duplicated
	fputc('\n', fdata);
	fputc('\n', fcode);
	
	// Point to beginning of file
	rewind(fdata);
	rewind(fcode);
	
	// Header
	switch (cdata->f_abi)
	{
		case OS_LINUX: fprintf(fout, "format elf64 executable\n"); break;
		case OS_WIN32: fprintf(fout, "header PE64\n"); break;
	}
	
	// Data section
	switch (cdata->f_abi)
	{
		case OS_LINUX: fprintf(fout, "segment writeable\n"); break;
		case OS_WIN32: fprintf(fout, "section '.data' data readable writeable\n"); break;
	}
	
	// Add constant variables to code section file
	FILE *fc;
	
	// Float constants
	fc = fopen("fconst.txt", "r+");
	
	if (fc != NULL)
	{
		while (fgets(line, STRING_LENGTH_LINE, fc))
		{
			// Process line
			strip(line);
			
			if (strlen(line)) fprintf(fout, "%s\n", line);
			else              fputc('\n', fout);
		}
		
		// Close file
		fclose(fc);
		fc = NULL;
	}
	
	// Double constants
	fc = fopen("dconst.txt", "r+");
	
	if (fc != NULL)
	{
		while (fgets(line, STRING_LENGTH_LINE, fc))
		{
			// Process line
			strip(line);
			
			if (strlen(line)) fprintf(fout, "%s\n", line);
			else              fputc('\n', fout);
		}
		
		// Close file
		fclose(fc);
		fc = NULL;
	}
	
	// String constants
	fc = fopen("sconst.txt", "r+");
	
	if (fc != NULL)
	{
		while (fgets(line, STRING_LENGTH_LINE, fc))
		{
			// Process line
			strip(line);
			
			if (strlen(line)) fprintf(fout, "%s\n", line);
			else              fputc('\n', fout);
		}
		
		// Close file
		fclose(fc);
		fc = NULL;
	}
	
	// Align bytes
	fputs("align 8\n", fout);
	
	// Variables
	while (fgets(line, STRING_LENGTH_LINE, fdata))
	{
		// Process line
		strip(line);
		
		if (strlen(line))
			fprintf(fout, "%s\n", line);
	}
	
	// Code section
	switch (cdata->f_abi)
	{
		case OS_LINUX: fprintf(fout, "segment executable\n"); break;
		case OS_WIN32: fprintf(fout, "section '.text' code readable executable\n"); break;
	}
	
	// Align bytes
	fputs("align 16\n", fout);
	
	while (fgets(line, STRING_LENGTH_LINE, fcode))
	{
		// Process line
		strip(line);
		
		if (strlen(line)) fprintf(fout, "%s\n", line);
		else              fputc('\n', fout);
	}
	
	// Close output file
	if (fout != NULL)
		fclose(fout);
	
	// Remove data section file
	if (fdata != NULL)
	{
		fclose(fdata);
		remove("data.txt");
	}
	
	// Remove code section file
	if (fcode != NULL)
	{
		fclose(fcode);
		remove("codex.txt");
	}
	
	// Remove temporary function code file
	remove("codef.txt");
	
	// Remove constants files
	remove("fconst.txt");
	remove("dconst.txt");
	remove("sconst.txt");
	
	return 0;
}

// Configure compile data in program
int cdataconf(struct compiledata *cdata, char *args)
{
	char
	*flag =  stritok(args, NULL, "-=", 0),
	*value = stritok(args, NULL, "=",  1);
	
	// Search for valid flag
	switch (args[1])
	{
		case '-':
			if (strstr(flag, "help")     == args + 2) goto j_help;     else
			if (strstr(flag, "license")  == args + 2) goto j_license;  else
			if (strstr(flag, "platform") == args + 2) goto j_targetos; else
			if (strstr(flag, "target")   == args + 2) goto j_targetos; else
			if (strstr(flag, "verbose")  == args + 2) goto j_verbose;
			else goto j_flag_invalid;
		
		// Help text
		case 'h':
		j_help:
			printf("Verse Compiler (v%i.%i.%i)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
			puts("Copyright (c) 2021, chris03-dev\n");
			puts("Usage: gcpl <output file> <flag=value> ... <input file> ...\n");
			
			puts("Generates FASM-syntax assembly code from verse-syntax source");
			puts("code and libraries.\n");
			
			puts("Flags may be used anywhere in your console input after the");
			puts("the compiler executable name. Note that the flags are only");
			puts("in effect for file arguments succeeding such flags.\n");
			
			puts("Values are automatically ignored in flags that do not need");
			puts("them.\n");
			
			puts("Flag     Description                            Value");
			puts("-c       Set target CPU architecture            x64,rv64*");
			puts("-h       Read this whole text                   --");
			puts("-l       Read the license of the source code    --");
			puts("         used to build this executable");
			puts("-p -t    Set target platform for assembly       linux,win32*,osx*");
			puts("-s       Compile to assembly code only          --");
			puts("-v       Expose compiler data upon operation    0 - 1");
			puts("         Includes boolean values, lines read,");
			puts("         tokens identified, etc.");
			puts("-vso*    Enable variable substitution to        0 - 1");
			puts("         optimize operations.");
			puts("-vsoadv* Enable advanced variable substitution  0 - 1");
			puts("         to optimize loop and array operations.\n");
			
			puts("*  Not yet fully supported.\n\n");
			
			break;
		
		// License text
		case 'l':
		j_license:
			puts("Mozilla Public License, version 2.0\n");
			puts("This Executable Form was built using its Source Code form that");
			puts("is subject to the terms of the Mozilla Public License, v. 2.0.");
			puts("If a copy of the MPL was not distributed with this Executable");
			puts("form, you can obtain one at:\n");
			puts("'https://mozilla.org/MPL/2.0/'\n");
			puts("If a copy of the Source Code form was not distributed with this");
			puts("Executable Form, you can obtain one at:\n");
			puts("'https://codeberg.org/chris03-dev/verse'\n\n\n");
			break;
		
		// Target platform
		case 'p':
		case 't':
		j_targetos:
			if (value != NULL)
			{
				if (!strcmp(value, "linux"))
				{
					cdata->f_abi = OS_LINUX;
					puts("Set platform target to Linux.");
				}
				else if (!strcmp(value, "win32"))
				{
					cdata->f_abi = OS_WIN32;
					puts("Set platform target to Windows.");
				}
				else if (!strcmp(value, "osx"))
				{
					cdata->f_abi = OS_MACOS;
					puts("Set platform target to Mac OS.");
				}
				else goto j_value_invalid;
			}
			else goto j_value_notfound;
			break;
		
		// Verbose mode
		case 'v':
		j_verbose:
			if (isinteger(value))
			{
				switch (stoi(value))
				{
					case 1:
						cdata->f_verbose = 1;
						puts("Enabled verbose mode.");
						break;
					case 0:
						cdata->f_verbose = 0;
						puts("Disabled verbose mode.");
						break;
					default:
						goto j_value_invalid;
				}
			}
			else goto j_value_notfound;
			break;
		
		// Warnings and errors
		default:
		j_flag_invalid:
			fprintf(stderr, "\nWarning: Invalid flag '%s'.\n\n", flag);
			break;
		j_value_notfound:
			fprintf(stderr, "\nWarning: Value not found on flag '%s'.\n\n", flag);
			break;
		j_value_invalid:
			fprintf(stderr, "\nWarning: Invalid value '%s' for flag '%s'.\n\n", value, flag);
			break;
	}
	return 0;
}

// Open output file
int foutdef(FILE **fptr, char *args)
{
	*fptr = fopen(args, "w+");
	
	// Check for errors
	if (fptr == NULL)
	{
		printf("\nError: File '%s' cannot be read, or does not exist.\n\n", args);
		return 1;
	}
	else
	{
		printf("Set '%s' as output file.\n", args);
		return 0;
	}
}

// Main code
int main(int argc, char **argv)
{
	// FOR DEBUG PURPOSES ONLY
	setbuf(stdout, NULL);
	
	// Clean up files
	remove("data.txt");
	remove("codex.txt");
	remove("codef.txt");
	remove("fconst.txt");
	remove("dconst.txt");
	remove("sconst.txt");
	
	// Compiler data
	struct compiledata cdata = 
	{
		// CPU architecture
		#if   defined __x86_64__
		CPU_X64,
		#elif defined __arm__
		CPU_ARM64,
		#endif
		
		// OS platform target
		#if   defined __linux__
		OS_LINUX,
		#elif defined _WIN32
		OS_WIN32,
		#elif defined __APPLE__
		OS_MACOS,
		#endif
		
		0, 0, 0, 0, {0}, {0},
		{0}, 0, 0, 0, 0, 0, 0, 0, 0,
		
		calloc(sizeof(struct vlist), 1),
		calloc(sizeof(struct vlist), 1),
		
		calloc(sizeof(struct vlist), 1),
		calloc(sizeof(struct vlist), 1),
		calloc(sizeof(struct vlist), 1),
		
		calloc(sizeof(struct vlist), 1)
	};
	
	// Setup files
	FILE
	*fout = NULL,
	*fcode = fopen("codex.txt", "a+"),
	*fdata = fopen("data.txt",  "a+");
	
	// Check for errors
	if ((fcode == NULL) || (fdata == NULL))
	{
		fputs("\nError: Compiler does not have permission to generate files.\n\n", stderr);
		return 1;
	}
	
	// Loop arguments
	if (argc <= 1)
	{
		printf("No input detected.\nInput '%s -h' for instructions.\n", argv[0]);
		return 1;
	}
	for (unsigned int i = 1; i < argc; ++i) 
	{
		// Check for flags
		if (argv[i][0] == '-')
			cdataconf(&cdata, argv[i]);
		
		// Setup output file
		else if (fout == NULL)
		{
			if (foutdef(&fout, argv[i]))
				return 1;
		}
		
		// Read input file
		else if (fileproc(&cdata, fdata, fcode, argv[i]))
			return 1;
	}
	
	// Write to output file
	if (fout != NULL)
	{
		if (cdata.f_noparse == 0)
			fcompile(&cdata, fout, fdata, fcode);
	
		// Free lists
		vdatadestall(cdata.l_i);
		vdatadestall(cdata.l_u);
		vdatadestall(cdata.l_p);
	}
	
	puts("Program successful.");
	
	return 0;
}