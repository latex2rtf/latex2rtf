#ifndef _AUX_H_INCLUDED
#define _AUX_H_INCLUDED 1
/*
 * open and read an auxiliary file.
 * filter lines which start with the
 * macros contained in char *macros[]
 * and send them to ConvertString()
 */

void LoadAuxFile(void);
#endif
