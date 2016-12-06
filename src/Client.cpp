#include <iostream>
#include "Client.h"

using namespace std;
using namespace MySocket;

Client::Client(): m_sizeBufferIn(0), m_isConnected(false) {
    // On vérifie que l'on a bien init notre socket
    if(m_socket.Success()) {

        // J'initialise mes buffers
        CleanBuffers();
    }
}

bool Client::connectionServeur(std::string addr_serv, ushort port_serv) {
    // On vérifie que l'on a bien un socket valide
    if(m_socket.Success()) {

        // On récupère et adapte l'adresse du serveur
        Address a(addr_serv.c_str(), port_serv);

        // On se Connect à notre serveur
        if(!m_socket.Connect(a)) {
            cerr << "Impossible de se connecter au serveur "<< addr_serv << ":" << port_serv << endl
                 << "\t> Le serveur distant a refusé la connexion (ou n'existe pas)." << endl;
            return false;
        }

        // Tout s'est bien passé, on retourne vrai
        m_isConnected = true;
        return true;

    }
    else {
        cerr << "Impossible de se connecter au serveur "<< addr_serv << ":" << port_serv << endl
             << "\t> Le socket du client n'est pas initialisé." << endl;
    }

    return false;
}

const char * Client::RecvResponse() {
    ssize_t size = m_socket.Read(m_bufferIn, BUFFER_SIZE);

    // Si l'on recoit une erreur, on informe de l'erreur et on passe en "deconnecté"
    if(size < 0)
    {
        perror("Une erreur est survenue lors de la reception d'un message client!");
        m_isConnected = false;
    }

        // Si l'on recoit 0, alors le client s'est deconnecté
    else if(size == 0)
        m_isConnected = false;

    return m_bufferIn;
}

bool Client::SendCommande(const string commande, std::string parametre) {
    string message(commande);
    if(!parametre.empty()) {
        message += " ";
        message += parametre;
    }

    if(!SendMessage(message.c_str(), message.size() + 1)) {
        cerr << "Le client n'a pas pu envoyer la commande: " << commande << "(" << parametre << ")" << endl;
    }
    else
        return true;

    return false;
}

bool Client::SendMessage(const char *message, size_t msgSize) {
    m_sizeBufferIn = m_socket.Write((void*)message, msgSize);
    if(m_sizeBufferIn < 0) perror("Impossible d'envoyer le message au client!");
    return m_sizeBufferIn >= 0;
}

const Socket_TCP &Client::GetSocket() const {
    return m_socket;
}

const char * Client::GetBufferIn() const {
    return m_bufferIn;
}

const char * Client::GetBufferOut() const {
    return m_bufferOut;
}

bool Client::isConnected() const {
    return m_isConnected;
}

void Client::CleanBuffers() {
    memset(m_bufferIn, 0, BUFFER_SIZE);
    memset(m_bufferOut, 0, BUFFER_SIZE);
}

string Client::GetListe() {

    string result, rep;
    char c = 2;
    while (c != '\0') {
        if (m_socket.Read(&c, 1) != 1)
            return "";
        else {
            if (c == ';') {
                result += "- " + rep + "\n";
                rep.clear();
            }
            else {
                rep += c;
            }
        }
    }

    /*string result(RecvResponse(), m_sizeBufferIn);

    while(m_sizeBufferIn == BUFFER_SIZE)
        result += string(RecvResponse(), m_sizeBufferIn);*/

    return result;
}

//__________________________________ MAIN __________________________________

const string MENU = "\nQue faire: \n \
\t> 1: Demander liste des fichiers disponibles (LIST) \n \
\t> 2: Demander si un fichier existe (EXIST <<file>>) \n \
\t> 3: Téléchargemennt d'un fichier (GET <<file>>) \n \
\t> 4: Envoie d'un fichier sur le serveur (PUSH <<file>>) \n \
\t> 5: Quitter \n \
Que voulez-vous faire: ";

int main(int argc, char** argv) {
    int choix = 0;

    cout << "Serveur de fichier: Client" << endl;

    // On a pas les arguments
    if(argc != 3) {
        cerr << "Le nombres d'arguments ne correspond pas!" << endl;
        cout << "Utilisation: client [adresseServeur] [portServeur]" << endl;
        return 1;
    }

    // On créé notre client
    Client client;


    // On le connect au serveur
    if(!client.connectionServeur(argv[1], (unsigned short) strtoul(argv[2], NULL, 0))) {
        return -1;
    }

    while(choix != 5) {

        // On affiche notre menu
        cout << MENU;
        cin >> choix;

        switch(choix) {
            case 1:
                if(client.isConnected())
                    client.SendCommande(CMD_LIST);
                cout << "\nListe des fichiers disponibles: \n" << client.GetListe();
                break;

            case 5:
                cout << "Au revoir!" << endl;
                break;

            default:
                cerr << "Ce choix n'existe pas ..." << endl << endl;
                choix = 0;
                break;
        }
    }

    return 0;
}
