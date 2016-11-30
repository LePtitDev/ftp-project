#include "mysocket.h"

/****** CLASS ADDRESS ******/

MySocket::Address::Address() {
    this->s_addr.sin_family = AF_INET;
    struct hostent * ht = gethostbyname("localhost");
    if (ht) {
        this->s_addr.sin_addr.s_addr = MySocket::Address::_getipbyname(ht);
        MySocket::Address::_addrtoip(this->s_ip, &(this->s_addr));
    }
    else {
        this->s_success = false;
        return;
    }
    this->s_port = 0;
    this->s_addr.sin_port = htons((unsigned short)this->s_port);
    this->s_success = true;
}

MySocket::Address::Address(int portn) {
    this->s_addr.sin_family = AF_INET;
    struct hostent * ht = gethostbyname("localhost");
    if (ht) {
        this->s_addr.sin_addr.s_addr = MySocket::Address::_getipbyname(ht);
        MySocket::Address::_addrtoip(this->s_ip, &(this->s_addr));
    }
    else {
        this->s_success = false;
        return;
    }
    this->s_port = portn;
    this->s_addr.sin_port = htons((unsigned short)this->s_port);
    this->s_success = true;
}

MySocket::Address::Address(const char * addr, int portn) {
    this->s_addr.sin_family = AF_INET;
    struct hostent * ht = gethostbyname(addr);
    if (ht) {
        this->s_addr.sin_addr.s_addr = MySocket::Address::_getipbyname(ht);
        MySocket::Address::_addrtoip(this->s_ip, &(this->s_addr));
    }
    else {
        this->s_success = false;
        return;
    }
    this->s_port = portn;
    this->s_addr.sin_port = htons((unsigned short)this->s_port);
    this->s_success = true;
}

MySocket::Address::Address(const struct sockaddr_in * addr) {
    this->s_addr.sin_family = addr->sin_family;
    this->s_addr.sin_addr = addr->sin_addr;
    this->s_addr.sin_port = addr->sin_port;
    MySocket::Address::_addrtoip(this->s_ip, &(this->s_addr));
    this->s_port = MySocket::Address::_addrtoportn(addr);
    this->s_success = true;
}

bool MySocket::Address::Success() const {
    return this->s_success;
}

const char* MySocket::Address::GetIP() const {
    return this->s_ip;
}

int MySocket::Address::GetPort() const {
    return this->s_port;
}

const struct sockaddr_in* MySocket::Address::GetSockaddr() const {
    return &(this->s_addr);
}

void MySocket::Address::Refresh() {
    MySocket::Address::_addrtoip(this->s_ip, &(this->s_addr));
    this->s_port = MySocket::Address::_addrtoportn(&(this->s_addr));
}

in_addr_t MySocket::Address::_getipbyname(struct hostent * ht) {
    return ((struct in_addr *)(ht->h_addr_list[0]))->s_addr;
}

void MySocket::Address::_addrtoip(char * dest, const struct sockaddr_in * addr) {
    strcpy(dest, inet_ntoa(addr->sin_addr));
}

int MySocket::Address::_addrtoportn(const struct sockaddr_in * addr) {
    return ntohs(addr->sin_port);
}

/****** CLASS SOCKET_TCP ******/

MySocket::Socket_TCP::Socket_TCP() : c_addr() {
    this->_init_serv();
}

MySocket::Socket_TCP::Socket_TCP(int portn) : c_addr(portn) {
    this->_init_serv();
}

MySocket::Socket_TCP::Socket_TCP(const char * addr, int portn) : c_addr(addr, portn) {
    if (!this->c_addr.Success()) {
        this->s_success = false;
        return;
    }

    this->s_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->s_socket == -1) {
        this->s_success = false;
        return;
    }

    int res = bind(this->s_socket, (const struct sockaddr *) this->c_addr.GetSockaddr(), sizeof(struct sockaddr_in));
    if (res < 0) {
        this->s_success = false;
        close(this->s_socket);
        return;
    }

    this->c_addr.Refresh();

    this->s_success = true;
}

MySocket::Socket_TCP::Socket_TCP(const MySocket::Socket_TCP &cpy): c_addr(cpy.c_addr), s_socket(cpy.s_socket),
    s_success(cpy.s_success)
{}

MySocket::Socket_TCP::~Socket_TCP() {
    close(s_socket);
}

bool MySocket::Socket_TCP::Success() {
    return this->s_success;
}

bool MySocket::Socket_TCP::Listen(int size) {
    return (listen(this->s_socket, size) < 0);
}

MySocket::Socket_TCP * MySocket::Socket_TCP::Accept() {
    struct sockaddr_in addr;
    socklen_t len;
    int descr = accept(this->s_socket, (struct sockaddr *)&addr, &len);
    return new MySocket::Socket_TCP(descr, &addr);
}

bool MySocket::Socket_TCP::Connect(MySocket::Address &addr_serv) {
    int result = connect(s_socket, (const struct sockaddr *)addr_serv.GetSockaddr(), sizeof(struct sockaddr));
    if(result == -1)
        perror("Erreur de connexion");
    return result != -1;
}

const MySocket::Address& MySocket::Socket_TCP::GetDestination() const {
    return this->c_addr;
}

ssize_t MySocket::Socket_TCP::Read(void *buf, size_t max) {
    return recv(this->s_socket, buf, max, 0);
}

ssize_t MySocket::Socket_TCP::Write(void *datas, size_t len) {
    return send(this->s_socket, datas, len, 0);
}

MySocket::Socket_TCP::Socket_TCP(int descr, struct sockaddr_in * addr) : c_addr(addr) {
    this->s_socket = descr;
    this->s_success = true;
}

void MySocket::Socket_TCP::_init_serv() {
    if (!this->c_addr.Success()) {
        this->s_success = false;
        return;
    }

    this->s_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (this->s_socket == -1) {
        this->s_success = false;
        return;
    }

    int res = bind(this->s_socket, (const struct sockaddr *) this->c_addr.GetSockaddr(), sizeof(struct sockaddr_in));
    if (res < 0) {
        this->s_success = false;
        close(this->s_socket);
        return;
    }

    this->c_addr.Refresh();

    this->s_success = true;
}
