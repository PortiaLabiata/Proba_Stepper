#pragma once
#include "core/system.h"
#include "core/prefs.h"
#include "core/types.h"
#include "driver/stepper.h"

#include "mb.h"
#include "port.h"

/* Registers parameters */

#define REG_INPUT_START     1
#define REG_INPUT_NREGS     1
#define REG_HOLDING_START   1
#define REG_HOLDING_NREGS   4
#define COIL_START          1
#define COIL_NCOILS         1
#define DISCR_START         1
#define DISCR_NDISCR        1

/* Typedefs */

typedef struct MB_Proxy MB_Proxy_t; 

/* Callback prototypes */

eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs);
eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs,
    eMBRegisterMode eMode);
eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils,
    eMBRegisterMode eMode);
eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete);

MB_Proxy_t *MB_Proxy_Init(void);

System_Status_t MB_Proxy_Marshall(MB_Proxy_t *me);

System_Status_t MB_Proxy_Unmarshall(MB_Proxy_t *me);

Command_t MB_Proxy_AccessCmd(MB_Proxy_t *me);
Stepper_Mode_t MB_Proxy_AccessMode(MB_Proxy_t *me);
uint8_t MB_Proxy_AccessDir(MB_Proxy_t *me);
uint32_t MB_Proxy_AccessSpeed(MB_Proxy_t *me);
uint32_t MB_Proxy_AccessSteps(MB_Proxy_t *me);
System_Status_t MB_Proxy_AccessErr(MB_Proxy_t *me);

System_Status_t MB_Proxy_SetCmd(MB_Proxy_t *me, Command_t cmd);
