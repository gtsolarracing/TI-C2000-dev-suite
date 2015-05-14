/*
 * logging.h
 *
 *  Created on: Jun 12, 2014
 *      Author: Alex
 */

#include <stdio.h>
#include <string.h>

#ifndef LOGGING_H_
#define LOGGING_H_

// Use for debugging with a computer, disable for production build:
#define STDIODEBUG 1

void stdiologstr(char *str);
void stdiolog1int(char* format, int num);



#endif /* LOGGING_H_ */
