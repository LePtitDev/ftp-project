all: src/mysocket.h src/mysocket.cpp src/serveur.h src/serveur.cpp src/myclient.h src/myclient.cpp
	g++ -std=c++11 -pthread src/mysocket.cpp src/serveur.cpp -o bin/serveur
	g++ -std=c++11 -pthread src/mysocket.cpp src/myclient.cpp -o bin/client

serveur: src/mysocket.h src/mysocket.cpp src/serveur.h src/serveur.cpp
	g++ -std=c++11 -pthread src/mysocket.cpp src/serveur.cpp -o bin/serveur

client: src/mysocket.h src/mysocket.cpp src/myclient.h src/myclient.cpp
	g++ -std=c++11 -pthread src/mysocket.cpp src/myclient.cpp -o bin/client