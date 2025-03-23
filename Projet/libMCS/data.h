/**
 * @file session.h
 * @author Julien Martinez
 * @date 06/01/25
 * @brief Librairie de fonctions de data
 */

#include "session.h"

#define MAX_BUFFER 1024 

/**
 * @struct buffer_t
 * @brief buffer contenant les données transférées
 */
typedef char buffer_t[MAX_BUFFER];

/**
 * @struct generic
 * @brief représente n'importe quel type de données
 */
typedef void * generic;

/**
 * @struct pFct
 * @brief fonction prend en entrée deux types de données quelconques
 * @returns type de données quelconque
 */
typedef void * pFct(generic, generic);

/**
 * @struct envoyer
 * @brief fonction servant à envoyer des données à travers une socket
 * @param sockEch socket d'échange
 * @param quoi données quelconques à envoyer
 * @param serial fonction de sérialisation
 * @param ... adresse IP et port de réception (mode DGRAM)
 */
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...);

/**
 * @struct envoyerSTREAM
 * @brief fonction servant à envoyer des données à travers une socket en mode STREAM
 * @param sockEch socket d'échange
 * @param quoi données quelconques à envoyer
 * @param serial fonction de sérialisation
 */
void envoyerSTREAM(socket_t *sockEch, generic quoi, pFct serial);

/**
 * @struct envoyerDGRAM
 * @brief fonction servant à envoyer des données à travers une socket en mode DGRAM
 * @param sockEch socket d'échange
 * @param quoi données quelconques à envoyer
 * @param serial fonction de sérialisation
 * @param adrIP adresse IP 
 * @param port port de réception
 */
void envoyerDGRAM(socket_t *sockEch, generic quoi, pFct serial, char* adrIP, short port);

/**
 * @struct recevoir
 * @brief fonction servant à recevoir des données à travers une socket
 * @param sockEch socket d'échange
 * @param quoi données quelconques à recevoir (après désérialisation)
 * @param deSerial fonction de désérialisation
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial);
