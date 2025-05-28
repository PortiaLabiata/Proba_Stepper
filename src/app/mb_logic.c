#include "app/mb_logic.h"

/* Typedefs */

struct MB_Proxy {
    Command_t cmd;
    uint32_t steps;
    Stepper_Mode_t mode;
    uint8_t dir;
    uint32_t del;
    System_Status_t err;
};

struct Holding {
    uint16_t cmd;
    uint16_t steps;
    uint16_t mode;
    uint16_t del;
};

/* Registers */

static USHORT usRegInputBuf[REG_INPUT_NREGS];
static USHORT usRegHoldingBuf[REG_HOLDING_NREGS];
static USHORT usCoilBuf[COIL_NCOILS];
static USHORT usDiscrBuf[DISCR_NDISCR];

/* MB Proxy object */

MB_Proxy_t _proxy;

/* MB Callbacks */

static eMBErrorCode eMBGenericRead(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, \
    USHORT usStart, USHORT usNumber, USHORT *usDataBuffer) {
   eMBErrorCode    eStatus = MB_ENOERR;
   int             iRegIndex = 0;

   if( ( usAddress >= usStart )
       && ( usAddress + usNRegs <= usStart + usNumber ) )
   {
       iRegIndex = ( int )( usAddress - usStart );
       while( usNRegs > 0 )
       {
           *pucRegBuffer++ =
               ( unsigned char )( usDataBuffer[iRegIndex] >> 8 );
           *pucRegBuffer++ =
               ( unsigned char )( usDataBuffer[iRegIndex] & 0xFF );
           iRegIndex++;
           usNRegs--;
       }
   }
   else
   {
       eStatus = MB_ENOREG;
   }

   return eStatus;
}

static eMBErrorCode eMBGenericWriteReg(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, \
   USHORT usStart, USHORT usNumber, USHORT *usDataBuffer) {
   eMBErrorCode eStatus = MB_ENOERR;

   if ((usAddress >= usStart) && (usAddress + usNRegs <= usStart + usNumber)) {
       int iRegIndex = (int)(usAddress - usStart);
       for (USHORT iIndex = 0; iRegIndex < usAddress - usStart + usNRegs; ) {
           USHORT usRegValue = pucRegBuffer[iIndex++] << 8;
           usRegValue += pucRegBuffer[iIndex++] & 0xFF;
           usDataBuffer[iRegIndex++] = usRegValue;
       }
   } else {
       eStatus = MB_ENOREG;
   }
   return eStatus;
}

eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
   eMBErrorCode eStatus = eMBGenericRead(pucRegBuffer, usAddress, usNRegs, \
       REG_INPUT_START, REG_INPUT_NREGS, usRegInputBuf);
   return eStatus;
}

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                eMBRegisterMode eMode) {
   /* First we have to make sure, that we reply */
   eMBErrorCode eStatus = MB_ENOERR;
   if (eMode == MB_REG_READ) {
       eStatus = eMBGenericRead(pucRegBuffer, usAddress, usNRegs, \
           REG_HOLDING_START, REG_HOLDING_NREGS, usRegHoldingBuf);
   } else if (eMode == MB_REG_WRITE) {
       eStatus = eMBGenericWriteReg(pucRegBuffer, usAddress, usNRegs, \
           REG_HOLDING_START, REG_HOLDING_NREGS, usRegHoldingBuf);
   }
   return eStatus;
}


eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils,
              eMBRegisterMode eMode) {
   eMBErrorCode eStatus = MB_ENOERR;
   if (eMode == MB_REG_READ) {
       eStatus = eMBGenericRead(pucRegBuffer, usAddress, usNCoils, \
           COIL_START, COIL_NCOILS, usCoilBuf);
   } else if (eMode == MB_REG_WRITE) {
       eStatus = eMBGenericWriteReg(pucRegBuffer, usAddress, usNCoils, \
           COIL_START, COIL_NCOILS, usCoilBuf);
   }
   return eStatus;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
   eMBErrorCode eStatus = eMBGenericRead(pucRegBuffer, usAddress, usNDiscrete, \
       DISCR_START, DISCR_NDISCR, usDiscrBuf);
   return eStatus;
}

/* MB Proxy functions */

/**
 * \brief Initializes MB Proxy object.
 * \returns Pointer to an object, there is only one of them.
 */
MB_Proxy_t *MB_Proxy_Init(void) {
    return &_proxy;
}

/**
 * \brief Loads abstracted values from proxy to registers.
 * \param[in] me Proxy object.
 * \returns Operation status.
 */
System_Status_t MB_Proxy_Marshall(MB_Proxy_t *me) {
    if (me == NULL) {
        return SYS_ERROR_NULLPTR;
    }
    usRegHoldingBuf[INDEX_HOLD_CMD] = me->cmd;
    usRegHoldingBuf[INDEX_HOLD_MODE] = ((me->mode) << 8) | (me->dir);
    usRegHoldingBuf[INDEX_HOLD_SPEED] = me->del;
    usRegHoldingBuf[INDEX_HOLD_STEPS] = me->steps;
    usRegInputBuf[INDEX_INP_ERRC] = me->err;
    return SYS_OK;
}

/**
 * \brief Abstracts register values.
 * \param[out] me Proxy object.
 * \returns Operation status.
 */
System_Status_t MB_Proxy_Unmarshall(MB_Proxy_t *me) {
    if (me == NULL) {
        return SYS_ERROR_NULLPTR;
    }
    me->cmd = (Command_t)usRegHoldingBuf[INDEX_HOLD_CMD];
    me->mode = (Stepper_Mode_t)usRegHoldingBuf[INDEX_HOLD_MODE] >> 8;
    me->dir = usRegHoldingBuf[INDEX_HOLD_MODE] & 0xFF;
    me->del = usRegHoldingBuf[INDEX_HOLD_SPEED];
    me->steps = usRegHoldingBuf[INDEX_HOLD_STEPS];
    me->err = (System_Status_t)usRegInputBuf[INDEX_INP_ERRC];
    return SYS_OK;
}

Command_t MB_Proxy_AccessCmd(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->cmd;
}

Stepper_Mode_t MB_Proxy_AccessMode(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->mode;
}

uint8_t MB_Proxy_AccessDir(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->dir;
}

uint32_t MB_Proxy_AccessSpeed(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->del;
}

uint32_t MB_Proxy_AccessSteps(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->steps;
}

System_Status_t MB_Proxy_AccessErr(MB_Proxy_t *me) {
    if (me == NULL) return CMD_INVALID;
    return me->err;
}

System_Status_t MB_Proxy_SetCmd(MB_Proxy_t *me, Command_t cmd) {
    if (cmd < CMD_NOCMD || cmd > CMD_INVALID) {
        return SYS_ERROR_ILLVALUE;
    }
    if (me == NULL) {
        return SYS_ERROR_NULLPTR;
    }
    me->cmd = cmd;
    return SYS_OK;
}