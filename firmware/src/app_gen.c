/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app_gen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "app_gen.h"
#include "Mc32DriverLcd.h"
#include "Mc32gestSpiDac.h"
#include "GesPec12.h"
#include "Generateur.h"
#include "MenuGen.h"
#include "DefMenuGen.h"
#include "Mc32gest_SerComm.h"
#include "Mc32gestI2cSeeprom.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_GEN_DATA app_genData;
S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;
bool saveParams = false;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/********************************************************************************************************
 * Nom de fonction: APP_GEN_Callback_TMR1
 * Auteur: [JAR & LGA]
 * Paramètres: 
 *   Entree: Aucune
 *   Sortie: Aucune
 * 
 * Description: Cette fonction est le callback de l'interruption du timer 1. Elle gère les actions 
 *              périodiques telles que le changement d'état d'une LED, le traitement antirebonds pour les 
 *              entrées PEC12 et le bouton S9, et le passage à l'état de service des tâches de l'application 
 *              après un délai de 10 ms.
 * 
 ********************************************************************************************************/
void APP_GEN_Callback_TMR1(void)
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
            // Indique qu'un changement de etat de machine d'etat principale
            app_genData.intTMR1 = true;
        }
    }
}

/********************************************************************************************************
 * Nom de fonction: APP_GEN_Callback_TMR3
 * Auteur: [JAR & LGA]
 * Paramètres: 
 *   Entree: Aucune
 *   Sortie: Aucune
 * 
 * Description: Cette fonction est le callback de l'interruption du timer TMR3. Elle allume une LED, 
 *              exécute une fonction d'envoi sur le DAC, puis éteint la LED.
 * 
 ********************************************************************************************************/
void APP_GEN_Callback_TMR3(void)
{
    // Allume led 0
    LED0_W  = 1;
    
    // Execute fonction d'envoi sur DAC
    GENSIG_Execute();
    
    // Etetint led 0
    LED0_W = 0;
}

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/********************************************************************************************************
 * Nom de fonction: APP_GEN_GETSET_STR
 * Auteur: [JAR & LGA]
 * Paramètres: 
 *   Entree: 
 *     - str : Pointeur vers la chaîne de caractères à traiter (uint8_t*)
 *     - tailleChaine : Taille de la chaîne de caractères (uint32_t)
 *   Sortie: Aucune
 * 
 * Description: Cette fonction copie une chaîne de caractères d'entrée dans un buffer local, puis utilise 
 *              cette chaîne pour mettre à jour des paramètres à distance. Si les paramètres sont mis à jour, 
 *              elle renvoie la chaîne modifiée et active un drapeau pour mettre à jour l'affichage LCD.
 * 
 ********************************************************************************************************/
void APP_GEN_GETSET_STR(uint8_t* str, uint32_t tailleChaine)
{
    char strHERE[APP_GEN_TAILLE_MAX_STR];
    
    if(tailleChaine > APP_GEN_TAILLE_MAX_STR)
    {
        tailleChaine = APP_GEN_TAILLE_MAX_STR;
    }
    
    memcpy(strHERE, str, tailleChaine);
    strHERE[tailleChaine] = '\0';

    app_genData.updateParams = GetMessage((int8_t*)strHERE, &RemoteParamGen, &saveParams);

    if(app_genData.updateParams == true)
    {
        SendMessage((int8_t*)strHERE, &RemoteParamGen, saveParams);
        memcpy(str, strHERE, strlen(strHERE));
        app_genData.flagUpdateAffichageLCD = true;
    }  
}

/********************************************************************************************************
 * Nom de fonction: All_leds
 * Auteur: [DBS & LGA]
 * Paramètres: 
 *   Entrée: state -> variable 8 bits non signé, État des LEDs (1 pour allumer, 0 pour éteindre)
 *   Sortie: Aucune
 * 
 * Description: Cette fonction contrôle l'état de toutes les LEDs sur la carte. Si l'état passé en 
 *              paramètre est 1, toutes les LEDs sont allumées. Sinon, si l'état est 0, toutes les 
 *              LEDs sont éteintes.
 * 
 * Remarque: Cette fonction utilise des macros BSP_LEDOn et BSP_LEDOff pour contrôler les LEDs.
 ********************************************************************************************************/
void All_ledsBSP(bool stateLeds)
{
    BSP_LED tabBSPLeds[8] = {   BSP_LED_0, BSP_LED_1, BSP_LED_2, BSP_LED_3, 
                                BSP_LED_4, BSP_LED_5, BSP_LED_6, BSP_LED_7};
    uint8_t numLed = 0;
    
    if(stateLeds == false)
    {
        for(numLed = 0; numLed < 8; numLed++)
        {
            BSP_LEDOff(tabBSPLeds[numLed]);
        }
    } 
    else
    {
        for(numLed = 0; numLed < 8; numLed++)
        {
            BSP_LEDOn(tabBSPLeds[numLed]);
        }
    }

}/*Fin de fonction All_leds*/

void APP_GEN_UPDATE_STATE_USB(bool state)
{
    app_genData.usbIsConnected = state;
    app_genData.flagUpdateAffichageLCD = true;
    app_genData.updateParams = true;
}


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_GEN_Initialize ( void )

  Remarks:
    See prototype in app_gen.h.
 */

void APP_GEN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_genData.state = APP_GEN_STATE_INIT;
    app_genData.intTMR1 = false;
    app_genData.flagAskToSave = false;
    app_genData.flagUpdateAffichageLCD = true;
    app_genData.updateParams = false;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_GEN_Tasks ( void )

  Remarks:
    See prototype in app_gen.h.
 */

void APP_GEN_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( app_genData.state )
    {
        /* Application's initial state. */
        case APP_GEN_STATE_INIT:
        {
                        // Initialisation de l'écran LCD
            lcd_init();
            
            // Allume le rétroéclairage de l'écran LCD
            lcd_bl_on();
            
            // Initialisation du BSP (Board Support Package)
            BSP_Initialize();

            // Init SPI DAC
            SPI_InitLTC2604();

            // Init I2C EEPROM
            I2C_InitMCP79411();
            
            // Initialisation PEC12
            Pec12Init();
            
            // Initialisation de bouton S9
            btnOKInit();
            
            // Test si USB est connnecté sur kit
            if(app_genData.usbIsConnected == true)
            {
                // Initialisation du generateur et menu en mode remote (usb connecté))
                GENSIG_Initialize(&RemoteParamGen);
                MENU_Initialize(&RemoteParamGen);
                
                // Synchronisation des parametres
                LocalParamGen = RemoteParamGen;
            }
            else
            {
                // Initialisation du generateur et mnue en mode local (usb non connecté))
                GENSIG_Initialize(&LocalParamGen);
                MENU_Initialize(&LocalParamGen);
                
                // Synchronisation des parametres
                RemoteParamGen = LocalParamGen;
            }
            
            // Eteint toutes les leds
            All_ledsBSP(false);
            
            // Active les timers 
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            
            // Changement d'etat de machine d'etat
            app_genData.state = APP_STATE_WAIT;
            break;
        }

        case APP_GEN_STATE_SERVICE_TASKS:
        {
            if(app_genData.usbIsConnected == true)
            {
                MENU_Execute(&RemoteParamGen, true);
            }
            else
            {
                MENU_Execute(&LocalParamGen, false);
            }

            app_genData.state = APP_STATE_WAIT;
            break;
        }        
        case APP_STATE_WAIT:
        {
            if(app_genData.intTMR1 == true)
            {
                app_genData.state = APP_GEN_STATE_SERVICE_TASKS;
                app_genData.intTMR1 = false;
            }
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

/*******************************************************************************
 End of File
 */
