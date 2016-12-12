#include "serveur.h"

//UTILE POUR L'AFFICHAGE DES MESSAGES
// "\x0d" remet le curseur au début de la ligne
// "\033[K" efface la ligne

FTP_Server::Server::Server() : sock_listen() {
    this->__assignFileNames(".");
}

FTP_Server::Server::Server(int portnum) : sock_listen(portnum) {
    this->__assignFileNames(".");
}

bool FTP_Server::Server::Start() {
    if (!this->sock_listen.Listen(SOCKLISTEN_NUMBER)) return false;
    bool error = false;

    /**** AFFICHE LES FICHIERS ****/
    std::cout << "Liste des fichiers :" << std::endl;
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        std::cout << "- " << this->file_names[i] << std::endl;
    }
    std::cout << std::endl;

	MySocket::Socket_TCP * socket_tcp = this->sock_listen.Accept();

    std::string req;
    char tmp;
    ssize_t recepsucc;
	std::string debugMsg;

    if (socket_tcp->Write("\0", 1) != 1) {
        std::cout << "Une tentative de connexion a echouee (il est conseille de redemarrer le serveur)\n";
        pthread_exit(NULL);
        return NULL;
    }

	//--- DEBUG
	debugMsg = std::string("Connexion de ") + socket_tcp->GetDestination().GetIP() + ":" + std::to_string(socket_tcp->GetDestination().GetPort()) + "\n";
	std::cout << debugMsg;
	//--- DEBUG

    while (true) {
        req.clear();
        while ((recepsucc = socket_tcp->Read(&tmp, 1) != 0) && (tmp != '\0')) {
            req += tmp;
        }

        //On vérifie que la socket n'a pas été fermée
        if (recepsucc <= 0) break;

        if (req != "") {
            //--- DEBUG
            debugMsg = std::string("Requete de ") + socket_tcp->GetDestination().GetIP() + ":" + std::to_string(socket_tcp->GetDestination().GetPort()) + " >> " + req + "\n";
            std::cout << debugMsg;
            //--- DEBUG
        }

        if (req[0] == 'L' && req[1] == 'I' && req[2] == 'S' && req[3] == 'T') {
            //Traitement de la requête "LIST"
            this->ReqList(socket_tcp);
        } else if (req[0] == 'E' && req[1] == 'X' && req[2] == 'I' && req[3] == 'S' && req[4] == 'T') {
            //Traitement de la requête "EXIST"
            tmp = 6;
            //On oublie les espaces
            while (req[tmp] == ' ') tmp++;
            this->ReqExist(socket_tcp, req.c_str() + tmp);
        } else if (req[0] == 'G' && req[1] == 'E' && req[2] == 'T') {
            //Traitement de la requête "GET"
            tmp = 4;
            //On oublie les espaces
            while (req[tmp] == ' ') tmp++;
            this->ReqGet(socket_tcp, req.c_str() + tmp);
        } else if (req[0] == 'P' && req[1] == 'U' && req[2] == 'S' && req[3] == 'H') {
            //Traitement de la requête "PUSH"
            tmp = 5;
            //On oublie les espaces
            while (req[tmp] == ' ') tmp++;
            this->ReqPush(socket_tcp, req.c_str() + tmp);
        } else {
            //Requête inconnue
            socket_tcp->Write((void *) "\0\0\0\0", 4);
        }

        if (req != "") {
            //--- DEBUG
            debugMsg = std::string("Reponse envoyee a ") + socket_tcp->GetDestination().GetIP() + ":" + std::to_string(socket_tcp->GetDestination().GetPort()) + "\n";
            std::cout << debugMsg;
            //--- DEBUG
        }

    }
	
	//--- DEBUG
    if (recepsucc == 0) {
        debugMsg = std::string("Deconnexion de ");
    }
    else {
        debugMsg = std::string("Erreur de connexion avec ");
    }
	debugMsg += std::string(socket_tcp->GetDestination().GetIP()) + ":" + std::to_string(socket_tcp->GetDestination().GetPort()) + "\n";
	std::cout << debugMsg;
	//--- DEBUG

    delete socket_tcp;

    return !error;
}

void FTP_Server::Server::Stop() {
    this->stop = true;
}

const MySocket::Address& FTP_Server::Server::GetAddress() {
	return this->sock_listen.GetDestination();
}

bool FTP_Server::Server::NeedToStop() {
    bool res = this->stop;
    return res;
}

void FTP_Server::Server::ReqList(MySocket::Socket_TCP * socket_tcp) {
    std::string rep;
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        rep += this->file_names[i] + ";";
    }
    socket_tcp->Write((void *)rep.c_str(), rep.size() + 1);
}

void FTP_Server::Server::ReqExist(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool result = false;
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        if (this->file_names[i] == file_name) {
            result = true;
            break;
        }
    }
    if (result) {
        socket_tcp->Write((void *)"TRUE", 5);
    }
    else {
        socket_tcp->Write((void *)"FALSE", 6);
    }
}

void FTP_Server::Server::ReqGet(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool tmp = false;
    //On vérifie que le nom du fichier se trouve dans la liste
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        if (this->file_names[i] == file_name) {
            tmp = true;
            break;
        }
    }
    if (tmp) {
        this->SendFile(socket_tcp, file_name);
    }
    else {
        //On indique qu'il y a eu une erreur
        socket_tcp->Write((void *)"\0\0\0\0", 4);
    }
}

void FTP_Server::Server::ReqPush(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool tmp = true;
    bool success = this->RecvFile(socket_tcp, file_name);

    if (success) {
        //On vérifie que le fichier est dans la liste des fichiers
        tmp = false;
        for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
            if (this->file_names[i] == file_name) {
                tmp = true;
                break;
            }
        }
        //S'il n'est pas présent, on l'ajoute
        if (!tmp)
            this->file_names.push_back(std::string(file_name));
    }
}

void FTP_Server::Server::SendFile(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        //On indique qu'il y a eu une erreur
        socket_tcp->Write((void *)"\0\0\0\0", 4);
        return;
    }

    //On détermine la taille du fichier
    struct stat st;
    stat(file_name, &st);
    uint32_t size = st.st_size;
    //On envoie la taille sur 4 octets en little endian
    uint8_t bufsize[] = {
            (uint8_t)(size & 0xFF),
            (uint8_t)((size >> 8) & 0xFF),
            (uint8_t)((size >> 16) & 0xFF),
            (uint8_t)((size >> 24) & 0xFF)
    };
    socket_tcp->Write((void *)bufsize, 4);

    //On lit le fichier
    uint8_t * datas = new uint8_t[(size > 1024) ? 1024 : size];
    size_t _bufsize;
    bool success = true;
    size_t i;
    for (i = 0; i < size; i += 1024) {
        _bufsize = (size - i > 1024) ? 1024 : (size - i);
        if ((read(fd, datas, _bufsize) != _bufsize) || (socket_tcp->Write(datas, _bufsize) != _bufsize)) {
            //On indique qu'il y a eu une erreur
            success = false;
            break;
        }
    }
    close(fd);

    if (!success) {
        //On finit de remplir la socket
        for (; i < size; i++) {
            if (socket_tcp->Write("\0", 1) != 1) {
                return;
            }
        }
    }
}

bool FTP_Server::Server::RecvFile(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    uint8_t bufsize[4], * datas;
    if (socket_tcp->Read(bufsize, 4) != 4) return false;
    //On récupère la taille depuis le format little endian
    uint32_t size = ((uint32_t)bufsize[3] << 24) + ((uint32_t)bufsize[2] << 16) + ((uint32_t)bufsize[1] << 8) + ((uint32_t)bufsize[0]);

    datas = new uint8_t[(size > 1024) ? 1024 : size];
    size_t _bufsize;


    char tmp;
    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        //On vide la socket
        for (size_t i = 0; i < size; i++) {
            if (socket_tcp->Read(&tmp, 1) != 1) return false;
        }
    }
    else {
        bool success = true;
        size_t i;
        for (i = 0; i < size; i += 1024) {
            //On récupère les données
            _bufsize = (size - i > 1024) ? 1024 : (size - i);
            if ((socket_tcp->Read(datas, _bufsize) != _bufsize) || (write(fd, datas, _bufsize) != _bufsize)) {
                success = false;
                break;
            }
        }

        if (!success) {
            //On vide la socket
            for (; i < size; i++) {
                if (socket_tcp->Read(&tmp, 1) != 1) {
                    close(fd);
                    return false;
                }
            }
        }

        close(fd);
    }
    return true;
}

bool FTP_Server::Server::__assignFileNames(const char * dir_name) {
    DIR * dirp = opendir(dir_name);
    if (dirp != NULL) {
        struct dirent * dp = NULL;

        while ((dp = readdir(dirp)) != NULL)
        {
            std::string file_name(dp->d_name);

            if (file_name == "." || file_name == "..") continue;
            else if (!(dp->d_type & DT_DIR)) this->file_names.push_back(file_name);
        }

        closedir( dirp );
        return true;
    }
    else {
        return false;
    }
}

int main(int argc, char * argv[]) {
    FTP_Server::Server serv = FTP_Server::Server();
	std::cout << "Adresse du serveur : " << serv.GetAddress().GetIP() << ":" << serv.GetAddress().GetPort() << std::endl;
    serv.Start();

    return 0;
}