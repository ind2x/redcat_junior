#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT 300
#define CONSOLESHELL_PROMPTMESSAGE "redcat> "

typedef void (*CommandFunction)(const char *pcParameter);

#pragma pack(push, 1)

typedef struct ShellCommandEntryStruct
{
    char *pcCommand;
    char *pcHelp;
    CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

typedef struct ParameterListStruct
{
    const char *pcBuffer;
    int iLength;
    int iCurrentPosition;
} PARAMETERLIST;

#pragma pack( pop )

void StartConsoleShell(void);
void ExecuteCommand(const char *pcCommandBuffer);
void InitializeParameter(PARAMETERLIST *pstList, const char *pcParameter);
int GetNextParameter(PARAMETERLIST *pstList, char *pcParameter);

static void Help(const char *pcParameterBuffer);
static void Cls(const char *pcParameterBuffer);
static void ShowTotalRAMSize(const char *pcParameterBuffer);
static void StringToDecimalHexTest(const char *pcParameterBuffer);
static void ShutDownAndReboot(const char *pcParamegerBuffer);

static void SetTimer(const char *pcParameterBuffer);
static void WaitUsingPIT(const char *pcParameterBuffer);
static void ReadTimeStampCounter(const char *pcParameterBuffer);
static void MeasureProcessorSpeed(const char *pcParameterBuffer);
static void ShowDateAndTime(const char *pcParameterBuffer);

static void CreateTestTask(const char *pcParameterBuffer);

static void ChangeTaskPriority(const char *pcParameterBuffer);
static void ShowTaskList(const char *pcParameterBuffer);
static void KillTask(const char *pcParameterBuffer);
static void CPULoad(const char *pcParameterBuffer);

static void TestMutex(const char *pcParameterBuffer);

#endif /*__CONSOLESHELL_H__*/