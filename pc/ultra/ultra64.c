//#include <os_internal.h>
//#include "osint.h"
#include "ultra64.h"
//#include "bss.h"
#include <math.h>
//#include <stdio.h>

extern struct __osThreadTail __osThreadTail;
//u8 g_Is4Mb = 0;

// Msg queues
void osCreateMesgQueue(OSMesgQueue *mq, OSMesg *msg, s32 msgCount)
{
	//mq->mtqueue = (OSThread *)&__osThreadTail;
	//mq->fullqueue = (OSThread *)&__osThreadTail;
	mq->validCount = 0;
	mq->first = 0;
	mq->msgCount = msgCount;
	mq->msg = msg;
}

u32 dummyMsgBuffer[128] = {0};
s32 osRecvMesg(OSMesgQueue *mq, OSMesg *msg, s32 flags)
{
	// http://ultra64.ca/files/documentation/online-manuals/man/n64man/os/osRecvMesg.html
	// TODO: implement message queues, scheduler etc...
	if (msg != NULL)
		*msg = dummyMsgBuffer;

	return -1;
}

void osSetEventMesg(OSEvent event, OSMesgQueue *mq, OSMesg msg)
{
}

s32 osSendMesg(OSMesgQueue *mq, OSMesg msg, s32 flags)
{
	return 0;
}

s32 osContInit(OSMesgQueue *mq, u8 *bitpattern, OSContStatus *data)
{
	return 0;
}

s32 osContStartQuery(OSMesgQueue *mq)
{
	return 0;
}

void osContGetQuery(OSContStatus *data)
{
}

void osCreateThread(OSThread *, OSId, void (*)(void *), void *, void *, OSPri)
{
	printf("stub: osCreateThread()\n");
}

void osStartThread(OSThread *)
{

}

OSThread * __osGetCurrFaultedThread(void)
{
	return 0;
}

OSIntMask osSetIntMask(OSIntMask)
{
	return 0;
}

// Timers
int osSetTimer(OSTimer *t, OSTime value, OSTime interval, OSMesgQueue *mq, OSMesg msg)
{
	return 0;
}

void osViBlack(u8)
{

}

u32	osGetCount(void)
{
	return 0;
}

float sinf(float angle)
{
	return (float)sin((double)angle);
}

float cosf(float angle)
{
	return (float)cos((double)angle);
}

float sqrtf(float value)
{
	return (float)sqrt((double)value);
}

void osInvalDCache(void *, s32)
{

}

u64 osVirtualToPhysical(void* ptr)
{
	return (u64)ptr;
}

void osWritebackDCacheAll(void)
{

}

void *rzipGetSomething(void)
{
	/*
		glabel rzipGetSomething
			lui    $v0, %hi(var80091558)
			addiu  $v0, $v0, %lo(var80091558)
			jr     $ra
			lw     $v0, 0x0($v0)
	*/

	

}

// Audio stubs

u32 osAiGetLength(void)
{
	return 0;
}

void osCreatePiManager(OSPri pri, OSMesgQueue *cmdQ, OSMesg *cmdBuf, s32 cmdMsgCnt)
{

}

void osDpGetCounters(u32 *array)
{

}

u32 osDpGetStatus()
{
	return 0;
}

void osDpSetStatus(u32 data)
{

}

s32 osEepromLongRead(OSMesgQueue *mq, u8 address, u8 *buffer, int length)
{

}

s32 osEepromLongWrite(OSMesgQueue *mq, u8 address, u8 *buffer, int length)
{

}

s32 osEepromProbe(OSMesgQueue *mq)
{

}

s32 osGbpakInit(OSMesgQueue *queue, OSPfs *pfs, int channel)
{

}

s32 osGbpakPower(OSPfs *pfs, s32 flag)
{

}

s32 osGbpakReadId(OSPfs *pfs, OSGbpakId *id, u8 *status)
{

}

s32 osGbpakReadWrite(OSPfs *pfs, u16 flag, u16 address, u8 *buffer, u16 size)
{

}

OSPri osGetThreadPri(OSThread *thread)
{
	return 0;
}

OSTime osGetTime()
{
	return 0;
}

s32 osMotorProbe(OSMesgQueue* ctrlrqueue, OSPfs* pfs, s32 channel)
{

}

s32 osPfsAllocateFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int num_bytes, s32 *file_no)
{

}

s32 osPfsChecker(OSPfs *pfs)
{

}

s32 osPfsDeleteFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name)
{

}

s32 osPfsFileState(OSPfs *pfs, s32 fileNo, OSPfsState *state)
{

}

s32 osPfsFindFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, s32 *file_no)
{

}

s32 osPfsFreeBlocks(OSPfs *pfs, s32 *remaining)
{

}

s32 osPfsInitPak(OSMesgQueue *queue, OSPfs *pfs, s32 channel, s32 *arg3)
{

}

s32 osPfsNumFiles(OSPfs *pfs, s32 *max_files, s32 *files_used)
{

}

s32 osPfsReSizeFile(OSPfs *pfs, u16 company_code, u32 game_code, u8 *game_name, u8 *ext_name, int length)
{

}

s32 osPfsReadWriteFile(OSPfs* pfs, s32 fileNo, u8 flag, int offset, int size, u8* data)
{

}

s32 osPiStartDma(OSIoMesg *mb, s32 priority, s32 direction, u32 devAddr, void *dramAddr, u32 size, OSMesgQueue *mq)
{

}

s32 osResetType = 0x8000030c;

void osSetThreadPri(OSThread *t, OSPri pri)
{

}

void osSpTaskLoad(OSTask *intp)
{

}

void osSpTaskStartGo(OSTask *tp)
{

}

void osSpTaskYield(void)
{

}

OSYieldResult osSpTaskYielded(OSTask *tp)
{

}

void osStopThread(OSThread *t)
{

}

int osStopTimer(OSTimer *t)
{

}

void *osViGetCurrentFramebuffer(void)
{

}

void *osViGetNextFramebuffer(void)
{

}

void osViSetMode(OSViMode *modep)
{

}

void osViSetSpecialFeatures(u32 func)
{

}

void osViSetXScale(f32 value)
{

}

void osViSetYScale(f32 value)
{

}

void osViSwapBuffer(void *frameBufPtr)
{

}



