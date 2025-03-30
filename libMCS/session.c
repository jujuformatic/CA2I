/**
 * @file session.c
 * @author Julien Martinez
 * @date 06/01/25
 * @brief Librairie de fonctions de session
 */
#include "session.h"

#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define MAX_REQ 5

/**
 * @fn adrToStruct
 * @brief Ajoute un port et une adresse à une adresse socket 
 * @param addr adresse socket (struct)
 * @param adrIP adresse IP 
 * @param port adresse de port
 */
void adrToStruct(struct sockaddr_in *addr, char *adrIP, short port){
    addr->sin_family = PF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(adrIP); //
    memset(addr->sin_zero, 0, 8);
}

/**
 * @fn creerSocket
 * @brief Créée une socket suivant un mode
 * @param mode Mode de la socket
 * @returns Socket de type socket_t avec un mode donné
 */
socket_t creerSocket(int mode){
    socket_t s;
    socklen_t addr_len = sizeof(s.addrloc);
    CHECK(s.fd=socket(PF_INET, mode, 0), "Can't create");
    s.mode=mode;

    return s;
}

/**
 * @fn creerSocketAdr
 * @brief Créée une socket suivant un mode et avec une addresse
 * @param mode Mode de la socket
 * @param adrIP Adresse IP de la socket
 * @param port Port de la socket
 * @returns Socket de type socket_t avec un mode et une adresse
 */
socket_t creerSocketAdr(int mode, char *adrIP, short port){
    socket_t s = creerSocket(mode);

    // Préparation de l’adressage du service
    adrToStruct(&s.addrloc, adrIP, port);
    
    // Association de l’adressage préparé avec la socket créée
    CHECK(bind(s.fd, (struct sockaddr *) &s.addrloc, sizeof(s.addrloc)), "Can't bind");

    return s;
}

/**
 * @fn creerSocketEcoute
 * @brief Créée une socket d'écoute avec une adresse IP
 * @param adrIP Adresse IP de la socket
 * @param port Port de la socket
 * @returns Socket d'écoute de type socket_t avec une adresse
 * @note La socket écoute depuis toutes les addresses
 */
socket_t creerSocketEcoute(char *adrIP, short port){
    socket_t se = creerSocketAdr(SOCK_STREAM, adrIP, port);

    // Mise en écoute de la socket
    CHECK(listen(se.fd, MAX_REQ) , "Can't calibrate");

    return se;
}

/**
 * @fn acceptClt
 * @brief Créée une socket de discussion vers un client
 * @param sockEcoute Socket d'écoute des nouveaux clients
 * @returns Socket de discussion de type socket_t 
 * @note La socket de discussion a la même adresse IP que la socket d'écoute mais un port différent
 */
socket_t acceptClt(const socket_t sockEcoute){
    socket_t sd;
    socklen_t cltLen = sizeof(sd.addrdist);

    sd.mode=SOCK_STREAM;
    
    // Attente d’un appel
    CHECK(sd.fd = accept(sockEcoute.fd, (struct sockaddr *)&sd.addrdist, &cltLen) , "Can't connect");
    
    return sd;
}

/**
 * @fn connectCltToSrv
 * @brief Créée une socket de discussion vers un serveur
 * @param adrIP Adresse du serveur
 * @param port Port de la socket d'écoute du serveur
 * @returns Socket de discussion de type socket_t 
 */
socket_t connectCltToSrv(char *adrIP, short port, int mode){
    socket_t s = creerSocket(mode);

    // Préparation de l’adressage du service à contacter
    adrToStruct(&s.addrdist, adrIP, port);
    
    // Demande d’une connexion au service
    CHECK(connect(s.fd, (struct sockaddr *)&s.addrdist, sizeof(s.addrdist)) , "Can't connect");

    // Récupération des informations locales après connexion
    socklen_t addr_len = sizeof(s.addrloc);
    CHECK(getsockname(s.fd, (struct sockaddr *)&s.addrloc, &addr_len), "getsockname() failed");

    return s;
}