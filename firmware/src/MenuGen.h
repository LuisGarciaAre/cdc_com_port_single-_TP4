#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du générateur
// Traitement cyclique à 1 ms du Pec12


#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

#define TIMEOUT_DISPLAY_SAVE 199    // Define pour valeur timeout d'attente pour revenir sur etat USER_CHOICE
#define LIMIT_MAX_DISPLAY   4
#define LIMIT_MIN_DISPLAY   1

#define LIMIT_MAX_FREQ      2000
#define LIMIT_MIN_FREQ      20
#define PAS_FREQ            20

#define LIMIT_MAX_AMPL      10000
#define LIMIT_MIN_AMPL      0
#define PAS_AMPL            100

#define LIMIT_MAX_OFS       5000
#define LIMIT_MIN_OFS      -5000
#define PAS_OFS             100


/*******************************************************
 * Nom de fonction: MENU_Initialize
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *  Sortie: -
 * 
 * Description: Initialisation du menu et des paramètres
 *******************************************************/
void MENU_Initialize(S_ParamGen *pParam);


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
void MENU_Execute(S_ParamGen *pParam, bool stateConnexionUSB);


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
void UpdateDisplayValues(S_ParamGen *pParams, e_MENU_STATE stateMenu, uint8_t indexMenu, bool appRemote);


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
bool MenuModifParams(S_ParamGen *pParams, S_ParamGen *tempParams, uint8_t indexMenu, bool* flagUpdateDisplay);


/*********************************************************************
 * Nom de fonction: SaveParamsOnMemory
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entrée: pParam -> Structure de type S_ParamGen
 *  Sortie: bolean indiquant fin de sauvegarde en memoire
 * 
 * Description: Fonction qui gere la sauvegarde dans la memoire flash 
 *********************************************************************/
bool SaveParamsOnMemory(S_ParamGen *pParam);

#endif




  
   







