#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT 300
#define CONSOLESHELL_PROMPTMESSAGE "redcat>"

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

void Help(const char *pcParameterBuffer);
void Cls(const char *pcParameterBuffer);
void ShowTotalRAMSize(const char *pcParameterBuffer);
void StringToDecimalHexTest(const char *pcParameterBuffer);
void ShutDownAndReboot(const char *pcParamegerBuffer);

#endif /*__CONSOLESHELL_H__*/