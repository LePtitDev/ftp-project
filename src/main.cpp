#include <iostream>
#include <sys/stat.h>

int main(int argc, char * argv[]) {
    char * filename = "/auto_home/aferre/Documents/Réseaux et communication/ftp-project/PROTOCOLE.txt";

    struct stat st;
    stat(filename, &st);
    int size = st.st_size;

    std::cout << size << std::endl;
	
	return 0;
}