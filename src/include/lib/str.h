#ifndef _IN_LIB_STR_H
#define _IN_LIB_STR_H

#include <ultra64.h>
#include "data.h"
#include "types.h"

// Note PC: use builtin functions
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t len);
char *strcat(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t len);
int toupper(int c);
int isdigit(int c);
int isalpha(int c);
int isspace(int c);

s32 strtol(const char *src, const char **endptr, s32 base);

#endif
