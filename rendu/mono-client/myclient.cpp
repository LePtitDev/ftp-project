#include <iostream>
#include <string>
#include <vector>

#include <stdint.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "mysocket.h"

//Taille maximale acceptée d'une commande sur le terminal
#define COMMANDE_MAXSIZE 256

//Liste des commande sous forme de chaine de caractères
const char * help_string = "\
Liste des commandes :\n\
- LIST : affiche la liste des fichiers du serveur\n\
- EXIST <nom du fichier> : determine si le fichier existe\n\
- GET <nom du fichier> [...] : telecharge le(s) fichier(s) depuis le serveur s'il existe\n\
- PUSH <nom du fichier> [...] : envoie le(s) fichier(s) vers le serveur\n\
- HELP : affiche les commandes\n\
- EXIT : arrete la connexion";

/**
 * Envoi une requête "LIST" au serveur et attend la réponse
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool Request_List(MySocket::Socket_TCP& socket) {
    if (socket.Write("LIST", 5) != 5)
        return false;

    std::string result = "Liste des fichiers :\n", rep;
    char c = 1;
    while (c != '\0') {
        if (socket.Read(&c, 1) != 1)
            return false;
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

    std::cout << result;

    return true;
}

/**
 * Envoi une requête "EXIST" au serveur et attend la réponse
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * - nom du fichier (char *)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool Request_Exist(MySocket::Socket_TCP& socket, char * file_name) {
    std::string name;
    while (*file_name == ' ') file_name++;
    while (*file_name != ' ' && *file_name != '\0') {
        name += *file_name;
        file_name++;
    }

    if (name == "") {
        std::cout << "ATTENTION : Vous devez specifier un nom de fichier pour la requete \"EXIST\", tapez \"HELP\" pour afficher les commandes disponibles" << std::endl;
        return true;
    }

    std::string req = "EXIST " + name, rep;
    if (socket.Write(req.c_str(), req.length() + 1) != req.length() + 1) {
        return false;
    }

    char c = 1;
    while (c != '\0') {
        if (socket.Read(&c, 1) != 1)
            return false;
        else {
            rep += c;
        }
    }
    rep.pop_back();

    if (strcmp(rep.c_str(), "TRUE") == 0) {
        std::cout << "Le fichier existe" << std::endl;
    }
    else {
        std::cout << "Le fichier n'existe pas" << std::endl;
    }

    return true;
}

/**
 * Envoi un fichier vers le serveur
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * - nom du fichier (const char *)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool SendFile(MySocket::Socket_TCP& socket_tcp, const char * file_name) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        std::cout << "ERREUR : Lecture du fichier \"" << file_name << "\" impossible" << std::endl;
        if (socket_tcp.Write("\0\0\0\0", 4) != 4)
            return false;
        else
            return true;
    }

    //On détermine la taille du fichier
    struct stat st;
    stat(file_name, &st);
    uint32_t size = st.st_size;

    //On lit le fichier
    uint8_t * datas = new uint8_t[(size > 1024) ? 1024 : size];
    size_t _bufsize;

    //On envoie la taille sur 4 octets en little endian
    uint8_t bufsize[] = {
            (uint8_t)(size & 0xFF),
            (uint8_t)((size >> 8) & 0xFF),
            (uint8_t)((size >> 16) & 0xFF),
            (uint8_t)((size >> 24) & 0xFF)
    };
    if (socket_tcp.Write((void *)bufsize, 4) != 4) {
        close(fd);
        return false;
    }

    size_t i;
    bool success = true;
    clock_t cpt = clock();
    for (i = 0; i < size; i += 1024) {
        _bufsize = (size - i > 1024) ? 1024 : (size - i);
        if ((read(fd, datas, _bufsize) != _bufsize) || (socket_tcp.Write(datas, _bufsize) != _bufsize)) {
            std::cout << "ERREUR : Une erreur est survenue lors de la lecture du fichier \"" << file_name << "\"" << std::endl;
            success = false;
            break;
        }
        if (clock() > cpt + CLOCKS_PER_SEC * 2) {
            //Si cela fait 2 secondes que l'on attend, on affiche la progression
            std::cout << std::string(std::string("Envoi du fichier \"") + file_name + "\" [" + std::to_string((i + 1024) * 100 / size) + "%]\n");
            cpt = clock();
        }
    }

    close(fd);

    if (!success) {
        //On remplit la socket
        for (; i < size; i++) {
            if (socket_tcp.Write("\0", 1) != 1) {
                return false;
            }
        }
    }

    std::cout << "Fichier \"" << file_name << "\" envoye avec succes" << std::endl;

    return true;
}

/**
 * Reçoit un fichier depuis le serveur
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * - nom du fichier (const char *)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool RecvFile(MySocket::Socket_TCP& socket_tcp, const char * file_name) {
    uint8_t bufsize[4], * datas;
    if (socket_tcp.Read(bufsize, 4) == 0) return false;
    //On récupère la taille depuis le format little endian
    uint32_t size = ((uint32_t)bufsize[3] << 24) + ((uint32_t)bufsize[2] << 16) + ((uint32_t)bufsize[1] << 8) + ((uint32_t)bufsize[0]);

    if (size == 0) {
        std::cout << "ERREUR : Le serveur ne peut pas envoyer le fichier \"" << file_name << "\"" << std::endl;
        return true;
    }

    datas = new uint8_t[(size > 1024) ? 1024 : size];
    size_t _bufsize;

    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        std::cout << "ERREUR : Ecriture du fichier \"" << file_name << "\" impossible" << std::endl;
        //On vide la socket
        for (size_t i = 0; i < size; i += 1024) {
            _bufsize = (size - i > 1024) ? 1024 : (size - i);
            if (socket_tcp.Read(datas, _bufsize) != _bufsize) {
                return false;
            }
        }
        return true;
    }

    clock_t cpt = clock();
    for (size_t i = 0; i < size; i += 1024) {
        _bufsize = (size - i > 1024) ? 1024 : (size - i);
        //On récupère les données
        if (socket_tcp.Read(datas, _bufsize) != _bufsize) {
            close(fd);
            return false;
        }
        //On les écrit
        write(fd, datas, _bufsize);

        if (clock() > cpt + CLOCKS_PER_SEC * 2) {
            //Si cela fait 2 secondes que l'on attend, on affiche la progression
            std::cout << std::string(std::string("Telechargement du fichier \"") + file_name + "\" [" + std::to_string((i + 1024) * 100 / size) + "%]\n");
            cpt = clock();
        }
    }

    close(fd);

    std::cout << "Fichier \"" << file_name << "\" recu avec succes" << std::endl;

    return true;
}

/**
 * Envoi une requête "GET" au serveur et attend la réponse
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * - nom du(ou des) fichier(s) (char *)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool Request_Get(MySocket::Socket_TCP& socket, char * file_name) {
    std::string name;
    while (*file_name == ' ') file_name++;
    while (*file_name != ' ' && *file_name != '\0') {
        name += *file_name;
        file_name++;
    }

    std::string req = "GET " + name, rep;
    if (socket.Write(req.c_str(), req.length() + 1) != req.length() + 1) {
        return false;
    }

    if (!RecvFile(socket, name.c_str()))
        return false;

    while (*file_name == ' ') file_name++;
    if (*file_name == '\0')
        return true;
    else
        return Request_Get(socket, file_name);
}

/**
 * Envoi une requête "PUSH" au serveur
 * Paramètre:
 * - socket (MySocket::Socket_TCP&)
 * - nom du(ou des) fichier(s) (char *)
 * Retourne false si la connexion avec le serveur a été interrompue
*/
bool Request_Push(MySocket::Socket_TCP& socket, char * file_name) {
    std::string name;
    while (*file_name == ' ') file_name++;
    while (*file_name != ' ' && *file_name != '\0') {
        name += *file_name;
        file_name++;
    }

    //On vérifie que le fichier existe
    int fd = open(name.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cout << "ERREUR : Le fichier \"" << name << "\" n'existe pas ou ne peut pas etre ouvert" << std::endl;
        return true;
    }
    close(fd);

    std::string req = "PUSH " + name, rep;
    if (socket.Write(req.c_str(), req.length() + 1) != req.length() + 1) {
        return false;
    }

    if (!SendFile(socket, name.c_str()))
        return false;

    while (*file_name == ' ') file_name++;
    if (*file_name == '\0')
        return true;
    else
        return Request_Push(socket, file_name);
}

/**
 * Fonction principale
 * Arguments:
 * - l'adresse du serveur
 * - le numéro de port
*/
int main(int argc, char * argv[]) {
    if (argc != 3) {
        std::cout << "ERREUR : Usage :\n> " << argv[0] << " <adresse du serveur> <numero de port du serveur>" << std::endl;
        return -1;
    }

    std::cout << "Tentative de connexion au serveur " << argv[1] << ":" << atoi(argv[2]) << std::endl;
	//On se connecte avec le serveur en créant la socket
    MySocket::Socket_TCP socket = MySocket::Socket_TCP(argv[1], atoi(argv[2]));
    char tmp;
    bool success;
	//On vérifie si la connexion a été établie (que la socket a été crée)
    if (success = socket.Success()) {
		//On vérifie que la connexion a été établie correctement (c'est à dire que l'on tranférer des données)
        if (socket.Read(&tmp, 1) != 1)
            success = false;
        else
            std::cout << "La connexion avec le serveur a ete etablie" << std::endl;
    }
    if (!success) {
        std::cout << "ECHEC : La connexion avec le serveur a echouee" << std::endl;
        return -1;
    }

	//On affiche les commandes disponibles
    std::cout << std::endl << help_string << std::endl;

    char commande[COMMANDE_MAXSIZE];
    bool error = false;
    while (strncmp(commande, "EXIT", 4) != 0) {
		//On lit les commandes de l'utilisateur
        std::cout << std::endl  << "Entrez une commande : ";
        std::cin.getline(commande, COMMANDE_MAXSIZE);

		//On détermine la commande entrée et on appelle la fonction correspondante
        if (strncmp(commande, "LIST", 4) == 0) {
            if (!Request_List(socket)) {
                error = true;
                break;
            }
        }
        else if (strncmp(commande, "EXIST ", 6) == 0) {
            if (!Request_Exist(socket, commande + 6)) {
                error = true;
                break;
            }
        }
        else if (strncmp(commande, "GET ", 4) == 0) {
            if (!Request_Get(socket, commande + 4)) {
                error = true;
                break;
            }
        }
        else if (strncmp(commande, "PUSH ", 5) == 0) {
            if (!Request_Push(socket, commande + 5)) {
                error = true;
                break;
            }
        }
        else if (strncmp(commande, "HELP", 4) == 0) {
            std::cout << help_string << std::endl;
        }
        else if (strncmp(commande, "EXIT", 4) == 0) {
            break;
        }
        else {
            std::cout << "ATTENTION : Commande inconnue, tapez \"HELP\" pour afficher les commandes disponibles" << std::endl;
        }
    }

	//S'il y a eu une intéruption de la connexion avec le serveur, on le fait savoir
    if (error) {
        std::cout << "ECHEC : La connexion avec le serveur a ete interrompue" << std::endl;
        return -1;
    }
	else {
		std::cout << "Fermeture de la connexion avec le serveur" << std::endl;
	}
	
    return 0;
}