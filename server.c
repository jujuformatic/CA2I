#include "libCA2I/comm.h"

#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <string.h>

void traiterSignal(int sig);
void installSignal(int sigNum, void (*handler)(int));
pthread_t create_thread_socket(pthread_t *thread, void *(*start_routine)(void *), socket_t socket);

void bye(void);
void dialogueClt(void *arg);

typedef struct{ 
    room_t *rooms[MAX_ROOMS];
    short roomNumber;
    sem_t sem;
} roomList_t;
void initRoomList(roomList_t *roomList);
int addRoom(roomList_t *roomList, room_t *room);
int popRoom(roomList_t *roomList, room_t *room);
int getRoom(roomList_t *roomList, char *hostName, room_t *room);

socket_t se, sd;
pthread_t client;
roomList_t *roomList;

int main()
{
    installSignal(SIGINT, traiterSignal); 
    atexit(bye);

    struct sockaddr_in svc, clt;
    socklen_t cltLen;

    se = creerSocketEcoute(INADDR_ANY_SOURCE, PORT_SVC);

    roomList = malloc(sizeof(roomList_t));
    initRoomList(roomList);
    
    while (1) {    
        // New player
        sd = acceptClt(se);

        create_thread_socket(&client, (void *)dialogueClt, sd);
        pthread_detach(client);
    }
    return 0;
}

void dialogueClt(void *arg) {
    socket_t *socket_ptr = (socket_t *)arg;
    socket_t sd = *socket_ptr; // Temporary socket
    free(socket_ptr);

    requete_t requete;
    room_t *room = malloc(sizeof(room_t));
    int i;
    int nbPlayers;
    question_t question;
    deck_t deck;

    // Début de la discussion : vérification de la demande du client (créer/rejoindre)

    recevoir(&sd, (generic)&requete, (pFct *)str2req);

    printf("Message reçu [%d:%s]\n", requete.id, requete.cont);

    switch (requete.id){
        case CREATE_ROOM:
            printf("Demande de création de salon\n");
            
            sscanf(requete.cont, "%s", room->host);
            strcpy(room->IP, inet_ntoa(sd.addrdist.sin_addr));
            
            addRoom(roomList, room);

            printf("Salon créé à l'adresse %s\n", room->IP);
            requete.id = OK;
            strcpy(requete.cont, "");
            envoyer(&sd, &requete, (pFct *)req2str);
            break;

        case JOIN_ROOM:
            printf("Demande pour rejoindre un salon\n");

            // Message d'erreur par défaut si le salon n'est pas trouvé
            requete.id = NOK;

            for (i=0; i<roomList->roomNumber; i++){
                if (strcmp(roomList->rooms[i]->host, requete.cont)==0 && roomList->rooms[i]->playerCount < MAX_PLAYERS && !roomList->rooms[i]->closed){
                    printf("Salon trouvé\n");
                    roomList->rooms[i]->playerCount++;
                    strcpy(requete.cont, roomList->rooms[i]->IP);
                    requete.id = OK;
                    break;
                }
            }
            envoyer(&sd, &requete, (pFct *)req2str);

            // Déconnection du joueur
            close(sd.fd);
            pthread_exit(NULL);
            break;
            
        default:
            break;        
    }

    // [2] Après avoir créé le salon et démarré la partie, le salon va demander au serveur les cartes
    recevoir(&sd, (generic)&requete, (pFct *)str2req);
    printf("Partie lancée par l'hôte\n");

    // On commence par fermer la connection au salon
    room->closed = 1;

    // Ensuite, on tire et envoie les cartes au salon
    envoyerDebutAuSalon(room, sd);
    printf("Lancement de la partie fini\n");

    //TODO: continuer communication salon

    pthread_exit(NULL);
}

void bye(){
    printf("Fermeture socket écoute\n");
    close(se.fd);
}

void traiterSignal(int sigNum) {
	switch (sigNum) {
		case SIGINT:
            exit(0); // Sortie par ^C
            break;
	}
}

void installSignal(int sigNum, void (*handler)(int)) {
	// Préparation du gestionnaire
	struct sigaction newAct;
	newAct.sa_handler = handler; // fonction à exécuter
	CHECK(sigemptyset(&newAct.sa_mask), "pb emptyset"); // vide le masque des signaux à bloquer pendant l'exécution du gestionnaire
	newAct.sa_flags = SA_RESTART; // relance les appels bloquants (wait, pause, ...) à la fin de la gestion

	// Activation du gestionnaire
	CHECK(sigaction(sigNum, &newAct, NULL), "pb sigaction");
}

/**
 * @brief Fonction qui créer un thread avec un numéro
 * @details La fonction créer un thread
 * @param thread destination de l'identifiant de thread
 * @param start_routine fonction principale du thread
 * @param socket socket de dialogue entre le thread et le client
 */
pthread_t create_thread_socket(pthread_t *thread, void *(*start_routine)(void *), socket_t socket) {
    pthread_attr_t attr;
    pthread_t tid;

    // Allouer dynamiquement le socket pour éviter les conflits
    socket_t *socket_ptr = malloc(sizeof(socket_t));
    if (socket_ptr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    *socket_ptr = socket;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Passer le pointeur du socket alloué au thread
    CHECK_T(tid = pthread_create(thread, &attr, start_routine, (void *) socket_ptr), "pthread_create");
    pthread_attr_destroy(&attr);

    return tid;
}

#pragma region RoomList
/**
 * @fn void initRoomList(roomList_t *roomList)
 * @brief initializes a roomList_t
 * Does not allocate memory
 */
void initRoomList(roomList_t *roomList){
    for (int i = 0; i < MAX_ROOMS; i++) {
        roomList->rooms[i] = NULL;  // Initialize room pointers to NULL
    }
    roomList->roomNumber = 0;
    CHECK_T(sem_init(&roomList->sem , 0, 1), "sem_open()");
}

/**
 * @fn int addRoom(roomList_t *roomList, room_t *room)
 * @brief appends a room into a roomList_t struct
 * @param roomList the initialized room list
 * @param room room_t initialized struct
 * @returns result of the operation
 * Returns 0 if the room was added, 1 if the room list is full
 * The list is protected by a semaphore => thread-compatible
 */
int addRoom(roomList_t *roomList, room_t *room){
    if(roomList->roomNumber == MAX_ROOMS) return 1;  // Can't accept any more rooms
        
    CHECK_T(sem_wait(&roomList->sem), "sem_wait()");
    int i = 0;
    
    roomList->rooms[roomList->roomNumber++] = room;

    CHECK_T(sem_post(&roomList->sem), "sem_post()");
    return 0;
}

/**
 * @fn int popRoom(roomList_t *roomList, room_t *room)
 * @brief removes a room from a roomList_t struct
 * @param roomList the initialized room list
 * @param room room_t initialized struct
 * @returns result of the operation
 * Returns 0 if the room was removed, 1 if the list if empty
 * The list is protected by a semaphore => thread-compatible
 */
int popRoom(roomList_t *roomList, room_t *room){
    if(roomList->roomNumber == 0) return 1; // No room in the list

    CHECK_T(sem_wait(&roomList->sem), "sem_wait()");
    int i = 0;
    
    while (strcmp(roomList->rooms[i]->host, room->host) != 0) i++;

    roomList->rooms[i] = roomList->rooms[roomList->roomNumber-1];
    roomList->roomNumber--;
    CHECK_T(sem_post(&roomList->sem), "sem_post()");
    return 0;
}

#pragma endregion