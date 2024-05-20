// GesPec12.c  Canevas pour r�alisation  
// C. HUBER    09/02/2015

// Fonctions pour la gestion du Pec12
//
//
// Principe : Il est n�cessaire d'appeler cycliquement la fonction ScanPec12
//            avec un cycle de 1 ms
//
//  Pour la gestion du Pec12, il y a 9 fonctions � disposition :
//       Pec12IsPlus       true indique un nouveau incr�ment
//       Pec12IsMinus      true indique un nouveau d�cr�ment
//       Pec12IsOK         true indique action OK
//       Pec12IsESC        true indique action ESC
//       Pec12NoActivity   true indique abscence d'activit� sur PEC12
//  Fonctions pour quittance des indications
//       Pec12ClearPlus    annule indication d'incr�ment
//       Pec12ClearMinus   annule indication de d�cr�ment
//       Pec12ClearOK      annule indication action OK
//       Pec12ClearESC     annule indication action ESC
//
//
//---------------------------------------------------------------------------


// d�finitions des types qui seront utilis�s dans cette application

#include "GesPec12.h"
#include "./Mc32DriverLcd.h"
#include "Mc32Debounce.h"
#include "bsp.h"
#include "app.h"
#include "stdbool.h"

// Descripteur des sinaux
S_SwitchDescriptor DescrA;
S_SwitchDescriptor DescrB;
S_SwitchDescriptor DescrPB;
S_SwitchDescriptor DescrS9;

// Structure pour les traitement du Pec12
S_Pec12_Descriptor Pec12;
// Variables globales � utiliser
S_S9_Descriptor btnOk;



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Principe utilisation des fonctions
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//
// ScanPec12 (bool ValA, bool ValB, bool ValPB)
//              Routine effectuant la gestion du Pec12
//              recoit la valeur des signaux et du boutons
//
// s'appuie sur le descripteur global.
// Apr�s l'appel le descripteur est mis � jour

// Comportement du PEC12
// =====================

// Attention 1 cran g�n�re une pulse compl�te (les 4 combinaisons)
// D'ou traitement uniquement au flanc descendand de B

// Dans le sens horaire CW:
//     __________                      ________________
// B:            |____________________|
//     ___________________                       _________
// A:                     |_____________________|                    

// Dans le sens anti-horaire CCW:
//     ____________________                      _________
// B:                      |____________________|
//     __________                       __________________
// A:            |_____________________|        

/**********************************************************************************************
 * Nom de fonction: ScanPec12
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entr�e: valA : variable boolean avec signal A de Pec12
 *          valB : variable boolean avec signal B de Pec12
 *          valPB : variable boolean avec signal de btn du Pec12
 *  Sortie: -
 * 
 * Description: Lecture et gestion de Pec12 pour indication d'incrementation ou decrementation
 *              Gestion de btn Pec12, fonction s'appelle dans interruption timer 1 chaque 1ms
 **********************************************************************************************/
void ScanPec12 (bool ValA, bool ValB, bool ValPB)
{    
   // Traitement antirebond sur A, B et PB
   DoDebounce (&DescrA, ValA);
   DoDebounce (&DescrB, ValB);
   DoDebounce (&DescrPB, ValPB);
   
   // Detection d'increment ou decrement
   // Test un flanc descendant sur signal B ensuite test etat de signal A
   if(DebounceIsPressed(&DescrB) == true)
   {
       // Signal A = 0(etat bas) = increment
       // Signal A = 1(etat haut) = decrement
       if(DebounceGetInput(&DescrA) == 0)
       {
           // Incrementation de valeur
           Pec12.Inc = 1;
           Pec12.Dec = 0;
       }
       else
       {
           // Decrementation de valeur
           Pec12.Inc = 0;
           Pec12.Dec = 1;
       }
   }
   
   
   // Traitement du PushButton
   // Test de flanc descendant sur btn Pec12
   if(DebounceIsPressed(&DescrPB) == true)
   {
       // Increment de variable compteur de temps de bnt Pec12 appuy�
       Pec12.PressDuration++;
              
       // Lors de l'etat haut de btn Pec12 (btn relach�)
       // Si btn appuy� pendant plus que 500 ms
       if(DebounceGetInput(&DescrPB) == true)
       {
            DebounceClearPressed(&DescrPB);
            
           if(Pec12.PressDuration >= TIMEOUT_PRESS_DURATION)
           {
               Pec12.ESC = 1;
               Pec12.OK = 0;
               Pec12.PressDuration = 0;
           }
           else
           {
               Pec12.OK = 1;
               Pec12.ESC = 0;
               Pec12.PressDuration = 0;
           }
       }
   }
   
   // Gestion inactivite
   // Eteint backligth si pas d'action sur pec12 ou btn s9 pendant 5 secondes
   if((DebounceIsPressed(&DescrB) == false) && (DebounceIsPressed(&DescrPB) == false)
       && BtnS9IsESC() == false && BtnS9IsSave() == false && app_genData.flagUpdateAffichageLCD == false)
   {
       
       if(Pec12.InactivityDuration == TIMEOUT_INACTIVITY)
       {
           lcd_bl_off();
           Pec12.NoActivity = 1;
       }
       else
       {
           Pec12.InactivityDuration++;
       }

   }
   else
   {
       DebounceClearPressed(&DescrB);
       Pec12.InactivityDuration = 0;
       Pec12.NoActivity = 0;
       lcd_bl_on();
   }

 } // fin de fonction ScanPec12

/**********************************************************************************************
 * Nom de fonction: Pec12Init
 * Auteur: LGA & DBZ
 * Parametres:
 *  Entr�e: -
 *  Sortie: -
 * 
 * Description: Initialisation de antirebond pour pec12 en utilisant les fonction de lib Mc32Debounce
 **********************************************************************************************/
void Pec12Init (void)
{
   // Initialisation des descripteurs de touches Pec12
   DebounceInit(&DescrA);
   DebounceInit(&DescrB);
   DebounceInit(&DescrPB);
   
   // Init de la structure PEc12
    Pec12.Inc = 0;             // Indication incr�ment  
    Pec12.Dec = 0;             // Indication decrementation
    Pec12.OK = 0;              // Indication action OK
    Pec12.ESC = 0;             // Indication action ESC
    Pec12.NoActivity = 0;      // Indication d'activit�
    Pec12.PressDuration = 0;   // Pour dur�e pression du P.B.
    Pec12.InactivityDuration = 0; // Dur�e inactivit�
  
 } // Pec12Init end



/********************************************************************************************************
* Nom de fonction: btnOKInit
* Auteur: [DBS & LGA]
* Param�tres: Aucun
* Description: Cette fonction initialise le bouton S9 en configurant le m�canisme de d�bordement pour 
*              �liminer les rebonds. Elle initialise �galement les �tats ESC et SAVE du bouton ainsi 
*              que la dur�e de pression du bouton � 0.
********************************************************************************************************/
void btnOKInit(void)
{
    DebounceInit(&DescrS9);

    btnOk.ESC = 0;
    btnOk.SAVE = 0;
    btnOk.PressDuration = 0;
}


/********************************************************************************************************
* Nom de fonction: scan_btnOk
* Auteur: [DBS & LGA]
* Param�tres: Aucun
* Description: Cette fonction effectue la num�risation du bouton S9 pour d�tecter les pressions du 
*              bouton. Elle met � jour les �tats ESC et SAVE du bouton en fonction de la dur�e de 
*              pression d�tect�e.
********************************************************************************************************/
void scan_btnOk(void)
{
    // Traitement d'antirebons pour bouton S9
    DoDebounce(&DescrS9, S_OK);
    
    // Traitement du PushButton S9
    // Test un flanc descendant sur signal du bouton S9
    if(DebounceIsPressed(&DescrS9) == true)
    {
        // Increment variable de compteur de temps tant que le bouton reste appuy�
        btnOk.PressDuration++;
       
        // Lors de etat de bouton haut sur signal de bouton S9 (bouton relaxh�)
        // Si btn appuy� pendant plus que 500 ms
       if(DebounceGetInput(&DescrS9) == true)
       {
           DebounceClearPressed(&DescrS9);
           
           if(btnOk.PressDuration >= TIMEOUT_PRESS_DURATION)
            {
               btnOk.SAVE = 1;
               btnOk.ESC = 0;
               btnOk.PressDuration = 0;
            }
            else
            {
               btnOk.ESC = 1;
               btnOk.SAVE = 0;
               btnOk.PressDuration = 0;
            }
       }
    }
}// fin de fonction scan_btnOk

// Fonction de control pour bouton S9

// S9IsOK true indique action SAVE
bool BtnS9IsSave    (void) {
   return (btnOk.SAVE);
}

// S9IsESC true indique action ESC
bool BtnS9IsESC    (void) {
   return (btnOk.ESC);
}

// S9ClearOK annule indication action SAVE
void BtnS9ClearSave   (void) {
   btnOk.SAVE = 0;
}

// S9ClearESC annule indication action ESC
void BtnS9ClearESC   (void) {
   btnOk.ESC = 0;
}

// Fonction de control pour bouton Pec12

// Pec12IsPlus       true indique un nouveau incr�ment
bool Pec12IsPlus    (void) {
   return (Pec12.Inc);
}

// Pec12IsMinus      true indique un nouveau d�cr�ment
bool Pec12IsMinus    (void) {
   return (Pec12.Dec);
}

// Pec12IsOK         true indique action OK
bool Pec12IsOK    (void) {
   return (Pec12.OK);
}

// Pec12IsESC        true indique action ESC
bool Pec12IsESC    (void) {
   return (Pec12.ESC);
}

// Pec12NoActivity   true indique abscence d'activit� sur PEC12
bool Pec12NoActivity    (void) {
   return (Pec12.NoActivity);
}

//  Fonctions pour quittance des indications
// Pec12ClearPlus    annule indication d'incr�ment
void Pec12ClearPlus   (void) {
   Pec12.Inc = 0;
}

//       Pec12ClearMinus   annule indication de d�cr�ment
void Pec12ClearMinus   (void) {
   Pec12.Dec = 0;
}

//       Pec12ClearOK      annule indication action OK
void Pec12ClearOK   (void) {
   Pec12.OK = 0;
}

//       Pec12ClearESC     annule indication action ESC
void Pec12ClearESC   (void) {
   Pec12.ESC = 0;
}

void Pec12ClearInactivity   (void) {
  Pec12.NoActivity = 0;
  Pec12.InactivityDuration = 0;
}


