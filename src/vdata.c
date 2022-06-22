#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/vdata.h"

// Create vdata
struct vdata *vdatainit(struct vlist *l, char *args, unsigned char type, unsigned char size, unsigned char ptrlv, int offset, int arrsz)
{
	struct vdata *v = calloc(sizeof(struct vdata), 1);
	
	if (l != NULL)
	{
		// Initialize list
		if (l->start == NULL)
		{
			l->start = v;
			l->end = v;
		}
		
		// Assign new vdata as end of list
		else
		{
			v->prev = l->end;
			l->end->next = v;
			l->end = v;
		}
		
		// Setup length
		l->length++;
	}
	
	// Setup data on new vdata
	v->ptrlv = ptrlv;
	v->offset = offset;
	v->type = type;
	v->size = size;
	v->arrsz = arrsz;
	
	strcpy(v->string, args);
	
	return v;
}

int vldest(struct vdata *v)
{
	if (v->next != NULL) vldest(v->next);
	if (v != NULL) free(v);
	
	return 0;
}

// Destroy list of vdata
int vdatadestall(struct vlist *l)
{
	// Loop through freeing vdata
	for (struct vdata *v = l->start; v != NULL; v = v->next)
	{
		// Remove vdata
		if (v->next == NULL) free(v);
		if (v->prev != NULL) free(v->prev);
	}
	
	// Reset vlist data
	memset(l, 0, sizeof(struct vlist));
	return 0;
}

// Search vdata from list
struct vdata *vdatafind(struct vlist *l, const char *s)
{
	// Check if null
	if ((l == NULL) || (s == NULL) || (l->length == 0))
		return NULL;
	
	// Loop through freeing vdata
	for (struct vdata *v = l->start; v != NULL; v = v->next)
	{
		// Check if equal
		if (!strcmp(v->string, s))
			return v;
	}
	
	// Find unsuccessful
	return NULL;
}

// Search and destroy vdata from list
int vdatadest(struct vlist *l, const char *s)
{
	// Check if null
	if ((l == NULL) || (s == NULL) || (l->length == 0))
		return 1;
	
	// Loop through freeing vdata
	for (struct vdata *v = l->start; v != NULL; v = v->next)
	{
		// Check if equal
		if (!strcmp(v->string, s))
		{
			// Remove vdata
			if (v->prev != NULL)
			{
				// End of list
				if (v->next == NULL)
				{
					l->end = v->prev;
					v->prev->next = NULL;
				}
				// Middle of list
				else
				{
					v->prev->next = v->next;
					v->next->prev = v->prev;
				}
			}
			else
			{
				// Lone vdata in list
				if (v->next == NULL)
				{
					l->start = NULL;
					l->end = NULL;
				}
				// Start of light
				else
				{
					l->start = v->next;
					v->next->prev = NULL;
				}
			}
			
			free(v);
			
			// Update vlist data
			l->length--;
			
			return 0;
		}
	}
	
	// Find unsuccessful
	return 1;
}