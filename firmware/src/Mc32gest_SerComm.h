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

// Enumeration des caract�res de rep�re
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

void SendMessage(int8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved);    // Fonction d'envoi d'un  message
bool GetMessage(int8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo); // Fonction de reception  d'un  message

E_FormesSignal convertCharToForme(char* valChar);   // Converti les lettres en formes
char* ConvertFormeToChar(E_FormesSignal forme); //Converti les formes en caract�re

#endif
