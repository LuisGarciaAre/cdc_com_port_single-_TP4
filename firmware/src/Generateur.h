#ifndef Generateur_h
#define Generateur_h

// TP3 MenuGen 2016
// C. HUBER  03.02.2016
// Fichier Generateur.h
// Prototypes des fonctions du g?n?rateur  de signal

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "DefMenuGen.h"

// Constantes ? utiliser dans fichier Generateur.c
#define MAX_ECH 100     // Define pour echantillons maximums pour valeur de recharge timer 3                             
#define Mi_Step 5000    // Define pour..
#define STEP    10000   // Define pour..
#define VALEUR_POUR_TRANSFORMATION_timer3 800000 // Define de valeurs de frequence Timer 3

/********************************************************************************************************
 * Nom de fonction: GENSIG_Initialize
 * Auteur: [DBS & LGA]
 * Param?tres: 
 *   Entr?e: pParam -> Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction initialise le g?n?rateur avec des param?tres par d?faut s'il n'y a pas de 
 *              donn?es sauvegard?es en m?moire, sinon elle initialise le g?n?rateur avec les donn?es 
 *              r?cup?r?es en m?moire.
 * 
 ********************************************************************************************************/
void  GENSIG_Initialize(S_ParamGen *pParam);


/********************************************************************************************************
 * Nom de fonction: GENSIG_UpdatePeriode
 * Auteur: [DBS & LGA]
 * Parametres: 
 *   Entree: pParam -> Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction met a jour la periode d'?chantillonnage du generateur en fonction de la 
 *              fr?quence specifiee dans les parametres.
 * 
 ********************************************************************************************************/
void  GENSIG_UpdatePeriode(S_ParamGen *pParam);


/********************************************************************************************************
 * Nom de fonction: GENSIG_UpdateSignal
 * Auteur: [DBS & LGA]
 * Parametres: 
 *   Entree: pParam -> Pointeur vers une structure de type S_ParamGen contenant les parametres du generateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction met a jour le signal genere en fonction des parametres specifies tels que 
 *              la forme d'onde, l'amplitude et le decalage (offset).
 * 
 ********************************************************************************************************/
void  GENSIG_UpdateSignal(S_ParamGen *pParam);

/********************************************************************************************************
 * Nom de fonction: GENSIG_Execute
 * Auteur: [DBS & LGA]
 * Parametres: 
 *   Entree: Aucun
 *   Sortie: Aucune
 * 
 * Description: Cette fonction execute le generateur en envoyant les valeurs du signal calcul? au DAC 
 *              pour generer une forme d'onde. Cette fonction est appelee dans l'interruption du timer3 
 *              avec un cycle variable.
 * 
 ********************************************************************************************************/
void  GENSIG_Execute(void);

#endif