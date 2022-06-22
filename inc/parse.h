#ifndef VERSE_HEADER_PARSE
#define VERSE_HEADER_PARSE

#include "cdata.h"

#define MAX_NESTLEVEL_FIELDLIMITER 64

// String checking functions
int  strchrcount(const char *, char);
int strnchrcount(const char *, char, size_t);

// Token functions
int  strltoklen(const char *, const char *, const char *, size_t);
int strfltoklen(const char *, const char *, const char *, const char *, size_t);

char  *strltok(char *, const char *, const char *, const char *, size_t);
char *strfltok(char *, const char *, const char *, const char *, const char *, size_t);

char  *stritok(const char *, const char *, const char *, size_t);
char *striftok(const char *, const char *, const char *, const char *, size_t);

// String-number conversion functions
int      isinteger(const char *);
int      isprecise(const char *);
long long int stoi(const char *);
double        stod(const char *);

// String modification functions
char *strstrip(char *);
int strpreproc(char *, struct compiledata *);

#endif