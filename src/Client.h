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
    Client(MySocket::Socket_TCP c_socket);

    const byte * RecvMessage();
    bool SendMessage(std::string messageText);
    bool SendMessage(byte *message, size_t msgSize);

    const MySocket::Socket_TCP &GetSocket() const;
    const MySocket::Address &GetDestination() const;
    const byte *GetBufferIn() const;
    const byte *GetBufferOut() const;

    bool isConnected() const;

private:
    MySocket::Socket_TCP m_socket;
    ssize_t m_sizeBufferIn;
    byte m_bufferIn[BUFFER_SIZE];
    byte m_bufferOut[BUFFER_SIZE];
    bool m_isConnected;
};

