#ifndef FTP_MYSOCKET_H
#define FTP_MYSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

namespace MySocket {

    /****** CLASS ADRESS ******/

    //Contient les informations d'une adresse
    class Address {

    private:

        //Structure d'adressage
        struct sockaddr_in s_addr;

        //Chaine de caractère contenant l'adresse IP textuelle
        char s_ip[16];

        //Numéro de port de l'adresse
        int s_port;

        //Détermine si la création de l'adresse a réussi
        bool s_success;

    public:

        /**
         * Crée une adresse locale avec un numéro de port aléatoire
         */
        Address();

        /**
         * Crée une adresse locale
         * Paramètre:
         * - numéro de port (int)
         */
        Address(int);

        /**
         * Crée une adresse
         * Paramètres:
         * - adresse (const char *)
         * - numéro de port (int)
         */
        Address(const char *, int);

        /**
         * Crée une adresse à partir d'une structure d'adressage
         * Paramètre:
         * - structure d'adressage (const struct sockaddr_in *)
        */
        Address(const struct sockaddr_in *);

        /**
         * Détermine si la création de l'adresse a réussi
        */
        bool Success() const;

        /**
         * Retourne l'adresse IP
        */
        const char * GetIP() const;

        /*
         * Retourne le numéro de port
        */
        int GetPort() const;

        /**
         * Retourne un pointeur vers la structure d'adressage
        */
        const struct sockaddr_in * GetSockaddr() const;

        /**
         * Met à jour les attributs de l'adresse
        */
        void Refresh();

    private:

        //Retourne l'adresse ip formaté en fonction de l'adresse textuelle
        static in_addr_t _getipbyname(struct hostent *);

        //Remplie une chaine de caractère avec l'adresse ip correspondant
        static void _addrtoip(char *, const struct sockaddr_in *);

        //Retourne le numéro de port correspondant
        static int _addrtoportn(const struct sockaddr_in *);

    };


    /****** CLASS SOCKET_TCP ******/

    //Permet d'utiliser des sockets avec TCP
    class Socket_TCP {

    private:

        //Adresse de la cible (ou locale si c'est une BR publique)
        Address c_addr;

        //Descripteur de la socket
        int s_socket;

        //Détermine si la création de la socket a réussi
        bool s_success;

    public:

        /**
         * Crée une socket TCP pour serveur avec numéro de port aléatoire
        */
        Socket_TCP();

        /**
         * Crée une socket TCP pour serveur
         * Paramètre:
         * - numéro de port (int)
        */
        Socket_TCP(int);

        /**
         * Crée une socket TCP pour client
         * Paramètre:
         * - adresse (const char *)
         * - numéro de port (int)
        */
        Socket_TCP(const char *, int);


        /**
         * Créé une copie d'un socket
         * @param cpy Socket à copier
         */
        Socket_TCP(const Socket_TCP &cpy);

        /**
         * Destructeur du socket
         */
        virtual ~Socket_TCP();

        /**
         * Détermine si la création de la socket a réussi
        */
        bool Success();

        /**
         * Lance l'écoute du socket
         * Paramètre:
         * - longeur de la file (int)
         * Retourne vrai si réussi et faux sinon
        */
        bool Listen(int);

        /**
         * Accepte une demande de connexion
         * Retourne la socket associée
        */
        Socket_TCP Accept();

        /**
         * Retourne le destinataire de la socket
        */
        const Address& GetDestination() const;

        /**
         * Lit dans la socket
         * Paramètres:
         * - buffer (void *)
         * - taille du buffer (int)
         * Retourne le nombre de caractères lus
        */
        ssize_t Read(void *, size_t);

        /**
         * Ecrit dans la socket
         * Paramètres:
         * - données (void *)
         * - taille (int)
         * Retourne le nombre de caractères écrits
        */
        ssize_t Write(void *, size_t);

    private:

        //Initialise la class avec la socket et la cible
        Socket_TCP(int, struct sockaddr_in *);

        //Initialise la socket
        void _init_serv();

    };

}

#endif
