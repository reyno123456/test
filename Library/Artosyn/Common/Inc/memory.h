#ifndef __MEMORY__H
#define __MEMORY__H


extern void memory_set(char *s, char c, unsigned int len);
extern unsigned char memory_cmp(const char *s1, const char *s2, unsigned int len);
extern unsigned char memory_strlen(const char *s);
extern void *memory_cpy(void *dest, const void *src, unsigned int len);
#endif

