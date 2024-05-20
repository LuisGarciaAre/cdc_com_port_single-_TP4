// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms

#include <stdint.h>                   
#include <stdbool.h>

#include "app.h"
#include "MenuGen.h"
#include "GesPec12.h"
#include "Generateur.h"
#include "Mc32Debounce.h"
#include "Mc32DriverLcd.h"
#include "Mc32gestI2cSeeprom.h"

/*******************************************************
 * Nom de fonction: MENU_Initialize
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *  Sortie: -
 * 
 * Description: Initialisation du menu et des paramètres
 *******************************************************/
void MENU_Initialize(S_ParamGen *pParam)
{
    lcd_gotoxy(1,1);
    printf_lcd("TP4 UsbGen 23 - 24");
    lcd_gotoxy(1,2);
    printf_lcd("Garcia Luis");
    lcd_gotoxy(1,3);
    printf_lcd("Jeremy Affolter");
    GENSIG_UpdatePeriode(pParam);
    GENSIG_UpdateSignal(pParam);
}/*Fin de fonction MENU_Initialize*/


/********************************************************************************************************
 * Nom de fonction: Menu_Execute
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *  Sortie: -
 * 
 * Description: Fonction qui gere le menu de l'interface utilisateur, 
 *              machine d'etat avec 3 etat principales, 
 *              Etat USER_CHOICE : Gestion du choix du parametre a modifier via le Pec12 (etat principal)
 *              Etat PARAMS_MODIF: Gestion de modification du parametre choisi dans etat USER_CHOCE
 *              Etat SAVE_PARAMS:  Gestion de sauvegarde de valeurs parametrées dans la memoire
 *              La fonction s'apelle dans le APP_STATE_SERVICE_TASKS chaque 10 ms 
 ********************************************************************************************************/
void MENU_Execute(S_ParamGen *pParam, bool connectionIsRemote)
{
    // Variables locales
    static S_ParamGen tempParams;                   // Variable de sauvegarde temporelle des parametres de pParam
    static e_MENU_STATE stateMenu = USER_CHOICE;    // Variable de machine d'etat principale
    static uint8_t indexMenu = LIMIT_MIN_DISPLAY;                   // Variable d'index/curseur de menu
    uint8_t indexLigne;                             // Variable d'index de boucle for
    
    if(app_genData.flagUpdateAffichageLCD == true)
    {
        // Mise à jour d'affichage de valeurs du LCD
        UpdateDisplayValues(pParam, stateMenu, indexMenu, connectionIsRemote);
        app_genData.flagUpdateAffichageLCD = false;
    }
    
    // Machine d'etat principale d'etat du menu
    switch (stateMenu)
    {
        // Etat choix de parametre ou sauvegarde des valeurs en memoire
        case USER_CHOICE:
        { 
            if(connectionIsRemote == true)
            {                                        
                if(app_genData.flagAskToSave == true)
                { 
                    if(SaveParamsOnMemory(pParam))
                    {
                        app_genData.flagAskToSave = false;
                        app_genData.flagUpdateAffichageLCD = true;
                    }
                }
                
                if(app_genData.updateParams == true)
                {
                    Update_GENparams(pParam);
                    app_genData.updateParams = false;
                }
            }
            else
            {
                if(Pec12IsPlus() == true)
                {
                    Pec12ClearPlus();

                    // Si index/curseur > 4 reboucle pour revenir à 1
                    if(indexMenu < LIMIT_MAX_DISPLAY)
                    {
                        indexMenu += 1;
                        lcd_gotoxy(1,indexMenu);
                        printf_lcd("*");
                        lcd_gotoxy(1, (indexMenu - 1));
                        printf_lcd(" ");
                    }
                }

                if(Pec12IsMinus() == true)
                {
                    // Si index/curseur > 4 reboucle pour revenir à 0
                    Pec12ClearMinus();
                    if(indexMenu > LIMIT_MIN_DISPLAY)
                    {
                        indexMenu -= 1;
                        lcd_gotoxy(1,indexMenu); 
                        printf_lcd("*");
                        lcd_gotoxy(1, (indexMenu + 1));
                        printf_lcd(" ");
                    }
                }

                // Change vers etat PARAMS_MODIF et sauvegarde les valeurs actuels de pParams sur tempParams 
                if(Pec12IsOK() == true)
                {
                    Pec12ClearOK();
                    stateMenu = PARAMS_MODIF;
                    app_genData.flagUpdateAffichageLCD = true;
                    tempParams = *pParam;
                }
                                
                // Change ver etat SAVE_PARAMS, affichage selon CdC
                if(BtnS9IsESC() == true)
                {
                    BtnS9ClearESC();
                    for(indexLigne = LIMIT_MIN_DISPLAY; indexLigne <= LIMIT_MAX_DISPLAY ; indexLigne++)
                    {
                        lcd_ClearLine(indexLigne);
                    }

                    lcd_gotoxy(5, 2);
                    printf_lcd("Sauvegarde ?");
                    lcd_gotoxy(5, 3);
                    printf_lcd("(appui long)");
                    stateMenu = SAVE_PARAMS;
                }
            }
            break;
        }
        
        // Etat de modification des valeurs des parametres de pParam
        case PARAMS_MODIF:
        {
            // Appel de fonction MenuModifParams, appuie sur btn Pec12 (retourne true) pour revenir à l'état USER_CHOICE
            if(MenuModifParams(pParam, &tempParams, indexMenu, & app_genData.flagUpdateAffichageLCD) == true)
            {
                stateMenu = USER_CHOICE;
            }
            break;
        }
        
        // Etat de sauvegarde des valeurs parametrés dans memoire flash du uC
        case SAVE_PARAMS:
        {
            // Appel de fonction SaveParamsOnMemory, Appuie pendant plus que 500 ms sur btn S9 pour sauvegarder 
            // et revenir à l'état USER_CHOICE, autre action sur Pec12 ou appuie sur btn S0 pour ne pas sauvegarder
            if(SaveParamsOnMemory(pParam) == true)
            {
                stateMenu = USER_CHOICE;
                app_genData.flagUpdateAffichageLCD = true;
            }
            
            break;
        }
    }
}// Fin de fonction Menu_Execute


/********************************************************************************************************
 * Nom de fonction: UpdateDisplayValues
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *          stateMenu -> enum de machine d'etat principale pour indiquer l'etat pour maj d'affichage
 *          indexMenu -> entier non signé 1 byte pour indiquer le parametre à modifier l'affichage
 *  Sortie: -
 * 
 * Description: Fonction qui gere la mise à jour de l'affichage sur le LCD en fonction de l'etat du prog
 ********************************************************************************************************/
void UpdateDisplayValues(S_ParamGen *pParams, e_MENU_STATE stateMenu, uint8_t indexMenu, bool appRemote)
{
    // Variables locales
    // Tableau de constantes pour affichage de noms des parametres sur LCD
    const char* tabAffichageMenu[4] = {" Forme = ", " Freq [Hz] =   ", " Ampl [mV] =   ", " Offset [mV] = "};
        // Tableau de constantes pour affichage de noms des parametres sur LCD
    const char* tabAffichageMenuRemote[4] = {"#Forme = ", "#Freq [Hz] =   ", "#Ampl [mV] =   ", "#Offset [mV] = "};
    
    // Tableau de constantes pour affichage du parametre forme sur LCD
    const char* tabFormesString[4] = {"      Sinus", "   Triangle", " DentDeScie", "      Carre"};
    
    e_MENU_MODIF paramToModif = indexMenu;  // Variable de machine d'etat pour affichage de forme
    uint8_t indexLigne;                              // Variable d'index de boucle for

    // Mise à jour des parametres modifiés lors que la machine d'etat est sur l'etat USER_CHOICE
    if(stateMenu == USER_CHOICE)
    {
        for(indexLigne = LIMIT_MIN_DISPLAY; indexLigne <= LIMIT_MAX_DISPLAY ; indexLigne++)
        {
            lcd_ClearLine(indexLigne);
        }
        
        if(appRemote == true)
        {
            lcd_gotoxy(1,1);
            printf_lcd("%s%s", tabAffichageMenuRemote[0], tabFormesString[pParams->Forme]);

            lcd_gotoxy(1,2);
            printf_lcd("%s%d", tabAffichageMenuRemote[1], pParams->Frequence);

            lcd_gotoxy(1,3);
            printf_lcd("%s%d", tabAffichageMenuRemote[2], pParams->Amplitude);

            lcd_gotoxy(1,4);
            printf_lcd("%s%i", tabAffichageMenuRemote[3], pParams->Offset);
        }
        else
        {
            lcd_gotoxy(1,1);
            printf_lcd("%s%s", tabAffichageMenu[0], tabFormesString[pParams->Forme]);

            lcd_gotoxy(1,2);
            printf_lcd("%s%d", tabAffichageMenu[1], pParams->Frequence);

            lcd_gotoxy(1,3);
            printf_lcd("%s%d", tabAffichageMenu[2], pParams->Amplitude);

            lcd_gotoxy(1,4);
            printf_lcd("%s%i", tabAffichageMenu[3], pParams->Offset);
            lcd_gotoxy(1,indexMenu); 
            printf_lcd("*");
        }
    }
    // Affiche mise à jour en fonciton de param modifié lorsque prog n'est pas ni dans etat USER_CHOICE et SAVE_PARAMS
    else if((stateMenu != USER_CHOICE) && (stateMenu != SAVE_PARAMS))
    {
        lcd_ClearLine(indexMenu);
        lcd_gotoxy(1, indexMenu);

        switch (paramToModif)
        {
            case FORME:
            {
                printf_lcd("%s%s", tabAffichageMenu[0], tabFormesString[pParams->Forme]);
                break;
            }
            case FREQUENCE:
            {
                printf_lcd("%s%d", tabAffichageMenu[1], pParams->Frequence);
                break;
            }
            case AMPLITUDE:
            {
                printf_lcd("%s%d", tabAffichageMenu[2], pParams->Amplitude);
                break;
            }
            case OFFSET:
            {
                printf_lcd("%s%i", tabAffichageMenu[3], pParams->Offset);
                break;
            }
                
        }
        lcd_gotoxy(1,indexMenu); 
        printf_lcd("?");
    }
}/*Fin de fonction UpdateDisplayValues*/

/********************************************************************************************************
 * Nom de fonction: MenuModifParams
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *          tempParams -> enum de machine d'etat principale pour indiquer l'etat pour maj d'affichage
 *          paramToModif -> entier non signé 1 byte pour indiquer le parametre à modifier l'affichage
 *  Sortie: -
 * 
 * Description: Fonction qui gere la mise à jour de l'affichage sur le LCD en fonction de l'etat du prog
 ********************************************************************************************************/
bool MenuModifParams(S_ParamGen *pParams, S_ParamGen *tempParams, uint8_t indexMenu, bool* flagUpdateDisplay)
{
    // Variables locales
    bool flagEndModif = false;              // Variable flag qui indique la fin de la modification d'un parametre
    e_MENU_MODIF paramToModif = indexMenu;  // Variable machine d'etat de parametre à modifier
    
    // Machine d'etat de parametre à modifier
    // A chaque etat test si increment ou decrement sur Pec12 
    // en focntion du parametre choisi dans l'etat USER_CHOICE
    switch (paramToModif)
    {
        case FORME:
        {   
            if(Pec12IsPlus() == true)
            {
                if(pParams->Forme < Carre)
                {
                    pParams->Forme++;
                    *flagUpdateDisplay = true;
                }
                else
                {
                    pParams->Forme = Sinus;
                    *flagUpdateDisplay = true;
                }
                Pec12ClearPlus();
            }
            
            if(Pec12IsMinus() == true)
            {
                if(pParams->Forme > Sinus)
                {
                    pParams->Forme--;
                    *flagUpdateDisplay = true;
                }
                else
                {
                    pParams->Forme = Carre;
                    *flagUpdateDisplay = true;
                }
                Pec12ClearMinus();
            }            
            break;
        }
        case FREQUENCE:
        {            
            if(Pec12IsPlus() == true)
            {
                Pec12ClearPlus();
                if(pParams->Frequence < LIMIT_MAX_FREQ)
                {
                    pParams->Frequence += PAS_FREQ;
                }
                else
                {
                    pParams->Frequence = LIMIT_MIN_FREQ;
                }
                *flagUpdateDisplay = true;
            }
            
            if(Pec12IsMinus() == true)
            {
                Pec12ClearMinus();
                if(pParams->Frequence > LIMIT_MIN_FREQ)
                {
                    pParams->Frequence -= PAS_FREQ;
                }
                else
                {
                    pParams->Frequence = LIMIT_MAX_FREQ;
                }
                *flagUpdateDisplay = true;
            }
            break;
        }
        case AMPLITUDE:
        {            
            if(Pec12IsPlus() == true)
            {
                Pec12ClearPlus();
                if(pParams->Amplitude < LIMIT_MAX_AMPL)
                {
                    pParams->Amplitude += PAS_AMPL;
                }
                else
                {
                    pParams->Amplitude = LIMIT_MIN_AMPL;
                }
                *flagUpdateDisplay = true;
            }
            
            if(Pec12IsMinus() == true)
            {
                Pec12ClearMinus();
                
                if(pParams->Amplitude > LIMIT_MIN_AMPL)
                {
                    pParams->Amplitude -= PAS_AMPL;
                }
                else
                {
                    pParams->Amplitude = LIMIT_MAX_AMPL;
                }
                *flagUpdateDisplay = true;
            }
            
            break;
        }
        case OFFSET:
        {            
            if(Pec12IsPlus() == true)
            {
                Pec12ClearPlus();
                if(pParams->Offset < LIMIT_MAX_OFS)
                {
                    pParams->Offset += PAS_OFS;
                }
                *flagUpdateDisplay = true;
            }
            
            if(Pec12IsMinus() == true)
            {
                Pec12ClearMinus();
                if(pParams->Offset > LIMIT_MIN_OFS)
                {
                    pParams->Offset -= PAS_OFS;
                }
                *flagUpdateDisplay = true;
            }
            
            break;
        }
    }
    
    // Appuie sur btn Pec12 une fois indique fin de modification et updateSignal pour DAC 
    if(Pec12IsOK() == true)
    {
        Pec12ClearOK();
        Update_GENparams(pParams);
        *flagUpdateDisplay = true;
        flagEndModif = true;
    }
    
    // Appuie sur btn Pec12 pendant 500 ms indique fin de modification et restore valeurs precedentes
    if(Pec12IsESC() == true)
    {
        Pec12ClearESC();
        pParams->Forme = tempParams->Forme;
        pParams->Amplitude = tempParams->Amplitude;
        pParams->Offset = tempParams->Offset;
        pParams->Frequence = tempParams->Frequence;
        *flagUpdateDisplay = true;
        flagEndModif = true;
    }
    
    return flagEndModif;
}/*Fin de MenuModifParams*/


/*********************************************************************
 * Nom de fonction: SaveParamsOnMemory
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *  Sortie: bolean indiquant fin de sauvegarde en memoire
 * 
 * Description: Fonction qui gere la sauvegarde dans la memoire flash 
 *********************************************************************/
bool SaveParamsOnMemory(S_ParamGen *pParam)
{
    // Variables locales
    static uint8_t cntBackToMenu = 0;       // variable statique de compteur de 2s avant de revenir sur etat USER_CHOICE
    
    // Variable flag pour indiquer affichage choix de sauvegarde ou non params dans memoire
    // Variable flag pour indiquer fin de choix de sauvegarde
    static bool flagShowUserChoice = true;
    bool flagBackToMenu = false;
    
    // Sauvegarde des parametres en appuyant pendant 500 ms sur btn S9 ou 
    // Autre action annule la sauvegarde et revient sur etat USER_CHOICE
    if(Pec12IsMinus() || Pec12IsPlus() || Pec12IsOK() || BtnS9IsESC()) 
    {
        if(cntBackToMenu < TIMEOUT_DISPLAY_SAVE)
        {
            // Affiche une fois "Sauvegarde ANNULEE"
            if(flagShowUserChoice == true)
            {
                flagShowUserChoice = false;
                lcd_ClearLine(2);
                lcd_ClearLine(3);
                lcd_gotoxy(2,2);
                printf_lcd("Sauvegarde ANNULEE");
            }
            cntBackToMenu++;
        }
        else
        {
            Pec12ClearOK();
            Pec12ClearMinus();
            Pec12ClearPlus();
            BtnS9ClearESC();
            lcd_ClearLine(2);
            cntBackToMenu = 0;
            flagShowUserChoice = true;
            flagBackToMenu = true;
        }
    }
    else if(BtnS9IsSave() == true || app_genData.flagAskToSave == true)
    {
        if(cntBackToMenu < TIMEOUT_DISPLAY_SAVE)
        {
            // Affiche une fois "Sauvegarde OK!"
            if(flagShowUserChoice == true)
            {
                I2C_WriteSEEPROM((uint32_t*)pParam, 0x00, sizeof(S_ParamGen));
                flagShowUserChoice = false;
                lcd_ClearLine(1);
                lcd_ClearLine(2);
                lcd_ClearLine(3);
                lcd_ClearLine(4);
                lcd_gotoxy(4,2);
                printf_lcd("Sauvegarde OK!");
            }
            cntBackToMenu++;
        }
        else
        {
            BtnS9ClearSave();
            lcd_ClearLine(2);
            cntBackToMenu = 0;
            flagShowUserChoice = true;
            flagBackToMenu = true;
        }
    }
    
    return flagBackToMenu;
}/*Fin fonction SaveParamsOnMemory*/

void Update_GENparams(S_ParamGen *pParams)
{
    GENSIG_UpdateSignal(pParams);
    GENSIG_UpdatePeriode(pParams);
}
