---- SE TROUVE DANS L'ARCHIVE ----

- un r�pertoire "mono-client" avec un serveur g�rant un seul client et qui se ferme lorsque le travail avec
  le client est termin�.
- un r�pertoire "multi-client" avec un serveur g�rant plusieurs clients en m�me temps, il faut arr�ter le
  serveur manuellement


---- POUR COMPILER ----

Aller dans le r�pertoire "mono-client" ou "multi-client" et entrer la commande : make
Les executables "client" et "serveur" sont alors cr��s.
Le serveur n'a pas besoin d'argument, il doit �tre ex�cut� en premier.
Le client a besoin de deux arguments :
- l'adresse du serveur
- le num�ro de port
(exemple : ./client 10.5.12.13 25871)


---- REMARQUES ----

1. Il est conseill� d'ex�cuter le client et le serveur dans des dossiers s�parer pour �viter des conflits de
   lecture et d'�criture sur un m�me fichier.

2. Parfois la connexion TCP ne se fait pas correctement pour une raison inconnue, dans ce cas on affiche une
   erreur, il faudra alors r�essayer plusieurs fois jusqu'� ce que la connexion s'�tablisse correctement
   (il m'est arriv� de recommencer 5-6 fois avant que cela fonctionne).

3. Toutes les informations n�c�ssaires pour l'utilisation du client et du serveur sont affich�s � l'ex�cution,
   donc lisez bien.

4. Pour �viter de se balader dans l'arborescence, le serveur est programm� pour n'�changer que des fichiers du
   r�pertoire courant.

5. Le "mono-client" et "multi-client" sont tous les deux capables d'expoter un fichier depuis un client vers le
   serveur avec la commande "PUSH" donc nous n'avons pas cr�� de r�pertoire "bonus".

6. L'adresse affich�e par le serveur est l'adresse locale de la machine, si vous ex�cutez le client et le serveur
   sur des terminaux diff�rents entrez l'adresse suivante :
   10.[num�ro du b�timent].[num�ro de la salle].[num�ro du terminal]


---- EXECUTION ----

1. Commencez par ex�cuter le serveur.
2. A l'ex�cution, le serveur affiche son adresse locale et le num�ro de port (al�atoire) utilis� pour l'�coute.
3. Ex�cutez le client avec en arguments l'adresse du serveur et le num�ro de port affich� par celui-ci.
4. Si la connexion a fonctionn�e, le serveur affiche "Connexion de xxx.xxx.xxx.xxx:xxxxx" et le client affiche
   "La connexion avec le serveur a �t� �tablie", puis la liste des commandes disponibles.
   Sinon le serveur indique qu'un client a essay� de se connecter sans succ�s et le client affiche une erreur.
6. Lorsque le tranfert dure longtemps (pour les gros fichiers) le client affiche la progression du transfert
   toutes les 2 secondes.
7. Lors de la d�connexion du client, le serveur "mono-client" s'arr�te, pas le serveur "mutli-client".
8. Pour arr�ter le serveur "multi-client", il faut entrer "EXIT" dans le terminal du serveur puis faire CTRL+C.