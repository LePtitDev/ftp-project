#include "myclient.h"

#define COMMANDE_MAXSIZE 256

const char * help_string = "\
Liste des commandes :\n\
- LIST : affiche la liste des fichiers du serveur\n\
- EXIST <nom du fichier> : determine si le fichier existe\n\
- GET <nom du fichier> [...] : telecharge le(s) fichier(s) depuis le serveur s'il existe\n\
- PUSH <nom du fichier> [...] : envoie le(s) fichier(s) vers le serveur\n\
- HELP : affiche les commandes\n\
- EXIT : arrete la connexion";

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

bool SendFile(MySocket::Socket_TCP& socket_tcp, const char * file_name) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        std::cout << "ERREUR : Lecture du fichier \"" << file_name << "\" impossible" << std::endl;
        return true;
    }

    //On détermine la taille du fichier
    struct stat st;
    stat(file_name, &st);
    uint32_t size = st.st_size;

    //On lit le fichier
    uint8_t * datas = new uint8_t[size];
    if (read(fd, datas, size) <= 0) {
        std::cout << "ERREUR : Une erreur est survenue lors de la lecture du fichier \"" << file_name << "\"" << std::endl;
        close(fd);
        return true;
    }
    close(fd);

    //On envoie la taille sur 4 octets en little endian
    uint8_t bufsize[] = {
            (uint8_t)(size & 0xFF),
            (uint8_t)((size >> 8) & 0xFF),
            (uint8_t)((size >> 16) & 0xFF),
            (uint8_t)((size >> 24) & 0xFF)
    };

    if ((socket_tcp.Write((void *)bufsize, 4) != 4) || (socket_tcp.Write(datas, size) != size)) {
        return false;
    }

    std::cout << "Fichier \"" << file_name << "\" envoye avec succes" << std::endl;

    return true;
}

bool RecvFile(MySocket::Socket_TCP& socket_tcp, const char * file_name) {
    uint8_t bufsize[4], * datas;
    if (socket_tcp.Read(bufsize, 4) == 0) return false;
    //On récupère la taille depuis le format little endian
    uint32_t size = ((uint32_t)bufsize[3] << 24) + ((uint32_t)bufsize[2] << 16) + ((uint32_t)bufsize[1] << 8) + ((uint32_t)bufsize[0]);

    if (size == 0) {
        std::cout << "ERREUR : Le serveur ne peut pas envoyer le fichier \"" << file_name << "\"" << std::endl;
        return true;
    }

    datas = new uint8_t[size];
    //On récupère les données
    if (socket_tcp.Read(datas, size) == 0) {
        return false;
    }

    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        std::cout << "ERREUR : Ecriture du fichier \"" << file_name << "\" impossible" << std::endl;
        return true;
    }

    write(fd, datas, size);

    close(fd);

    std::cout << "Fichier \"" << file_name << "\" recu avec succes" << std::endl;

    return true;
}

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

bool Request_Push(MySocket::Socket_TCP& socket, char * file_name) {
    std::string name;
    while (*file_name == ' ') file_name++;
    while (*file_name != ' ' && *file_name != '\0') {
        name += *file_name;
        file_name++;
    }

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

int main(int argc, char * argv[]) {
    std::cout << "Tentative de connexion au serveur " << argv[1] << ":" << atoi(argv[2]) << std::endl;
    if (argc != 3) {
        std::cout << "ERREUR : Usage :\n> " << argv[0] << " <adresse du serveur> <numero de port du serveur>" << std::endl;
        return -1;
    }

    MySocket::Socket_TCP socket = MySocket::Socket_TCP(argv[1], atoi(argv[2]));
    if (socket.Success()) {
        std::cout << "La connexion avec le serveur a ete etablie" << std::endl;
    }
    else {
        std::cout << "ECHEC : La connexion avec le serveur a echouee" << std::endl;
        return -1;
    }

    std::cout << std::endl << help_string << std::endl;

    char commande[COMMANDE_MAXSIZE];
    bool error = false;
    while (strncmp(commande, "EXIT", 4) != 0) {
        std::cout << std::endl  << "Entrez une commande : ";
        std::cin.getline(commande, COMMANDE_MAXSIZE);

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

    if (error) {
        std::cout << "ECHEC : La connexion avec le serveur a ete interrompue" << std::endl;
        return -1;
    }

    std::cout << "Fermeture de la connexion avec le serveur" << std::endl;

    return 0;
}