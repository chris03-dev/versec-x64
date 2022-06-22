#ifndef VERSE_HEADER_MAIN
#define VERSE_HEADER_MAIN

#include <stdio.h>

#include "macro.h"
#include "cdata.h"
#include "node.h"

int shiftdiff(long long unsigned int);
int shiftbase(long long unsigned int);

// Assembly processes
int    asmbspillx64_s(FILE *, struct compiledata *, const char *, char *);
int    asmbspillx64_l(FILE *, struct compiledata *, const char *, char *);
int        asmpreproc(FILE *, struct compiledata *, struct node *);
int        asmprocx64(FILE *, struct compiledata *, struct node *, struct node *);
int      asmprocarm64(FILE *, struct compiledata *, struct node *, struct node *);
int       callprocx64(FILE *, struct compiledata *, struct node *);
struct node* nodeproc(FILE *, struct compiledata *, struct node *, char *, int);

// Function processes
int  fmodproc(struct compiledata *, FILE *, int);
int kwordproc(struct compiledata *, FILE *, char *);//, char[STRING_LENGTH_LINE/32][STRING_LENGTH_LINE/8]);

// Data definition processes
int vdataproc(FILE *, struct vlist *, char *, char *, int, int, int);
int  vdefproc(struct compiledata *, FILE *, FILE *, char *);

// File compilation processes
int fileproc(struct compiledata *, FILE *, FILE *, char *);
int fcompile(struct compiledata *, FILE *, FILE *, FILE *);

// Main processes
int cdataconf(struct compiledata *, char *);
int   foutdef(FILE **, char *);
int      main(int, char **);

#endif