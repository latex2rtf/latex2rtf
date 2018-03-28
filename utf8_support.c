/* utf8_support.c - LaTeX to RTF conversion program

This file contains a function used to convert verbatim sections containing
Utf-8 characters properly.

Authors:
    2018 Alex Itkes
*/

#include "main.h"
#include "chars.h"
#include "fonts.h"
#include "encodings.h"

/* Initializes a unicode character represented by the Utf-8 sequencd
   based on the first byte of the sequence. The returned value must
   then be updated with updateUf8Sequence a number of times (returned
   by getUtf8SequenceLength using the following bytes of the input
   stream.
*/
uint16_t getUtf8SequenceInitialValue (uint8_t byte)
{
    if (byte >= 0xF0) {
        return byte & ~0xF0;
    } else if (byte >= 0xE0) {
        return byte & ~0xE0;
    } else if (byte >= 0xC0) {
        return byte & ~0xC0;
    } else {
        return 0;
    }
}

/* Determines the length of a Utf-8 sequence based on its first byte.
   Actually returns the length decreased by 1, i.e. the number of
   bytes to be read later.
*/
uint16_t getUtf8SequenceLength (uint8_t byte)
{
    if (byte >= 0xF0) {
        return 3;
    } else if (byte >= 0xE0) {
        return 2;
    } else if (byte >= 0xC0) {
        return 1;
    } else {
        return 0;
    }
}

/* Adds a following byte of Utf-8 sequence to the unicode
   character, return the resulting code.
*/
uint16_t updateUtf8Sequence (uint16_t value, uint8_t byte)
{
    return ((value << 6) + (byte & ~0xC0));
}

/* Prints a string to RTF with escaped characters if needed.
   unlike putRtfStrEscaped it prints Utf8 characters properly.
*/
void putRtfUtf8StrEscaped(const char * string)
{
    char *s = (char *) string;
    if (string == NULL) return;
    while (*s) {
        /* Much of code actually copied from Convert () function, but some
           code later moved to additional functions (getUtf8SequenceLength,
           getUtf8SequenceInitialValue and updateUtf8Sequence).
           TODO: Could it be a good idea to call them also from Convert?
        */
        if ((uint8_t)(*s) >= 0x80 && (CurrentFontEncoding() == ENCODING_UTF8) && *(s + 1)) {
            /* Handle a Utf-8 byte sequence. Try to convert it to a Unicode
               character in same way with the Convert function does and then
               output it with CmdUnicodeChar.
            */
            uint8_t byte = *s;

            uint16_t len = getUtf8SequenceLength (byte);
            uint16_t value = getUtf8SequenceInitialValue (byte);
            uint16_t i;

            s++;
            for (i=0; i<len; i++) {
                if (*s) {
                    value = updateUtf8Sequence (value, *s);
                    s++;
                } else {
                    /* If the sequnce is shorted then it should be, display warning and output nothing. */
                    diagnostics(1, "An incorrect Utf-8 sequence encountered at end of string '%s', continuing anyway...", string);
                    return;
                }
            }

            diagnostics(4,"(flag = 0x%X) char value = 0X%04X or %u (%u bytes)", (unsigned char) byte, value, value, len);
            CmdUnicodeChar(value);
        } else {
            /* Not a Utf8 character. */
            putRtfCharEscaped(*s++);
        }
    }
}
