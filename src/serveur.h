#ifndef FTP_PROJECT_SERVEUR_H
#define FTP_PROJECT_SERVEUR_H

#include <iostream>

#include <stdint.h>
#include <vector>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mysocket.h"

#define SOCKLISTEN_NUMBER 10

namespace FTP_Server {

    //Détaille la structure d'un serveur
    class Server {

    private:

        /****** ATTRIBUTS PRIVEES DU SERVEUR ******/

        //Tableau des nom de fichiers disponibles
        std::vector<std::string> file_names;

        //Socket d'écoute
        MySocket::Socket_TCP sock_listen;

        //Liste des fichiers en cours de lecture
        std::vector<std::string> read_files;
        //Liste des fichiers en cours d'écriture
        std::vector<std::string> write_files;

        //Détermine si le serveur doit s'arrêter
        bool stop;

        //Liste des threads
        std::vector<pthread_t *> thread_list;

        //Les verrous de chaque variable protégée
        pthread_mutex_t mtx_stop;
        pthread_mutex_t mtx_file_names;
        //Variable conditionnelle pour la suppression d'entrées des listes "write" et "read"
        pthread_cond_t cond_file_names;


    public:

        /****** CONSTRUCTEURS DE LA CLASS ******/

        /**
         * Initialise le serveur avec un numéro de port aléatoire
        */
        Server();

        /**
         * Initialise le serveur
         * Paramètre:
         * - numéro de port (int)
        */
        Server(int);

        /****** METHODES PUBLIQUES DE LA CLASS ******/

        /**
         * Lance d'exécution du serveur
        */
        bool Start();

        /**
         * Arrète l'exécution du serveur
        */
        void Stop();

        /**
         * Détermine si le serveur doit s'arrèter (pour les threads)
        */
        bool NeedToStop();

        /**
         * Répond à la requête : Demander la liste des fichiers disponibles
         * Paramètre:
         * - socket de réponse (MySocket::Socket_TCP *)
        */
        void ReqList(MySocket::Socket_TCP *);

        /**
         * Répond à la requête : Demander l'existance d'un fichier
         * Paramètre:
         * - socket de réponse (MySocket::Socket_TCP *)
         * - nom du fichier (const char *)
        */
        void ReqExist(MySocket::Socket_TCP *, const char *);

        /**
         * Répond à la requête : Demander un fichier
         * Paramètre:
         * - socket de réponse (MySocket::Socket_TCP *)
         * - nom du fichier (const char *)
        */
        void ReqGet(MySocket::Socket_TCP *, const char *);

        /**
         * Répond à la requête : Demander d'envoyer un fichier
         * Paramètre:
         * - socket de réponse (MySocket::Socket_TCP *)
         * - nom du fichier (const char *)
        */
        void ReqPush(MySocket::Socket_TCP *, const char *);

        /**
         * Envoi un fichier dans la socket
         * Paramètre:
         * - socket (MySocket::Socket_TCP *)
         * - nom du fichier (const char *)
        */
        void SendFile(MySocket::Socket_TCP *, const char *);

        /**
         * Recoit un fichier depuis la socket
         * Paramètre:
         * - socket (MySocket::Socket_TCP *)
         * - nom du fichier (const char *)
        */
        void RecvFile(MySocket::Socket_TCP *, const char *);


    private:

        /****** METHODES PRIVEES DU SERVEUR ******/

        /**
         * Fait la liste des fichiers et les assigne
         * Paramètre:
         * - nom du répertoire (const char *)
        */
        bool __assignFileNames(const char *);

    };

    //Structure détaillant les paramètres reçu par le thread qui va traiter la requête
    typedef struct Request_Params {
        //Socket qui doit traiter la requête
        MySocket::Socket_TCP * req_sock;
        //Pointeur vers la class serveur appelante
        FTP_Server::Server * serv;
    } Request_Params;

    /**
     * Fonction du thread qui va traiter la requête
     * Paramètre:
     * - paramètres de la requête (FTP_Server::Request_Params *)
    */
    void * Request_Thread(void *);

}

#endif
