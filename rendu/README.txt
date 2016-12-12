---- SE TROUVE DANS L'ARCHIVE ----

- un répertoire "mono-client" avec un serveur gérant un seul client et qui se ferme lorsque le travail avec
  le client est terminé.
- un répertoire "multi-client" avec un serveur gérant plusieurs clients en même temps, il faut arrêter le
  serveur manuellement


---- POUR COMPILER ----

Aller dans le répertoire "mono-client" ou "multi-client" et entrer la commande : make
Les executables "client" et "serveur" sont alors créés.
Le serveur n'a pas besoin d'argument, il doit être exécuté en premier.
Le client a besoin de deux arguments :
- l'adresse du serveur
- le numéro de port
(exemple : ./client 10.5.12.13 25871)


---- REMARQUES ----

1. Il est conseillé d'exécuter le client et le serveur dans des dossiers séparer pour éviter des conflits de
   lecture et d'écriture sur un même fichier.

2. Parfois la connexion TCP ne se fait pas correctement pour une raison inconnue, dans ce cas on affiche une
   erreur, il faudra alors réessayer plusieurs fois jusqu'à ce que la connexion s'établisse correctement
   (il m'est arrivé de recommencer 5-6 fois avant que cela fonctionne).

3. Toutes les informations nécéssaires pour l'utilisation du client et du serveur sont affichés à l'exécution,
   donc lisez bien.

4. Pour éviter de se balader dans l'arborescence, le serveur est programmé pour n'échanger que des fichiers du
   répertoire courant.

5. Le "mono-client" et "multi-client" sont tous les deux capables d'expoter un fichier depuis un client vers le
   serveur avec la commande "PUSH" donc nous n'avons pas créé de répertoire "bonus".

6. L'adresse affichée par le serveur est l'adresse locale de la machine, si vous exécutez le client et le serveur
   sur des terminaux différents entrez l'adresse suivante :
   10.[numéro du bâtiment].[numéro de la salle].[numéro du terminal]


---- EXECUTION ----

1. Commencez par exécuter le serveur.
2. A l'exécution, le serveur affiche son adresse locale et le numéro de port (aléatoire) utilisé pour l'écoute.
3. Exécutez le client avec en arguments l'adresse du serveur et le numéro de port affiché par celui-ci.
4. Si la connexion a fonctionnée, le serveur affiche "Connexion de xxx.xxx.xxx.xxx:xxxxx" et le client affiche
   "La connexion avec le serveur a été établie", puis la liste des commandes disponibles.
   Sinon le serveur indique qu'un client a essayé de se connecter sans succès et le client affiche une erreur.
6. Lorsque le tranfert dure longtemps (pour les gros fichiers) le client affiche la progression du transfert
   toutes les 2 secondes.
7. Lors de la déconnexion du client, le serveur "mono-client" s'arrète, pas le serveur "mutli-client".
8. Pour arrêter le serveur "multi-client", il faut entrer "EXIT" dans le terminal du serveur puis faire CTRL+C.