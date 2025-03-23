/**
 * @file session.h
 * @author Julien Martinez
 * @date 06/01/25
 * @brief Librairie de fonctions de session
 */

#ifndef SESSION_H // Avoids multi-inclusion
#define SESSION_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CHECK(sts,msg) if ((sts) == -1) {perror(msg);exit(-1);}

/**
 * @struct socket_t
 * @brief Structure représentant une socket complète
 */
typedef struct {
    int fd;
    int mode;
    struct sockaddr_in addrloc; 
    struct sockaddr_in addrdist;
} socket_t;

/**
 * @fn adrToStruct
 * @brief Ajoute une adresse et un port à une socket
 * @param addr
 * @param adrIP
 * @param port
 */
void adrToStruct(struct sockaddr_in *addr, char *adrIP, short port);

/**
 * @fn creerSocket
 * @brief Créée une socket suivant un mode
 * @param mode Mode de la socket
 * @returns Socket de type socket_t avec un mode donné
 */
socket_t creerSocket(int mode);

/**
 * @fn creerSocketAdr
 * @brief Créée une socket suivant un mode et avec une addresse
 * @param mode Mode de la socket
 * @param adrIP Adresse IP de la socket
 * @param port Port de la socket
 * @returns Socket de type socket_t avec un mode et une adresse
 */
socket_t creerSocketAdr(int mode, char *adrIP, short port);

/**
 * @fn creerSocketEcoute
 * @brief Créée une socket d'écoute avec une adresse IP
 * @param adrIP Adresse IP de la socket
 * @param port Port de la socket
 * @returns Socket d'écoute de type socket_t avec une adresse
 */
socket_t creerSocketEcoute(char *adrIP, short port);

/**
 * @fn acceptClt
 * @brief Créée une socket de discussion vers un client
 * @param sockEcoute Socket d'écoute des nouveaux clients
 * @returns Socket de discussion de type socket_t 
 * 
 * @note La socket de discussion a la même adresse IP que la socket d'écoute mais un port différent
 */
socket_t acceptClt(const socket_t sockEcoute);

/**
 * @fn connectCltToSrv
 * @brief Créée une socket de discussion vers un serveur
 * @param adrIP Adresse du serveur
 * @param port Port de la socket d'écoute du serveur
 * @param mode Type de discussion
 * @returns Socket de discussion de type socket_t 
 */
socket_t connectCltToSrv(char *adrIP, short port, int mode);

#endif // SESSION_H