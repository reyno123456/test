#include <stdio.h>
#include <string.h>
#include "memory_config.h"
#include "debuglog.h"
#include "sys_event.h"
#include "serial.h"
#include "reg_map.h"
#include "cpu_info.h"

static uint8_t s_u8_dlogServerCpuId = 0xFF;

#define DEBUG_LOG_OUTPUT_BUF_HEAD_0      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_0      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_0))
#define DEBUG_LOG_OUTPUT_BUF_HEAD_1      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_1      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_1))
#define DEBUG_LOG_OUTPUT_BUF_HEAD_2      ((char*)(((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->buf))
#define DEBUG_LOG_OUTPUT_BUF_TAIL_2      ((char*)(SRAM_DEBUG_LOG_OUTPUT_BUFFER_END_ADDR_2))

#define DEBUG_LOG_OUTPUT_BUF_WR_POS_0    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_0    (s_debug_log_output_buf_rd_pos_0)
#define DEBUG_LOG_OUTPUT_BUF_WR_POS_1    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_1    (s_debug_log_output_buf_rd_pos_1)
#define DEBUG_LOG_OUTPUT_BUF_WR_POS_2    (((STRU_DebugLogOutputBuffer*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->header.output_buf_wr_pos)
#define DEBUG_LOG_OUTPUT_BUF_RD_POS_2    (s_debug_log_output_buf_rd_pos_2)

#define DEBUG_LOG_INPUT_BUF_HEAD         ((char*)(((STRU_DebugLogInputBuffer*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->buf))
#define DEBUG_LOG_INPUT_BUF_TAIL         ((char*)(SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR + SRAM_DEBUG_LOG_INPUT_BUFFER_SIZE - 1))

#define DEBUG_LOG_INPUT_BUF_WR_POS       (((STRU_DebugLogInputBuffer*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->header.input_buf_wr_pos)
#define DEBUG_LOG_INPUT_BUF_RD_POS_0     (s_debug_log_input_buf_rd_pos_0)
#define DEBUG_LOG_INPUT_BUF_RD_POS_1     (s_debug_log_input_buf_rd_pos_1)
#define DEBUG_LOG_INPUT_BUF_RD_POS_2     (s_debug_log_input_buf_rd_pos_2)

#define CHECK_DEBUG_BUF_INIT_STATUS()    do { if (DLOG_CheckDebugBufInitStatus() == 0 ) return 0; } while(0)

#define SRAM_DEBUG_BUF_INIT_FLAG         0x30A5A503

typedef struct
{
    volatile uint32_t input_buf_wr_pos;
    volatile uint32_t input_buf_init_flag;
} STRU_DebugLogInputBufferHeader;

typedef struct
{
    volatile STRU_DebugLogInputBufferHeader header;
    volatile char buf[1];
} STRU_DebugLogInputBuffer;

typedef struct
{
    volatile uint32_t output_buf_wr_pos;
    volatile uint32_t output_buf_init_flag;
} STRU_DebugLogOutputBufferHeader;

typedef struct
{
    volatile STRU_DebugLogOutputBufferHeader header;
    volatile char buf[1];
} STRU_DebugLogOutputBuffer;

static char *s_debug_log_output_buf_rd_pos_0 = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
static char *s_debug_log_output_buf_rd_pos_1 = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
static char *s_debug_log_output_buf_rd_pos_2 = DEBUG_LOG_OUTPUT_BUF_HEAD_2;

static char *s_debug_log_input_buf_rd_pos_0 = DEBUG_LOG_INPUT_BUF_HEAD;
static char *s_debug_log_input_buf_rd_pos_1 = DEBUG_LOG_INPUT_BUF_HEAD;
static char *s_debug_log_input_buf_rd_pos_2 = DEBUG_LOG_INPUT_BUF_HEAD;

static unsigned char s_u8_commandPos;
static unsigned char s_u8_commandLine[50];
static FUNC_CommandRun s_func_commandRun = NULL;

__attribute__((weak)) ENUM_CPU_ID CPUINFO_GetLocalCpuId(void) 
{
    return *((ENUM_CPU_ID*)CPU_ID_INFO_ADDRESS);
}

__attribute__((weak)) void uart_putc(unsigned char index, char c)
{
}

__attribute__((weak)) void uart_puts(unsigned char index, const char *s)
{
}

__attribute__((weak)) void serial_puts(const char *s)
{
}

__attribute__((weak)) int32_t UART_ClearTflCnt(uint8_t u8_uartCh)
{
}

static unsigned char DLOG_CheckDebugBufInitStatus(void)
{
    // Check input SRAM buffer init flag 
    
    if (((volatile STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_init_flag != SRAM_DEBUG_BUF_INIT_FLAG) 
    {
        return 0; 
    }

    // Check output SRAM buffer init flag 
    
    unsigned int out_buf_addr;
    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0;
        break;
    case ENUM_CPU1_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1;
        break;
    case ENUM_CPU2_ID:
        out_buf_addr = SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2;
        break;
    }
    
    if (((STRU_DebugLogOutputBufferHeader*)out_buf_addr)->output_buf_init_flag != SRAM_DEBUG_BUF_INIT_FLAG) 
    {
        return 0; 
    }

    return 1;
}

static unsigned int DLOG_Input(char* buf, unsigned int byte_num)
{
    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() != s_u8_dlogServerCpuId)
    {
        return 0;
    }

    unsigned int iByte = 0;

    char* target = (char*)DEBUG_LOG_INPUT_BUF_WR_POS;

    // Copy to log buffer
    while (iByte < byte_num)
    {
        *target = buf[iByte];

        if (target >= DEBUG_LOG_INPUT_BUF_TAIL)
        {
            target = DEBUG_LOG_INPUT_BUF_HEAD;
        }
        else
        {
            target++;
        }

        iByte++;
    }

    DEBUG_LOG_INPUT_BUF_WR_POS = (uint32_t)target;
    
    return iByte;
}

static void DLOG_Uart_IrqHandler(uint32_t u32_vectorNum)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    unsigned int          isrType2;
    volatile uart_type   *uart_regs = (uart_type *)UART0_BASE;

    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;
    isrType2   = isrType;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            /* receive "enter" key */
            if (c == '\r')
            {
                uart_putc(DEBUG_LOG_UART_PORT, '\n');
                s_u8_commandLine[s_u8_commandPos++] = c;

                /* if s_u8_commandLine is not empty, go to parse command */
                if (s_u8_commandPos > 0)
                {
                    DLOG_Input(s_u8_commandLine, s_u8_commandPos);
                    s_u8_commandPos = 0;
                    memset(s_u8_commandLine, 0, sizeof(s_u8_commandLine));
                }
            }
            /* receive "backspace" key */
            else if (c == '\b')
            {
                if (s_u8_commandPos > 1)
                {
                    s_u8_commandLine[--s_u8_commandPos] = '\0';
                }
                uart_putc(DEBUG_LOG_UART_PORT, '\b');
                uart_putc(DEBUG_LOG_UART_PORT, ' ');
                uart_putc(DEBUG_LOG_UART_PORT, '\b');
            }
            /* receive normal data */
            else if (s_u8_commandPos < (sizeof(s_u8_commandLine) - 1))
            {
                uart_putc(DEBUG_LOG_UART_PORT, c);
                s_u8_commandLine[s_u8_commandPos++] = c;
            }
        }
    }

    // TX empty interrupt.
    if (UART_IIR_THR_EMPTY == (isrType2 & UART_IIR_THR_EMPTY))
    {
        UART_ClearTflCnt(0);
    }
}

static void DLOG_InputCommandInit(void)
{
    s_u8_commandPos = 0;
    memset(s_u8_commandLine, 0, sizeof(s_u8_commandLine));
    reg_IrqHandle(UART_INTR0_VECTOR_NUM, DLOG_Uart_IrqHandler, NULL);
    INTR_NVIC_SetIRQPriority(UART_INTR0_VECTOR_NUM, INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5, INTR_NVIC_PRIORITY_UART0, 0));
    INTR_NVIC_EnableIRQ(UART_INTR0_VECTOR_NUM);
}

unsigned int DLOG_InputParse(char *buf, unsigned int byte_max)
{
    unsigned char u8_dataValid = 0;
    unsigned int iByte = 0;
    char **p_src;

    CHECK_DEBUG_BUF_INIT_STATUS();

    ENUM_CPU_ID e_cpuID = CPUINFO_GetLocalCpuId();

    switch (e_cpuID)
    {
    case ENUM_CPU0_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_0;
        break;
    case ENUM_CPU1_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_1;
        break;
    case ENUM_CPU2_ID:
        p_src = &DEBUG_LOG_INPUT_BUF_RD_POS_2;
        break;
    default:
        return 0;
    }

    char *src = *p_src;

    while (src != (char *)DEBUG_LOG_INPUT_BUF_WR_POS)
    {
        *buf++ = *src;

        if ((*src == '\n') || (*src == '\r'))
        {
            u8_dataValid = 1;
        }

        if (src >= DEBUG_LOG_INPUT_BUF_TAIL)
        {
            src = DEBUG_LOG_INPUT_BUF_HEAD;
        }
        else
        {
            src++;
        }
        iByte++;

        if ((iByte >= byte_max) || (u8_dataValid == 1))
        {
            break;
        }
    }

    if (u8_dataValid == 1)
    {
        *p_src = src;
        return iByte;
    }
    else
    {
        return 0;
    }
}

static void DLOG_InputCommandParse(char *cmd)
{
    unsigned char cmdIndex;
    char *tempCommand[6];

    cmdIndex = 0;
    memset(tempCommand, 0, sizeof(tempCommand));

    while (cmdIndex < 6)
    {
        /* skip the sapce */
        while ((*cmd == ' ') || (*cmd == '\t'))
        {
            ++cmd;
        }

        /* end of the cmdline */
        if (*cmd == '\0')
        {
            tempCommand[cmdIndex] = 0;
            break;
        }

        tempCommand[cmdIndex++] = cmd;

        /* find the end of string */
        while (*cmd && (*cmd != ' ') && (*cmd != '\t'))
        {
            ++cmd;
        }

        /* no more command */
        if (*cmd == '\0')
        {
            tempCommand[cmdIndex] = 0;
            break;
        }

        /* current cmd is end */
        *cmd++ = '\0';
    }

    if (s_func_commandRun != NULL)
    {
        s_func_commandRun(tempCommand, cmdIndex);
    }
}

void DLOG_Process(void* p)
{
    char commandBuf[50];
    memset(commandBuf, 0, sizeof(commandBuf));

    while(DLOG_InputParse(commandBuf, sizeof(commandBuf)))
    {
        DLOG_InputCommandParse(commandBuf);
        memset(commandBuf, 0, sizeof(commandBuf));
    }

    while(DLOG_Output(3000))
    {
    }
}

void DLOG_Init(FUNC_CommandRun func, ENUM_DLOG_PROCESSOR e_dlogProcessor)
{
    s_func_commandRun = func;

    if (e_dlogProcessor == DLOG_SERVER_PROCESSOR)
    {
        s_u8_dlogServerCpuId = CPUINFO_GetLocalCpuId();
        
        ((STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_wr_pos = (uint32_t)DEBUG_LOG_INPUT_BUF_HEAD;
        ((STRU_DebugLogInputBufferHeader*)SRAM_DEBUG_LOG_INPUT_BUFFER_ST_ADDR)->input_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;

        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_0;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_0)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;
        
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_1;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_1)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;
        
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->output_buf_wr_pos = (uint32_t)DEBUG_LOG_OUTPUT_BUF_HEAD_2;
        ((STRU_DebugLogOutputBufferHeader*)SRAM_DEBUG_LOG_OUTPUT_BUFFER_ST_ADDR_2)->output_buf_init_flag = SRAM_DEBUG_BUF_INIT_FLAG;

        DLOG_InputCommandInit();
    }
    else
    {
        while (DLOG_CheckDebugBufInitStatus() == 0 ) ;
    }

#ifdef USE_SYS_EVENT_TRIGGER_DEBUG_LOG_PROCESS
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, DLOG_Process);
#endif
}

#ifdef DEBUG_LOG_USE_SRAM_OUTPUT_BUFFER

static unsigned int DLOG_StrCpyToDebugOutputLogBuf(const char *src)
{
    char* dst;
    char* head;
    char* tail;

    CHECK_DEBUG_BUF_INIT_STATUS();

    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_0;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_0;
        break;
    case ENUM_CPU1_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_1;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_1;
        break;
    case ENUM_CPU2_ID:
        dst = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_2;
        head = DEBUG_LOG_OUTPUT_BUF_HEAD_2;
        tail = DEBUG_LOG_OUTPUT_BUF_TAIL_2;
        break;
    default:
        return 0;
    }

    while (*src)
    {
        *dst = *src;
        if (dst >= tail)
        {
            dst = head;
        }
        else
        {
            dst++;
        }
        src++;
    }

    switch(CPUINFO_GetLocalCpuId())
    {
    case ENUM_CPU0_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_0 = (uint32_t)dst;
        break;
    case ENUM_CPU1_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_1 = (uint32_t)dst;
        break;
    case ENUM_CPU2_ID:
        DEBUG_LOG_OUTPUT_BUF_WR_POS_2 = (uint32_t)dst;
        break;
    default:
        return 0;
    }

    return 1;
}

unsigned int DLOG_Output(unsigned int byte_num)
{
    unsigned int iByte = 0;

    char **p_src;
    char* src;
    char* write_pos;
    char* head;
    char* tail;
    
    unsigned char output_index;
    unsigned char enter_detected;

    char tmp_buf[256];
    unsigned int tmp_buf_index;

    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() != s_u8_dlogServerCpuId)
    {
        return 0;
    }

    // Print output buffer 0,1,2 to serial 
    output_index = 0;
    while (output_index <= 2) 
    {
        switch (output_index)
        {
        case 0:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_0;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_0;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_0;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_0;
            break;
        case 1:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_1;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_1;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_1;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_1;
            break;
        case 2:
            p_src  = &DEBUG_LOG_OUTPUT_BUF_RD_POS_2;
            write_pos = (char*)DEBUG_LOG_OUTPUT_BUF_WR_POS_2;
            head = DEBUG_LOG_OUTPUT_BUF_HEAD_2;
            tail = DEBUG_LOG_OUTPUT_BUF_TAIL_2;
            break;
        default:
            return 0;
        }

        src = *p_src;
        tmp_buf_index = 0;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        enter_detected = 0;

        while (src != write_pos)
        {
            if (tmp_buf_index < sizeof(tmp_buf))
            {
                tmp_buf[tmp_buf_index++] = *src;
            }

            if (*src == '\n')
            {
                enter_detected = 1;
            }

            if (src >= tail)
            {
                src = head;
            }
            else
            {
                src++;
            }

            if (enter_detected == 1)
            {
                uart_puts(DEBUG_LOG_UART_PORT, tmp_buf);
                uart_putc(DEBUG_LOG_UART_PORT, '\r');
                
                iByte += tmp_buf_index;
                
                *p_src = src;

#ifdef DEBUG_LOG_OUTPUT_CPU_AFTER_CPU
                // Output cpu0, cpu1, cpu2.
                tmp_buf_index = 0;
                memset(tmp_buf, 0, sizeof(tmp_buf));
                enter_detected = 0;
#else
                // Output line by line
                break;
#endif
            }
        }

        if (iByte >= byte_num)
        {
            break;
        }

        output_index++;
    }

    return iByte;
}

int puts(const char * s)
{
    // Print to buffer
    DLOG_StrCpyToDebugOutputLogBuf(s);

    return 0;
}

#else

int puts(const char * s)
{
    // Print to serial
    char c;

    CHECK_DEBUG_BUF_INIT_STATUS();

    if (CPUINFO_GetLocalCpuId() == s_u8_dlogServerCpuId)
    {
        while (*s)
        {
            c = *s++;

            if (c == '\n')
            {
                uart_putc(DEBUG_LOG_UART_PORT, '\r');
            }
            
            uart_putc(DEBUG_LOG_UART_PORT, c);
        }
    }
    
    return 0;
}

unsigned int DLOG_Output(unsigned int byte_num)
{
    return 0;
}

#endif

