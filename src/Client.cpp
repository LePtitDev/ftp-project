#include "Client.h"

Client::Client(MySocket::Socket_TCP c_socket):m_socket(c_socket), m_sizeBufferIn(0), m_isConnected(true)
{
    memset(m_bufferIn, 0, BUFFER_SIZE);
    memset(m_bufferOut, 0, BUFFER_SIZE);
}

const byte* Client::RecvMessage() {
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

bool Client::SendMessage(std::string messageText) {
    m_sizeBufferIn = m_socket.Write((void*)messageText.c_str(), messageText.size());
    if(m_sizeBufferIn < 0) perror("Impossible d'envoyer le message au client!");
    return m_sizeBufferIn >= 0;
}

bool Client::SendMessage(byte *message, size_t msgSize) {
    m_sizeBufferIn = m_socket.Write((void*)message, msgSize);
    if(m_sizeBufferIn < 0) perror("Impossible d'envoyer le message au client!");
    return m_sizeBufferIn >= 0;
}

const MySocket::Socket_TCP &Client::GetSocket() const {
    return m_socket;
}

const MySocket::Address &Client::GetDestination() const {
    return m_socket.GetDestination();
}

const byte *Client::GetBufferIn() const {
    return m_bufferIn;
}

const byte *Client::GetBufferOut() const {
    return m_bufferOut;
}

bool Client::isConnected() const {
    return m_isConnected;
}
