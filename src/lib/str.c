#include <ultra64.h>
#include "lib/fault.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*

char *strcpy(char *dst, const char *src)
{
	char *ptr = dst;
	char c;

	if (dst == NULL) {
		faultAssert("s1", "str.c", 44);
	}

	if (src == NULL) {
		faultAssert("s2", "str.c", 45);
	}

	*ptr = c = *src;
	ptr++;
	src++;

	while (c != '\0') {
		*ptr = c = *src;
		ptr++;
		src++;
	}

	return dst;
}

char *strncpy(char *dst, const char *src, size_t len)
{
	char *ptr = dst;
	char c;

	*ptr = c = *src;
	ptr++;
	src++;

	while (c != '\0') {
		len--;

		if (len == 0) {
			break;
		}

		*ptr = c = *src;
		ptr++;
		src++;
	}

	while (len--) {
		*ptr = '\0';
		ptr++;
	}

	return dst;
}

char *strcat(char *dst, const char *src)
{
	char *ptr = dst;
	char c;

	while (*ptr != '\0') {
		ptr++;
	}

	*ptr = c = *src;
	ptr++;
	src++;

	while (c != '\0') {
		*ptr = c = *src;
		ptr++;
		src++;
	}

	return dst;
}

s32 strcmp(const char *s1, const char *s2)
{
	while (true) {
		if (*s1 != *s2) {
			if (*s1 < *s2) {
				return -1;
			} else {
				return 1;
			}
		}

		if (*s1 == '\0') {
			return 0;
		}

		s1++;
		s2++;
	}
}

s32 strncmp(const char *s1, const char *s2, s32 len)
{
	while (true) {
		if (len == 0) {
			return 0;
		}

		if (*s1 != *s2) {
			if (*s1 < *s2) {
				return -1;
			} else {
				return 1;
			}
		}

		if (*s1 == '\0') {
			return 0;
		}

		len--;

		s1++;
		s2++;
	}
}

char toupper(char c)
{
	if (c >= 'a' && c <= 'z') {
		return c - 0x20;
	}

	return c;
}


s32 isdigit(char c)
{
	return c >= '0' && c <= '9';
}

s32 isalpha(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

s32 isspace(char c)
{
	return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\v';
}
*/


s32 strtol(const char *src, const char **endptr, s32 base)
{
	bool negative;
	u32 cutoff;
	u32 cutlim;
	u32 value;
	const char *ptr;
	char c;
	const char *save;
	bool overflow;

	if (base < 0 || base == 1 || base > 36) {
		base = 10;
	}

	ptr = src;

	while (isspace(*ptr)) {
		ptr++;
	}

	if (*ptr != '\0') {
		if (*ptr == '-') {
			negative = true;
			ptr++;
		} else if (*ptr == '+') {
			negative = false;
			ptr++;
		} else {
			negative = false;
		}

		if (base == 16 && ptr[0] == '0' && toupper(ptr[1]) == 'X') {
			ptr += 2;
		}

		if (base == 0) {
			if (*ptr == '0') {
				if (toupper(ptr[1]) == 'X') {
					ptr += 2;
					base = 16;
				} else {
					base = 8;
				}
			} else {
				base = 10;
			}
		}

		save = ptr;

		cutoff = U32_MAX / base;
		cutlim = U32_MAX % base;
		overflow = false;
		value = 0;

		for (c = *ptr; c != '\0'; c = *++ptr) {
			if (isdigit(c)) {
				c -= '0';
			} else if (isalpha(c)) {
				c = toupper(c) - 'A' + 10;
			} else {
				break;
			}

			if (c >= base) {
				break;
			}

			if (value > cutoff || (value == cutoff && (uintptr_t) c > cutlim)) {
				overflow = true;
			} else {
				value *= base;
				value += (uintptr_t) c;
			}
		}

		if (ptr != save) {
			if (endptr != NULL) {
				*endptr = ptr;
			}

			if (overflow) {
				return -1;
			}

			return negative ? -value : value;
		}
	}

	if (endptr != NULL) {
		*endptr = src;
	}

	return 0;
}
