#include "serveur.h"

//UTILE POUR L'AFFICHAGE DES MESSAGES
// "\x0d" remet le curseur au début de la ligne
// "\033[K" efface la ligne

FTP_Server::Server::Server() : sock_listen() {
    this->__assignFileNames(".");
    this->stop = false;
    this->mtx_stop = PTHREAD_MUTEX_INITIALIZER;
    this->mtx_file_names = PTHREAD_MUTEX_INITIALIZER;
    this->cond_file_names = PTHREAD_COND_INITIALIZER;
}

FTP_Server::Server::Server(int portnum) : sock_listen(portnum) {
    this->__assignFileNames(".");
    this->stop = false;
    this->mtx_stop = PTHREAD_MUTEX_INITIALIZER;
    this->mtx_file_names = PTHREAD_MUTEX_INITIALIZER;
    this->cond_file_names = PTHREAD_COND_INITIALIZER;
}

bool FTP_Server::Server::Start() {
    if (!this->sock_listen.Listen(SOCKLISTEN_NUMBER)) return false;

    FTP_Server::Request_Params * params;
    pthread_mutex_lock(&this->mtx_stop);
    while (!this->stop) {
        pthread_mutex_unlock(&this->mtx_stop);
        params = new FTP_Server::Request_Params;
        params->serv = this;
        params->req_sock = this->sock_listen.Accept();
        pthread_mutex_lock(&this->mtx_stop);
    }
    pthread_mutex_unlock(&this->mtx_stop);

    return true;
}

void FTP_Server::Server::Stop() {
    pthread_mutex_lock(&this->mtx_stop);
    this->stop = true;
    pthread_mutex_unlock(&this->mtx_stop);
}

void FTP_Server::Server::ReqList(MySocket::Socket_TCP * socket_tcp) {
    std::string rep;
    pthread_mutex_lock(&this->mtx_file_names);
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        rep += this->file_names[i] + ";";
    }
    pthread_mutex_unlock(&this->mtx_file_names);
    socket_tcp->Write((void *)rep.c_str(), rep.size() + 1);
}

void FTP_Server::Server::ReqExist(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool result = false;
    pthread_mutex_lock(&this->mtx_file_names);
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        if (this->file_names[i] == file_name) {
            result = true;
            break;
        }
    }
    pthread_mutex_unlock(&this->mtx_file_names);
    if (result) {
        socket_tcp->Write((void *)"TRUE", 5);
    }
    else {
        socket_tcp->Write((void *)"FALSE", 6);
    }
}

void FTP_Server::Server::ReqGet(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool tmp = false;
    pthread_mutex_lock(&this->mtx_file_names);
    //On vérifie que le nom du fichier se trouve dans la liste
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        if (this->file_names[i] == file_name) {
            tmp = true;
            break;
        }
    }
    if (tmp) {
        //On vérifie que le nom du fichier ne se trouve dans la liste des fichiers en écriture
        while (tmp) {
            tmp = false;
            for (int i = 0, sz = this->write_files.size(); i < sz; i++) {
                if (this->write_files[i] == file_name) {
                    tmp = true;
                    pthread_cond_wait(&this->cond_file_names, &this->mtx_file_names);
                    break;
                }
            }
        }
        //On ajoute le fichier en lecture même s'il y est déjà
        this->read_files.push_back(file_name);
        tmp = true;
    }
    pthread_mutex_unlock(&this->mtx_file_names);
    if (tmp) {
        this->SendFile(socket_tcp, file_name);
        //On enlève le fichier de la liste des lectures
        pthread_mutex_lock(&this->mtx_file_names);
        for (int i = 0, sz = this->read_files.size(); i < sz; i++) {
            if (this->read_files[i] == file_name) {
                this->read_files.erase(this->read_files.begin() + i);
            }
        }
        pthread_mutex_unlock(&this->mtx_file_names);
    }
    else {
        //On indique qu'il y a eu une erreur
        socket_tcp->Write((void *)"\0\0\0\0", 4);
    }
}

void FTP_Server::Server::ReqPush(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    bool tmp = false;
    pthread_mutex_lock(&this->mtx_file_names);
    //On vérifie que le nom du fichier se trouve dans la liste
    for (int i = 0, sz = this->file_names.size(); i < sz; i++) {
        if (this->file_names[i] == file_name) {
            tmp = true;
            break;
        }
    }
    if (tmp) {
        //On vérifie que le nom du fichier ne se trouve dans la liste des fichiers en écriture ou en lecture
        while (tmp) {
            tmp = false;
            for (int i = 0, sz = this->write_files.size(); i < sz; i++) {
                if (this->write_files[i] == file_name) {
                    tmp = true;
                    pthread_cond_wait(&this->cond_file_names, &this->mtx_file_names);
                    break;
                }
            }
            if (!tmp) {
                for (int i = 0, sz = this->read_files.size(); i < sz; i++) {
                    if (this->read_files[i] == file_name) {
                        tmp = true;
                        pthread_cond_wait(&this->cond_file_names, &this->mtx_file_names);
                        break;
                    }
                }
            }
        }
        //On ajoute le fichier en écriture
        this->write_files.push_back(file_name);
        tmp = true;
    }
    pthread_mutex_unlock(&this->mtx_file_names);
    if (tmp) {
        this->RecvFile(socket_tcp, file_name);
        //On enlève le fichier de la liste des écritures
        pthread_mutex_lock(&this->mtx_file_names);
        for (int i = 0, sz = this->write_files.size(); i < sz; i++) {
            if (this->write_files[i] == file_name) {
                this->write_files.erase(this->write_files.begin() + i);
            }
        }
        pthread_mutex_unlock(&this->mtx_file_names);
    }
    else {
        //On indique qu'il y a eu une erreur
        socket_tcp->Write((void *)"\0\0\0\0", 4);
    }
}

void FTP_Server::Server::SendFile(MySocket::Socket_TCP * socket_tcp, const char * file_name) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        //On indique qu'il y a eu une erreur
        socket_tcp->Write((void *)"\0\0\0\0", 4);
        return;
    }

    struct stat st;
    stat(file_name, &st);
    uint32_t size = st.st_size;

    //On envoie la taille sur 4 octets en little endian
    uint8_t bufsize[] = {
            size & 0xFF,
            (size >> 8) & 0xFF,
            (size >> 16) & 0xFF,
            (size >> 24) & 0xFF
    };
    socket_tcp->Write((void *)bufsize, 4);

    //A FINIR
}

void FTP_Server::Server::RecvFile(MySocket::Socket_TCP * socket_tcp, const char * file_name) {

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

void* FTP_Server::Request_Thread(void * params) {
    MySocket::Socket_TCP * socket_tcp = ((FTP_Server::Request_Params *)params)->req_sock;
    FTP_Server::Server * server = ((FTP_Server::Request_Params *)params)->serv;

    std::string req;
    char tmp;
    while ((socket_tcp->Read(&tmp, 1) != 0) && (tmp != '\0')) {
        req += tmp;
    }

    if (req[0] == 'L' && req[1] == 'I' && req[2] == 'S' && req[3] == 'T') {
        //Traitement de la requête "LIST"
        server->ReqList(socket_tcp);
    }
    else if (req[0] == 'E' && req[1] == 'X' && req[2] == 'I' && req[3] == 'S' && req[4] == 'T') {
        //Traitement de la requête "EXIST"
        tmp = 6;
        //On oublie les espaces
        while (req[tmp] == ' ') tmp++;
        server->ReqExist(socket_tcp, req.c_str() + tmp);
    }
    else if (req[0] == 'G' && req[1] == 'E' && req[2] == 'T') {
        //Traitement de la requête "GET"
        tmp = 4;
        //On oublie les espaces
        while (req[tmp] == ' ') tmp++;
        server->ReqGet(socket_tcp, req.c_str() + tmp);
    }
    else if (req[0] == 'P' && req[1] == 'U' && req[2] == 'S' && req[3] == 'H') {
        //Traitement de la requête "PUSH"
        tmp = 5;
        //On oublie les espaces
        while (req[tmp] == ' ') tmp++;
        server->ReqPush(socket_tcp, req.c_str() + tmp);
    }
    else {
        //Requête inconnue
        socket_tcp->Write((void *)"\0\0\0\0", 4);
    }
    delete socket_tcp;

    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[]) {
    FTP_Server::Server serv = FTP_Server::Server();

    return 0;
}