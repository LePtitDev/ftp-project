all: serveur client

serveur: mysocket.h mysocket.cpp serveur.h serveur.cpp
	g++ -std=c++11 -pthread mysocket.cpp serveur.cpp -o serveur

client: mysocket.h mysocket.cpp myclient.cpp
	g++ -std=c++11 -pthread mysocket.cpp myclient.cpp -o client