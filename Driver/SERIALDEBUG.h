/*
 * debug.h
 *
 *		Project: Pilo
 *  	Original Project: LamborjohnnyII
 *      Author: Laci Kundra
 *      Mod: Debug to memory
 *
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <sys/types.h>

void Debug_Init( void );
void Debug_String( char *s );
void Debug_Printf(const char *fmt, ...);
void Debug_PrintResetType( void );
void Debug_PrintChars(const char* buf, int len);

uint32_t bytes;


#endif /* DEBUG_H_ */