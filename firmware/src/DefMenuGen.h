// Ecole supérieure SL229_MINF TP
// Manipulation Tp3_MenuGen
// Créé le 9 Mai 2006 CHR
// Version 2016  du 03.02.2016
// Modif 2015 : utilisation de stdint.h
// Modif 2016 : ajout Duty pour PWM
// Modif 2018 SCA : suppression PWM et duty
// Definition pour le menuGen

#ifndef DEFMENUGEN_H
#define DEFMENUGEN_H

#include <stdint.h>

#define MAGIC 0x123455AA

typedef enum  { Sinus, Triangle, DentDeScie, Carre, ERROR } E_FormesSignal;

// Structure des paramètres du générateur
typedef struct {
      E_FormesSignal Forme;
      int16_t Frequence;
      int16_t Amplitude;
      int16_t Offset;
      uint32_t Magic;
} S_ParamGen;

// Enum des etats pour machine d'etat principale
typedef enum
{
    USER_CHOICE  =   0,
    PARAMS_MODIF  =   1,
    SAVE_PARAMS =   2,
} e_MENU_STATE;

// Enum des etat pour machine d'etat de parametre
typedef enum
{
    FORME       =   1,
    FREQUENCE   =   2,
    AMPLITUDE   =   3,
    OFFSET      =   4,
} e_MENU_MODIF;


#endif
