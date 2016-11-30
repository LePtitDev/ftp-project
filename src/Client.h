#pragma once

#include <string>
#include "mysocket.h"
#include "Config.h"

// Liste des commandes
const std::string CMD_LIST("LIST");
const std::string CMD_EXIST("EXIST");
const std::string CMD_GET("GET");
const std::string CMD_PUSH("PUSH");

/**
 * \class Client
 * Cette classe va représenter le client
 */
class Client {
public:
    /**
     * Constructeur du clien
     * @param c_socket Socket du client
     */
    Client();

    /**
     * Methode pour se connecter sur un serveur distant
     * @param addr_serv Adresse du serveur
     * @param port_serv Port du serveur
     * @return Vrai si la connection s'est bien déroulée, sinon faux
     */
    bool connectionServeur(std::string addr_serv, ushort port_serv);

    std::string GetListe();

    /**
     * Méthode permettant de recevoir la reponse du serveur
     * @return Buffer d'entré
     */
    const char * RecvResponse();


    /**
     * * Permet d'envoyer une commande à un serveur et d'attendre la réponse
     * @param commande Commande à envoyer
     * @param parametre Paramètre de la commande s'il y en a (vide par default)
     * @return Vrai si la commandeà bien été envoyée
     */
    bool SendCommande(std::string commande, std::string parametre = "");

    /**
     * Méthode d'envoie de message de type données
     * @param message Pointeur du tableau de données
     * @param msgSize Taille du tableau
     * @return Vrai si le message à bien été envoyé, sinon faux
     */
    bool SendMessage(const char *message, size_t msgSize);


    /**
     * Méthode qui retourne le socket du client
     * @return MySocket::Socket_TCP du client
     */
    const MySocket::Socket_TCP &GetSocket() const;

    /**
     * Méthode qui retourne le buffer entrant (message reçu)
     * @return Pointeur sur le buffer entrant
     */
    const char * GetBufferIn() const;

    /**
     * Méthode qui retourne le buffer sortant (message envoyé)
     * @return Pointeur sur le buffer sortant
     */
    const char * GetBufferOut() const;

    void CleanBuffers();


    /**
     * Méthode qui retourne le status du client, mise à jour par la méthode RecvMessage
     * @return Vrai si le client est toujours connecté, sinon faux
     */
    bool isConnected() const;

    void deconnexion();

private:
    MySocket::Socket_TCP m_socket, s_socket;
    ssize_t m_sizeBufferIn;
    char m_bufferIn[BUFFER_SIZE];
    char m_bufferOut[BUFFER_SIZE];
    bool m_isConnected;
};
