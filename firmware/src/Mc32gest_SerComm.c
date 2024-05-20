// Mc32Gest_SerComm.C
// fonction d'émission et de réception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015


#include "app.h"
#include "Mc32gest_SerComm.h"
#include "DefMenuGen.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


// Fonction de reception  d'un  message
// Met à jour les paramètres du generateur a partir du message recu
// Format du message
//  !S=TF=2000A=10000O=+5000W=0#
//  !S=PF=2000A=10000O=-5000D=100W=1#
bool GetMessage(int8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo)
{
    uint8_t lenMsg = 0;
    bool askSave = false;
    char* indexForme;
    char* indexFrequence;
    char* indexAmplitude;
    char* indexOffset;
    char* indexSave;

    bool stateMsg = false;

    lenMsg = (uint8_t)strlen((char*)USBReadBuffer);

    if (lenMsg > APP_READ_BUFFER_SIZE)
    {
        stateMsg = false;
    }
    else if(USBReadBuffer[0] == '!' && USBReadBuffer[lenMsg - 1] == '#')
    {
        indexForme = strstr((char*)USBReadBuffer, "S");
        indexFrequence = strstr((char*)USBReadBuffer, "F");
        indexAmplitude = strstr((char*)USBReadBuffer, "A");
        indexOffset = strstr((char*)USBReadBuffer, "O");
        indexSave = strstr((char*)USBReadBuffer, "W");

        pParam->Forme = convertCharToForme(indexForme + 2);
        pParam->Frequence = atoi(indexFrequence + 2);
        pParam->Amplitude = atoi(indexAmplitude + 2);
        pParam->Offset = atoi(indexOffset + 2);
        askSave = atoi(indexSave + 2);
        
        if(askSave == true)
        {
            app_genData.flagAskToSave = true;
            *SaveTodo = true;
        }
        else
        {
            app_genData.flagAskToSave = false;
            *SaveTodo = false;
        }
        
        stateMsg = true;
    }
    
    return stateMsg;
} // GetMessage


// Fonction d'envoi d'un  message
// Rempli le tampon d'émission pour USB en fonction des paramètres du générateur
// Format du message
// !S=TF=2000A=10000O=+5000D=25WP=0#
// !S=TF=2000A=10000O=+5000D=25WP=1#    // ack sauvegarde

void SendMessage(int8_t *USBSendBuffer, S_ParamGen *pParam, bool Saved )
{
    char str[64] = {0};
    sprintf((char*)USBSendBuffer, "!S=%sF=%dA=%dO=%dWP=%d#", ConvertFormeToChar(pParam->Forme), pParam->Frequence, pParam->Amplitude, pParam->Offset, Saved);
    memcpy(str, (char*)USBSendBuffer, strlen((char*)USBSendBuffer));
} // SendMessage


E_FormesSignal convertCharToForme(char* valChar)
{
    E_FormesSignal forme = ERROR;

    switch (*valChar)
    {
        case 'S':
        {
            forme = Sinus;
            break;
        }
        case 'T':
        {
            forme = Triangle;
            break;
        }
        
        case 'C':
        {
            forme = Carre;
            break;
        }

        case 'D':
        {
            forme = DentDeScie;
            break;
        }
        default:
        {
            forme = ERROR;
            break;
        }
    }

    return forme;
}

char* ConvertFormeToChar(E_FormesSignal forme)
{
    char* charConvert;

    switch (forme)
    {
    case Sinus:
    {
        charConvert = "S";
        break;
    }
    case Triangle:
    {
        charConvert = "T";
        break;
    }

    case Carre:
    {
        charConvert = "C";
        break;
    }

    case DentDeScie:
    {
        charConvert = "D";
        break;
    }
    default:
    {
        charConvert = "E";
        break;
    }
    }

    return charConvert;
}

