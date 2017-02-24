#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "memory.h"

typedef long Align;/*for alignment to long boundary*/

union header
{ 
    struct
    {
        union header *ptr; /*next block if on free list*/
        unsigned size; /*size of this block*/
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep = NULL;

extern char end;
static unsigned int mem_malloc_start = (unsigned int)(&end);
static unsigned int mem_malloc_end;
static unsigned int mem_malloc_brk = (unsigned int)(&end);
#define MORECORE_FAILURE (0)

static inline void stack_get_position(void) __attribute__((optimize("O0")));

static inline void stack_get_position(void)
{
    __asm("MOV R0, SP");
    __asm("LDR R1, =mem_malloc_end");    
    __asm("STR R0, [R1]");
}

static void *sbrk(ptrdiff_t increment)
{
    unsigned int old = mem_malloc_brk;
    unsigned int new = old + increment;

    stack_get_position();

    /*
     * if we are giving memory back make sure we clear it out since
     * we set MORECORE_CLEARS to 1
     */
    if (increment < 0)
    {
        memset((void *)new, 0, -increment);
    }

    if ((new < mem_malloc_start) || (new > mem_malloc_end))
    {
        dlog_error("malloc out of boundary!");
        return (void *)MORECORE_FAILURE;
    }

    mem_malloc_brk = new;

    dlog_info("brk = 0x%x, sp = 0x%x", mem_malloc_brk, mem_malloc_end);

    return (void *)old;
}

#define NALLOC 128 //1024    /* minimum #units to request */
static Header *morecore(unsigned nu)
{
    char *cp;
    Header *up;
    
    if(nu < NALLOC)
    {
        nu = NALLOC;
    }
    
    cp = sbrk(nu * sizeof(Header));
    if(cp == (char *)MORECORE_FAILURE)    /* no space at all*/
    {
        return NULL;
    }
    
    up = (Header *)cp;
    up->s.size = nu;
    free_simple((void *)(up+1));
    
    return freep;
}

void *malloc_simple(size_t size)
{
    Header *p, *prevp;
    unsigned int nbytes = size;
    unsigned int nunits; 

    nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;
    
    if ((prevp = freep) == NULL)  /* no free list */
    {
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }
    
    for (p = prevp->s.ptr; ;prevp = p, p= p->s.ptr)
    {
        if (p->s.size >= nunits) /* big enough */
        {
            if (p->s.size == nunits)  /* exactly */
            {
                prevp->s.ptr = p->s.ptr;
            }
            else
            {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p+1);
        }
        if (p== freep) /* wrapped around free list */
        {
            if ((p = morecore(nunits)) == NULL)
            {
                return NULL; /* none left */
            }
        }
    }
}

void free_simple(void *ap)
{
    Header *bp,*p;
    bp = (Header *)ap -1; /* point to block header */
    
    for (p=freep;!(bp>p && bp< p->s.ptr);p=p->s.ptr)
    {
        if(p>=p->s.ptr && (bp>p || bp<p->s.ptr))
        {
            break;    /* freed block at start or end of arena*/
        }
    }
    
    if (bp+bp->s.size==p->s.ptr)    /* join to upper nbr */
    {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
    {
        bp->s.ptr = p->s.ptr;
    }
    
    if (p+p->s.size == bp)     /* join to lower nbr */
    {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
    {
        p->s.ptr = bp;
    }
    
    freep = p;
}

__attribute__((weak)) void *malloc(size_t size)
{
    return malloc_simple(size);
}

__attribute__((weak)) void free(void *ap)
{
    free_simple(ap);
}

__attribute__((weak)) void *realloc(void *ptr, size_t size)
{
    void *new;

    new = malloc(size);
    if (!new)
    {
        return NULL;
    }

    if (ptr)    
    {
        free(ptr);
    }

    return new;
}

__attribute__((weak)) void *calloc (size_t n, size_t elem_size)
{
    void *result;
    size_t sz = n * elem_size;

    result = malloc (sz);
    if (result != NULL)
        memset (result, 0, sz);

    return result;
}


