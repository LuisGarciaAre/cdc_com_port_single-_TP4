
// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du generateur

// Prevu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber
/*
 * Modification du fichier 
 * Realis? par: Damien Bignens
 * Date: 19.03.2024
 * 
 * Ajout de modifications pour la gestion de signal de generateur de signal
 */

#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "system_config.h"
#include "Mc32DriverLcd.h"
#include "Mc32NVMUtil.h"
#include "Mc32gestI2cSeeprom.h"


//Variable tableaux de 32 biits sign?s de 100 valeurs
int32_t signal_calculer [MAX_ECH]; 

/********************************************************************************************************
 * Nom de fonction: GENSIG_Initialize
 * Auteur: [JAR & LGA]
 * Param?tres: 
 *   Entree: pParam -> Pointeur vers une structure de type S_ParamGen contenant les parametres du generateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction initialise le geneerateur avec des paramatres par defaut s'il n'y a pas de 
 *              donnees sauvegardees en memoire, sinon elle initialise le generateur avec les donnees 
 *              recuperees en memoire.
 * 
 ********************************************************************************************************/
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    S_ParamGen valeursParamGem = *pParam;

    // R?cup?ration des donn?es sauvegard?es au d?marrage pr?c?dent
    //NVM_ReadBlock((uint32_t*)&valeursParamGem, sizeof(S_ParamGen));  
    I2C_ReadSEEPROM(&valeursParamGem, 0x00, sizeof(S_ParamGen));
    
    // Si valeur lu dans memoire = donn?es OK
    if(valeursParamGem.Magic == MAGIC)
    {
        *pParam = valeursParamGem;
    }
    else 
    {
        // Initialisation des param?tres par d?faut
        pParam->Forme = Sinus;
        pParam->Amplitude = 10000;     
        pParam->Frequence = 1000;
        pParam->Offset = 0;
        pParam->Magic = MAGIC;
    }
}
  

/********************************************************************************************************
 * Nom de fonction: GENSIG_UpdatePeriode
 * Auteur: [DBS & LGA]
 * Param?tres: 
 *   Entr?e: pParam -> Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction met ? jour la p?riode d'?chantillonnage du g?n?rateur en fonction de la 
 *              fr?quence sp?cifi?e dans les param?tres.
 * 
 * Param?tres:
 *   - pParam: Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 * 
 ********************************************************************************************************/
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    static uint16_t val_du_conteur_Timer3 ;

    // Calcul de la p?riode d'?chantillonnage
    val_du_conteur_Timer3 = (((VALEUR_POUR_TRANSFORMATION_timer3 /pParam->Frequence)-1)+0.5 );
        
    // Mise ? jour de la p?riode du timer 3
    PLIB_TMR_Period16BitSet(TMR_ID_3, val_du_conteur_Timer3);
}/*Fin de fonction GENSIG_UpdatePeriode*/


/********************************************************************************************************
 * Nom de fonction: GENSIG_UpdateSignal
 * Auteur: [DBS & LGA]
 * Param?tres: 
 *   Entr?e: pParam -> Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 *   Sortie: Aucune
 * 
 * Description: Cette fonction met ? jour le signal g?n?r? en fonction des param?tres sp?cifi?s tels que 
 *              la forme d'onde, l'amplitude et le d?calage (offset).
 * 
 * Param?tres:
 *   - pParam: Pointeur vers une structure de type S_ParamGen contenant les param?tres du g?n?rateur
 * 
 * Remarque: Cette fonction utilise diff?rentes formules en fonction de la forme d'onde sp?cifi?e pour 
 *           calculer les valeurs du signal ? chaque ?chantillon. Les valeurs du signal sont ensuite 
 *           ajust?es  s'assurer qu'elles restent dans les limites d?finies et sont converties en 
 *           valeurs apppourropri?es pour le DAC.
 ********************************************************************************************************/
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    // Variables locales
    int16_t amplitude;  // Variable 16 bits non sing? pour amplitude
    int16_t Offset;     // Variable 16 bits non sing? pour offset
    uint8_t indexEchantillons;
    
    // Tableau de valeurs constants pour generation de forme sinus
    const int16_t sinus[100] = {
    3, 6, 9, 12, 15, 18, 21, 24, 27, 29,
    32, 34, 36, 39, 40, 42, 44, 45, 46, 48,
    48, 49, 50, 50, 50, 50, 50, 49, 48,
    48, 46, 45, 44, 42, 40, 39, 36, 34, 32,
    29, 27, 24, 21, 18, 15, 12, 9, 6, 3, 0,
    -3, -6, -9, -12, -15, -18, -21, -24, -27, -29,
    -32, -34, -36, -39, -40, -42, -44, -45, -46, -48,
    -48, -49, -50, -50, -50, -50, -49, -48, -48,
    -47, -46, -45, -44, -42, -40, -39, -36, -34, -32,
    -29, -27, -24, -21, -18, -15, -12, -9, -6, -3, 0 };
    
    amplitude = (pParam ->Amplitude / 100);
    Offset = pParam->Offset * -0.5; 

    
    for ( indexEchantillons = 0; indexEchantillons < MAX_ECH ; indexEchantillons++)
    {
        switch ( pParam ->Forme )
        {
            // Calcul des valeurs du signal pour une forme d'onde sinuso?dale
            case Sinus:
            {   
                signal_calculer [indexEchantillons] = Mi_Step + Offset + (sinus [indexEchantillons]) * amplitude   ;

            }
            break;
            case Triangle:
            {   
                // Calcul des valeurs du signal pour une forme d'onde triangulaire
                if ((MAX_ECH /2) > indexEchantillons)
                {
                    signal_calculer [indexEchantillons] = Mi_Step + Offset + (amplitude *(2 * (indexEchantillons -25) ))  ;
                    
                }else 
                {

                    signal_calculer [indexEchantillons] = Mi_Step +  Offset + (amplitude * (100 - 2 * (indexEchantillons-25) ) ) ;
                }
            }
            break;
            case DentDeScie:
            {
                // Calcul des valeurs du signal pour une forme d'onde en dent de scie
               signal_calculer [indexEchantillons] = Mi_Step + (Offset + (((indexEchantillons-50) * amplitude) ))  ;  
            }
            break;
            case Carre:
            { 
                // Calcul des valeurs du signal pour une forme d'onde carr?e
                if ((MAX_ECH /2) > indexEchantillons)
                {
                    signal_calculer [indexEchantillons] = (Mi_Step + ( (amplitude / 2 *MAX_ECH)+ Offset )) ;
                    
                }else 
                {

                    signal_calculer [indexEchantillons] = (Mi_Step - (  (amplitude / 2 *MAX_ECH ) - Offset)) ;
                }
            }
            break;
            
            default:
            {
            /* TODO: Handle error in application's state machine. */
            break;
            }
        }
        
        // Assure que les valeurs du signal restent dans les limites d?finies
        if (signal_calculer [indexEchantillons] > STEP) 
        {
            signal_calculer [indexEchantillons] = STEP;
        }
        if (signal_calculer [indexEchantillons] < 0)
        {
            signal_calculer [indexEchantillons] = 0;
        }
        
        // Convertit les valeurs du signal en valeurs appropri?es pour le DAC
        signal_calculer[indexEchantillons] = ((65535 * signal_calculer [indexEchantillons])/STEP);  
    } 
    
}/*Fin de fonction GENSIG_UpdateSignal*/


/********************************************************************************************************
 * Nom de fonction: GENSIG_Execute
 * Auteur: [DBS & LGA]
 * Param?tres: 
 *   Entr?e: Aucun
 *   Sortie: Aucune
 * 
 * Description: Cette fonction ex?cute le g?n?rateur en envoyant les valeurs du signal calcul? au DAC 
 *              pour g?n?rer une forme d'onde. Cette fonction est appel?e dans l'interruption du timer3 
 *              avec un cycle variable.
 * 
 ********************************************************************************************************/
void  GENSIG_Execute(void)
{
   static uint16_t EchNb = 0;
   // Envoie la valeur du signal calcul? au DAC sur le canal 0
   SPI_WriteToDac(0, signal_calculer [EchNb]);     
   
   // Incr?mente le num?ro d'?chantillon
   EchNb++;
   
   // R?initialise le num?ro d'?chantillon s'il d?passe le nombre maximum d'?chantillons
   EchNb = EchNb % MAX_ECH;
}/*Fin de fonction GENSIG_Execute*/