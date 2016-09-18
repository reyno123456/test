#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

#define HEAP_BUFFER_SIZE (1024*8)
static unsigned char heap_buffer[HEAP_BUFFER_SIZE] = {0};
static unsigned int mem_malloc_start = (unsigned int)(&heap_buffer);
static unsigned int mem_malloc_end;
static unsigned int mem_malloc_brk;
#define MORECORE_FAILURE (0)

static unsigned char heap_initialized = 0;

static void heap_init(void)
{
    if (heap_initialized == 0)
    {
        mem_malloc_end = mem_malloc_start + HEAP_BUFFER_SIZE;
        mem_malloc_brk = mem_malloc_start;        
        heap_initialized = 1;
    }
}

void *sbrk(ptrdiff_t increment)
{
    unsigned int old = mem_malloc_brk;
    unsigned int new = old + increment;

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
        return (void *)MORECORE_FAILURE;
    }

    mem_malloc_brk = new;

    return (void *)old;
}

#define NALLOC 16 //1024    /* minimum #units to request */
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
    free((void *)(up+1));
    
    return freep;
}

__attribute__((weak)) void *malloc(size_t size)
{
    Header *p, *prevp;
    unsigned int nbytes = size;
    unsigned int nunits; 

    heap_init();
    
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

__attribute__((weak)) void free(void *ap)
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

