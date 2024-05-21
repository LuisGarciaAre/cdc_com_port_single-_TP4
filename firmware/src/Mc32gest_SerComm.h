#ifndef Mc32Gest_SERCOMM_H
#define Mc32Gest_SERCOMM_H
/*--------------------------------------------------------*/
// Mc32Gest_SerComm.h
/*--------------------------------------------------------*/
//	Description :	emission et reception specialisee
//			pour TP4 2015-2016
//
//	Auteur 		: 	C. HUBER
// Modificateur :   LGA & JAR
//
//	Version		:	V1.2
//	Compilateur	:	XC32 V1.40 + Harmony 1.06
//
/*--------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "DefMenuGen.h"

// Enumeration des caractères de repère
typedef enum {
    I_STARTMSG = '!',
    I_FORME = 'S',
    I_FREQ = 'F',
    I_AMPL = 'A',
    I_OFFS = 'O',
    I_SAVE = 'W',
    I_ENDMSG = '#'
}S_indexMSG;

/*--------------------------------------------------------*/
// Prototypes des fonctions 
/*--------------------------------------------------------*/

/**********************************************************************************************
 * Nom de fonction: SendMessage
 * Auteur: LGA & JAR
 * Parametres:
 *  Entrée: poiteur: int8_t USBReadBuffer, S_ParamGen pParam, bool Saved
 *  Sortie: -
 * 
 * Description: Fonction d'envoi d'un  message
 * Rempli le tampon d'émission pour USB en fonction des paramètres du générateur
 * Format du message
 *  !S=TF=2000A=10000O=+5000WP=0#
 *  !S=TF=2000A=10000O=-5000WP=1#
 **********************************************************************************************/
void SendMessage(int8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved);    


/**********************************************************************************************
 * Nom de fonction: GetMessage
 * Auteur: LGA & JAR
 * Parametres:
 *  Entrée: poiteur: int8_t USBReadBuffer, S_ParamGen pParam, bool SaveTodo
 *  Sortie: -
 * 
 * Description: Fonction de reception  d'un  message
 * Met à jour les paramètres du generateur a partir du message recu
 * Format du message
 *  !S=TF=2000A=10000O=+5000W=0#
 *  !S=PF=2000A=10000O=-5000D=100W=1#
 **********************************************************************************************/
bool GetMessage(int8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo); 


/**********************************************************************************************
 * Nom de fonction: convertCharToForme
 * Auteur: LGA & JAR
 * Parametres:
 *  Entrée: pointeur: char valChar
 *  Sortie: E_FormesSignal
 * 
 * Description: Converti les lettres en formes
 **********************************************************************************************/
E_FormesSignal convertCharToForme(char* valChar);   



/**********************************************************************************************
 * Nom de fonction: ConvertFormeToChar
 * Auteur: LGA & JAR
 * Parametres:
 *  Entrée: E_FormesSignal forme
 *  Sortie: pointeur: char
 * 
 * Description: Converti les formes en caractère
 **********************************************************************************************/
char* ConvertFormeToChar(E_FormesSignal forme); 

#endif
