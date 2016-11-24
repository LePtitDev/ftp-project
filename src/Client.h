#pragma once

#include <string>
#include "mysocket.h"
#include "Config.h"

/**
 * \class Client
 * Cette classe va représenter un client connecté
 */
class Client {
public:
    /**
     * Constructeur d'un clien
     * @param c_socket Socket du client
     */
    Client(MySocket::Socket_TCP c_socket);

    /**
     * Méthode permettant de recevoir un message (met à jour aussi le status de connexion!)
     * @return Buffer de sortie
     */
    const byte * RecvMessage();

    /**
     * Méthode d'envoie de message de forme string (chaîne de caractère)
     * @param messageText Message texte à envoyer
     * @return Vrai si le message à bien été envoyé, sinon faux
     */
    bool SendMessage(std::string messageText);

    /**
     * Méthode d'envoie de message de type données
     * @param message Pointeur du tableau de données
     * @param msgSize Taille du tableau
     * @return Vrai si le message à bien été envoyé, sinon faux
     */
    bool SendMessage(byte *message, size_t msgSize);


    /**
     * Méthode qui retourne le socket du client
     * @return MySocket::Socket_TCP du client
     */
    const MySocket::Socket_TCP &GetSocket() const;

    /**
     * Méthode qui retourne l'adresse du client
     * @return MySocket::Adresse du client
     */
    const MySocket::Address &GetDestination() const;

    /**
     * Méthode qui retourne le buffer entrant (message reçu)
     * @return Pointeur sur le buffer entrant
     */
    const byte *GetBufferIn() const;

    /**
     * Méthode qui retourne le buffer sortant (message envoyé)
     * @return Pointeur sur le buffer sortant
     */
    const byte *GetBufferOut() const;


    /**
     * Méthode qui retourne le status du client, mise à jour par la méthode RecvMessage
     * @return Vrai si le client est toujours connecté, sinon faux
     */
    bool isConnected() const;

private:
    MySocket::Socket_TCP m_socket;
    ssize_t m_sizeBufferIn;
    byte m_bufferIn[BUFFER_SIZE];
    byte m_bufferOut[BUFFER_SIZE];
    bool m_isConnected;
};

