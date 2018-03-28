#ifndef _UTF8_SUPPORT_H_INCLUDED
#define _UTF8_SUPPORT_H_INCLUDED 1

uint16_t getUtf8SequenceInitialValue (uint8_t byte);
uint16_t getUtf8SequenceLength (uint8_t byte);
uint16_t updateUf8Sequence (uint16_t value, uint8_t byte);

void putRtfUtf8StrEscaped(const char * string);

#endif
