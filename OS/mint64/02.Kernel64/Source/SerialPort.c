#include "SerialPort.h"
#include "Utility.h"
#include "AssemblyUtility.h"

static SERIALMANAGER gs_stSerialManager;

void InitializeSerialPort(void)
{
    WORD wPortBaseAddress;

    InitializeMutex(&(gs_stSerialManager.stLock));

    wPortBaseAddress = SERIAL_PORT_COM1;

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_INTERRUPTENABLE, 0);

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_LINECONTROL, SERIAL_LINECONTROL_DLAB);

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_DIVISORLATCHLSB, SERIAL_DIVISORLATCH_115200);

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_DIVISORLATCHMSB, SERIAL_DIVISORLATCH_115200 >> 8);

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_LINECONTROL, SERIAL_LINECONTROL_8BIT | SERIAL_LINECONTROL_NOPARITY | SERIAL_LINECONTROL_1BITSTOP);

    OutPortByte(wPortBaseAddress + SERIAL_PORT_INDEX_FIFOCONTROL, SERIAL_FIFOCONTROL_FIFOENABLE | SERIAL_FIFOCONTROL_14BYTEFIFO);
}

static BOOL IsSerialTransmitterBufferEmpty(void)
{
    BYTE bData;

    bData = InPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINESTATUS);
    
    if((bData & SERIAL_LINESTATUS_TRANSMITBUFFEREMPTY) == SERIAL_LINESTATUS_TRANSMITBUFFEREMPTY)
    {
        return TRUE;
    }

    return FALSE;
}


void SendSerialData(BYTE *pbBuffer, int iSize)
{
    int iSentByte;
    int iTempSize;
    int j;

    Lock(&(gs_stSerialManager.stLock));

    iSentByte = 0;
    while(iSentByte < iSize)
    {
        while(IsSerialTransmitterBufferEmpty() == FALSE)
        {
            Sleep(0);
        }

        iTempSize = MIN(iSize - iSentByte, SERIAL_FIFOMAXSIZE);

        for(j=0; j<iTempSize; j++)
        {
            OutPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_TRANSMITBUFFER, pbBuffer[iSentByte + j]);
        }

        iSentByte += iTempSize;
    }

    Unlock(&(gs_stSerialManager.stLock));
}

static BOOL IsSerialReceiveBufferFull(void)
{
    BYTE bData;

    bData = InPortByte(SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_LINESTATUS);

    if((bData & SERIAL_LINESTATUS_RECEIVEDDATAREADY) == SERIAL_LINESTATUS_RECEIVEDDATAREADY)
    {
        return TRUE;
    }

    return FALSE;
}


int ReceiveSerialData(BYTE *pbBuffer, int iSize)
{
    int i;

    Lock( &( gs_stSerialManager.stLock ) );

     for( i = 0 ; i < iSize ; i++ )
    {
        if( IsSerialReceiveBufferFull() == FALSE )
        {
            break;
        }
        
        pbBuffer[i] = InPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_RECEIVEBUFFER );
    }
    
    Unlock( &( gs_stSerialManager.stLock ) );

    return i;
}

void ClearSerialFIFO(void)
{
    Lock( &( gs_stSerialManager.stLock ) );
    
    OutPortByte( SERIAL_PORT_COM1 + SERIAL_PORT_INDEX_FIFOCONTROL,  SERIAL_FIFOCONTROL_FIFOENABLE | SERIAL_FIFOCONTROL_14BYTEFIFO | SERIAL_FIFOCONTROL_CLEARRECEIVEFIFO | SERIAL_FIFOCONTROL_CLEARTRANSMITFIFO );
    
    Unlock( &( gs_stSerialManager.stLock ) );
}
