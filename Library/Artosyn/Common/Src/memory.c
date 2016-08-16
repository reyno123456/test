#include "memory.h"

void memory_set(char *s, char c, unsigned int len)
{
    while (len > 0)
    {
        *s = c;
        len--;
        s++;
    }
}


unsigned char memory_cmp(const char *s1, const char *s2, unsigned int len)
{
    while ((len--)&&(*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return (*((unsigned char *)s1) - *((unsigned char *)s2));
}


unsigned char memory_strlen(const char *s)
{
    if ((0 == s)||('\0' == *s))
    {
        return 0;
    }
    else
    {
        return memory_strlen(s + 1) + 1;
    }
}


void *memory_cpy(void *dest, const void *src, unsigned int len)
{
    char *destaddr         = dest;
    const char *srcaddr    = src;

    while (len-- > 0)
    {
        *destaddr++ = *srcaddr++;
    }

    return dest;
}

void memset(char *s, char c, unsigned int len)
{
    memory_set(s, c, len);
}

void *memcpy(void *dest, const void *src, unsigned int len)
{
    memory_cpy(dest, src, len);
}

