/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/common/sys_common.h"
#include "app.h"
#include "app_gen.h"
#include "system_definitions.h"
#include "../../GesPec12.h"
#include "../../Generateur.h"

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************

 APP_GEN_DATA app_genData;

void __ISR(_TIMER_1_VECTOR, ipl3AUTO) IntHandlerDrvTmrInstance0(void)
{
    // Variables locales
    static uint16_t cntStart = 0;           // Variable 16 bits compteur de start
    static uint8_t cntCyclesProgram = 0;    // Variable 8 bits pour compteur de cycles de programme
    
    // Changement d'etat de led 1
    LED1_W = !LED1_R;
    
    // Realise traitement d'antirebonds pour PEC12
    ScanPec12(PEC12_A, PEC12_B, PEC12_PB);
    
    // Realise traitement d'antirebonds pour bouton S9
    scan_btnOk();
    
    // Attente de 3s 
    if(cntStart < VAL_WAIT_3S)
    {
        cntStart++;
    }
    else
    {
        // Realise le APP_STATE_SERVICE_TASKS chaque 10 cycles d'interruption (10 ms))
        if(cntCyclesProgram < VAL_WAIT_10ms)
        {
            cntCyclesProgram++;
        }
        else
        {
            cntCyclesProgram = 0;
            app_genData.state = APP_GEN_STATE_SERVICE_TASKS;
        }
    }
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
}


void __ISR(_TIMER_3_VECTOR, ipl7AUTO) IntHandlerDrvTmrInstance1(void)
{
    
    // Allume led 0
    LED0_W  = 1;
    
    // Execute fonction d'envoi sur DAC
    GENSIG_Execute();
    
    // Etetint led 0
    LED0_W = 0;
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_3);
}


 void __ISR(_USB_1_VECTOR, ipl4AUTO) _IntHandlerUSBInstance0(void)
{
    DRV_USBFS_Tasks_ISR(sysObj.drvUSBObject);
}

/*******************************************************************************
 End of File
*/