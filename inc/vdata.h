#ifndef VERSE_HEADER_VDATA
#define VERSE_HEADER_VDATA

#include "macro.h"

struct vlist
{
	int length;
	struct vdata *start, *end;
};

struct vdata
{
	char string[STRING_LENGTH_TOKEN];
	
	unsigned char
	type,
	size,
	ptrlv,
	infun,
	isfun;
	
	int
	arrsz,
	offset; 	// Stack offset of variables/object members
	
	struct vdata *prev, *next;
	struct vlist *list;
};

struct vdata *vdatainit(struct vlist *, char *, unsigned char, unsigned char, unsigned char, int, int);
struct vdata *vdatafind(struct vlist *, const char *);
int           vdatadest(struct vlist *, const char *);
int        vdatadestall(struct vlist *);

#endif