/*
 * logging.c
 *
 *  Created on: Jun 12, 2014
 *      Author: Alex
 */
#include "logging.h"

/**
 * \brief Log a string.
 */
void stdiologstr(char *str) {
#ifdef STDIODEBUG
	puts(str);
#endif
}

/**
 * \brief Log a formatted string with 1 integer.
 */
void stdiolog1int(char* format, int num) {
#ifdef STDIODEBUG
	char str[39];
	if (strlen(format) + 6 > 32) return; // Don't want to exceed the maximum string length!
	sprintf(str, format, num);
	puts(str);
#endif

}
