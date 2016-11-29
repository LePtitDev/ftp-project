#include "serveur.h"

FTP_Server::Server::Server() : sock_listen() {
    this->__assignFileNames(".");
    this->stop = false;
}

FTP_Server::Server::Server(int portnum) : sock_listen(portnum) {
    this->__assignFileNames(".");
    this->stop = false;
}

bool FTP_Server::Server::Start() {
    if (!this->sock_listen.Listen(SOCKLISTEN_NUMBER)) return false;

    FTP_Server::Request_Params * params;
    while (!this->stop) {
        params = new FTP_Server::Request_Params;
        params->serv = this;
        params->req_sock = this->sock_listen.Accept();
    }

    return true;
}

void FTP_Server::Server::Stop() {
    this->stop = true;
}

void FTP_Server::Server::ReqList(MySocket::Socket_TCP * socket_tcp) {

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
        socket_tcp->Write('\0', 1);
    }
    delete socket_tcp;

    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char * argv[]) {
    FTP_Server::Server serv = FTP_Server::Server();

    return 0;
}