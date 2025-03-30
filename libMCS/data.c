/**
 * @file session.h
 * @author Julien Martinez
 * @date 06/01/25
 * @brief Librairie de fonctions de data
 */
#include <stdarg.h>
#include <arpa/inet.h>
#include <string.h>

#include "data.h"

/**
 * @struct buffer_t
 * @brief buffer contenant les données transférées
 */
typedef char buffer_t[MAX_BUFFER];

/**
 * @struct generic
 * @brief représente un pointeur vers n'importe quel type de données
 */
typedef void * generic;

/**
 * @struct pFct
 * @brief fonction prenant en entrée un pointeur deux types de données quelconques
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
void envoyer(socket_t *sockEch, generic quoi, pFct serial, ...){
    if(sockEch->mode==SOCK_STREAM){
        envoyerSTREAM(sockEch, quoi, serial);
    }
    else if(sockEch->mode==SOCK_DGRAM){
        char adrIP[16];
        short port;
        va_list pArg;
        va_start(pArg, serial);
        char *tempAdrIP = va_arg(pArg, char*);
        strcpy(adrIP, tempAdrIP);  
        port=(short)va_arg(pArg, int);
        envoyerDGRAM(sockEch, quoi, serial, adrIP, port);
    }
}

/**
 * @struct envoyerSTREAM
 * @brief fonction servant à envoyer des données à travers une socket en mode STREAM
 * @param sockEch socket d'échange
 * @param quoi données quelconques à envoyer
 * @param serial fonction de sérialisation
 */
void envoyerSTREAM(socket_t *sockEch, generic quoi, pFct serial){
    buffer_t buff;

    //Sérialiser les données
    serial(quoi, buff);

    //Envoyer les données sérialisées
    ssize_t bytes_written = write(sockEch->fd, buff, strlen(buff)+1);
    if (bytes_written != (ssize_t)(strlen(buff) + 1)) { perror("write"); exit(-1); }
}

/**
 * @struct envoyerDGRAM
 * @brief fonction servant à envoyer des données à travers une socket en mode DGRAM
 * @param sockEch socket d'échange
 * @param quoi données quelconques à envoyer
 * @param serial fonction de sérialisation
 * @param adrIP adresse IP 
 * @param port port de réception
 */
void envoyerDGRAM(socket_t *sockEch, generic quoi, pFct serial, char* adrIP, short port){
    buffer_t buff;

    adrToStruct(&sockEch->addrdist, adrIP, port);

    //Sérialiser les données
    serial(quoi, buff);

    //Envoyer les données sérialisées
    ssize_t bytes_sent = sendto(sockEch->fd, buff, strlen(buff)+1, 0, (struct sockaddr *)&sockEch->addrdist, sizeof(sockEch->addrdist));
    if (bytes_sent != (ssize_t)(strlen(buff) + 1)) { perror("sendto"); exit(-1); }
}

/**
 * @struct recevoir
 * @brief fonction servant à recevoir des données à travers une socket
 * @param sockEch socket d'échange
 * @param quoi données quelconques à recevoir (après désérialisation)
 * @param deSerial fonction de désérialisation
 */
void recevoir(socket_t *sockEch, generic quoi, pFct deSerial){
    buffer_t buff;

    //Vérifier la socket d'échange
    ssize_t bytes_received = read(sockEch->fd, buff, sizeof(buff) - 1);
    if (bytes_received < 0) { perror("read"); exit(-1); }
    buff[bytes_received] = '\0';  // S'assurer que la chaîne est bien terminée

    //Désérialiser les données reçues
    deSerial(buff, quoi);
}