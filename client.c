#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "comm_modules/comm_7segments.h"
#include "comm_modules/comm_matriceLed.h"
#include "comm_modules/comm_matriceBtn.h"

#define NUM_THREAD_MAIN 1
#define NUM_THREAD_MINUTEUR 2
#define NUM_THREAD_LED 3
#define NUM_THREAD_BOUTONS 4

pthread_t thread_main;
pthread_t thread_minuteur;
pthread_t thread_led;
pthread_t thread_boutons;
volatile int stop_minuteur = 0; // Variable pour indiquer si le minuteur doit être arrêté
int points[8] = {0}; // Exemple de points à afficher
int buttonPressed = 0; // Variable pour stocker le bouton appuyé


void gestionnaireSIGUSR1(int signum);
void *lancerMinuteur(void *arg);
void *gererPoints(void *args);
void *gererBoutons(void *args);

int main() {
    initPins(); // Initialiser la matrice de boutons
    // Stocker l'identifiant du thread principal
    thread_main = pthread_self();

    // Configurer le gestionnaire pour SIGUSR1
    signal(SIGUSR1, gestionnaireSIGUSR1);


    // Créer le thread minuteur
    if (pthread_create(&thread_minuteur, NULL, lancerMinuteur, NULL) != 0) {
        perror("Erreur lors de la création du thread minuteur");
        return 1;
    }

    // Créer le thread LED
    if (pthread_create(&thread_led, NULL, gererPoints, NULL) != 0) {
        perror("Erreur lors de la création du thread LED");
        return 1;
    }

    // Créer le thread boutons
    if (pthread_create(&thread_boutons, NULL, gererBoutons, NULL) != 0) {
        perror("Erreur lors de la création du thread boutons");
        return 1;
    }

    /*// Simuler l'envoi d'un signal SIGUSR1 au thread minuteur après 10 secondes
    sleep(10);
    printf("Thread principal envoie un signal SIGUSR1 pour arrêter le minuteur.\n");
    pthread_kill(thread_minuteur, SIGUSR1);*/

    // Boucle principale du thread principal
    while (!stop_minuteur && !buttonPressed) {
    }

    // Attendre que les threads se terminent
    pthread_join(thread_minuteur, NULL);
    pthread_join(thread_led, NULL);
    pthread_join(thread_boutons, NULL);

    printf("Thread principal terminé\n");
    return 0;
}


void *lancerMinuteur(void *arg) {
    int i;
    wiringPiSetup();
    initHT16K33();

    printf("Thread minuteur (ID: %ld) démarré\n", pthread_self());
    
    for (i = 6000; i >= 0; i--) {
        if (stop_minuteur) {
            printf("Minuteur arrêté prématurément\n");
            pthread_exit(NULL);
        }
        displayNumber(i); // Afficher le temps restant
        usleep(7350); // Attendre 10 ms (7.35ms + temps de calcul)
    }

    printf("Minuteur terminé, envoi du signal SIGUSR1 au thread principal\n");
    if (pthread_kill(thread_main, SIGUSR1) != 0) {
        perror("Erreur lors de l'envoi du signal SIGUSR1");
    }
    pthread_exit(NULL);
}

void *gererPoints(void *args) {
    int i;
    initMatriceLed(); // Initialiser la matrice de LED

    printf("Thread LED (ID: %ld) démarré\n", pthread_self());

    while (!stop_minuteur) {
        for (i = 0; i < 8; i++) {
            afficherPoints(points);
        }
    }
    clearall(); // Effacer la matrice de LED
    printf("Thread LED terminé\n");
    pthread_exit(NULL);
}

void *gererBoutons(void *args) {
    

    printf("Thread boutons (ID: %ld) démarré\n", pthread_self());
    delay(100); // Attendre un peu pour éviter les faux positifs
    while (!stop_minuteur) {
        buttonPressed = detectButton(); // Détecter les boutons
        if (buttonPressed) {
            printf("Bouton %d appuyé\n", buttonPressed);
        }
        if(buttonPressed > 0) {
            // Si un bouton est appuyé, envoyer le signal SIGUSR1 au thread minuteur
            /*if (pthread_kill(thread_minuteur, SIGUSR1) != 0) {
                perror("Erreur lors de l'envoi du signal SIGUSR1");
            }*/
           points[buttonPressed - 1] ++; // Mettre à jour le tableau des points
        }
        buttonPressed = 0; // Réinitialiser l'état du bouton
        delay(100); // Petite pause pour éviter une surcharge CPU
    }

    
    printf("Thread boutons terminé\n");
    pthread_exit(NULL);
}


void gestionnaireSIGUSR1(int signum) {
    printf("Signal reçu : %d dans le thread ID: %ld\n", signum, pthread_self());
    if (pthread_self() == thread_main) {
        printf("Thread minuteur (ID: %ld) a envoyé le signal\n", pthread_self());
        stop_minuteur = 1; // Indique que le minuteur est arrêté
    } else if (pthread_self() == thread_minuteur) {
        printf("Thread principal (ID: %ld) a envoyé le signal SIGUSR1 pour arrêter le minuteur\n", pthread_self());
        stop_minuteur = 1; // Indiquer au minuteur de s'arrêter
    }
}