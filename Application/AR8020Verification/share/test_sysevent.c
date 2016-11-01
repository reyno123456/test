#include <stdint.h>
#include "sys_event.h"
#include "test_sysevent.h"
#include "debuglog.h"

void test_IdleCallback(void * p)
{
    dlog_info("idle function ...");
}

void command_TestSysEventIdle(void)
{
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, test_IdleCallback);
}
