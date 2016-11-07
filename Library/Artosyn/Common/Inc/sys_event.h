#ifndef SYS_EVENT_H
#define SYS_EVENT_H

typedef void (*SYS_Event_Handler)(void *);

#define SYS_EVENT_HANDLER_PARAMETER_LENGTH    16
#define SYS_EVENT_LEVEL_HIGH_MASK             0x10000000
#define SYS_EVENT_LEVEL_MIDIUM_MASK           0x20000000
#define SYS_EVENT_LEVEL_LOW_MASK              0x40000000

// Registered system event handler list

typedef struct _RegisteredSysEventHandler_Node
{
    SYS_Event_Handler handler;
    struct _RegisteredSysEventHandler_Node* prev;
    struct _RegisteredSysEventHandler_Node* next;
} STRU_RegisteredSysEventHandler_Node;

typedef STRU_RegisteredSysEventHandler_Node* STRU_RegisteredSysEventHandler_List;

// Registered system event list

typedef struct _RegisteredSysEvent_Node
{
    uint32_t event_id;
    STRU_RegisteredSysEventHandler_List handler_list;
    STRU_RegisteredSysEventHandler_Node* handler_list_tail;
    struct _RegisteredSysEvent_Node* prev;
    struct _RegisteredSysEvent_Node* next;
} STRU_RegisteredSysEvent_Node;

typedef STRU_RegisteredSysEvent_Node* STRU_RegisteredSysEvent_List;

// Notified system event list

typedef struct _NotifiedSysEvent_Node
{
    uint32_t event_id;
    uint8_t parameter[SYS_EVENT_HANDLER_PARAMETER_LENGTH];
    struct _NotifiedSysEvent_Node* prev;
    struct _NotifiedSysEvent_Node* next;
} STRU_NotifiedSysEvent_Node;

typedef STRU_NotifiedSysEvent_Node* STRU_NotifiedSysEvent_List;

// Test event

typedef struct _SysEvent_TestParameter
{
    uint8_t  p1;
    uint16_t p2;
    uint32_t p3;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 7];
} STRU_SysEvent_TestParameter;

#define SYS_EVENT_ID_TEST (SYS_EVENT_LEVEL_HIGH_MASK | 0x8000)

// APIs

uint8_t SYS_EVENT_RegisterHandler(uint32_t event_id, SYS_Event_Handler event_handler);
uint8_t SYS_EVENT_UnRegisterHandler(uint32_t event_id, SYS_Event_Handler event_handler);
uint8_t SYS_EVENT_Notify(uint32_t event_id, void* parameter);
uint8_t SYS_EVENT_Process(void);
void SYS_EVENT_DumpAllListNodes(void);

// Idle event

#define SYS_EVENT_ID_IDLE (SYS_EVENT_LEVEL_LOW_MASK | 0x8000)

// Misc driver event

#define SYS_EVENT_ID_ADV7611_FORMAT_CHANGE          (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0001)
#define SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0002)

typedef struct _SysEvent_ADV7611FormatChangeParameter
{
    uint8_t  index;
    uint16_t width;
    uint16_t hight;
    uint8_t  framerate;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 4];
} STRU_SysEvent_ADV7611FormatChangeParameter;

typedef struct _SysEvent_BB_ModulationChange
{
    uint8_t  BB_MAX_support_br; //BB_MAX_support_br: the MAX stream bitrate(MHz) 
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 1];
} STRU_SysEvent_BB_ModulationChange;

#endif
