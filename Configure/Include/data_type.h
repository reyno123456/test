
#ifndef __DATATYPE_H
#define __DATATYPE_H

#define ECHO
#define ENUMERATE
#define FPGA

#if 0
#ifndef int16_t
typedef  INT16  int16_t;
#endif
#ifndef uint16_t
typedef UINT16 uint16_t;
#endif
#ifndef int32_t
typedef  INT32  int32_t;
#endif
#ifndef int64_t
typedef  INT64  int64_t;
#endif
#ifndef uint32_t
typedef UINT32 uint32_t;
#endif
#ifndef uint64_t
typedef UINT64 uint64_t;
#endif
#endif

#define USE_FULL_ASSERT
/* shift bit field */
#define SBF(f, v)    ((v) << (f))

#ifndef BIT
#define BIT(n)    ((uint32_t)1 << (n))
#endif/* BIT */

/** @addtogroup Exported_macro
 * * @{
 * */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

#define __IO    volatile             /*!< Defines 'read / write' permissions */

#define m7_malloc             pvPortMalloc
#define m7_free               vPortFree

/*
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef uint32_t uint32_t;
typedef unsigned unsigned long int uint64_t;
*/

#define NULL ((void*)0)

#endif

