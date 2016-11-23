#include <iostream>

#include "mysocket.h"

int main(int argc, char * argv[]) {
	MySocket::Address adress = MySocket::Address();
	std::cout << "Adresse IP : " << adress.GetIP() << ":" << adress.GetPort() << std::endl;
	
	return 0;
}