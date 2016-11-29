#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

void listAllFiles( const std::string &dirName )
{
	DIR *dirp = opendir( dirName.c_str() );

	if ( dirp )
	{
		struct dirent *dp = NULL;

		while ( (dp = readdir( dirp )) != NULL )
		{
			std::string file( dp->d_name );

			if ( file == "." || file == ".." )    // skip these
				continue;

			if ( dp->d_type & DT_DIR )
			{
				// found a directory; recurse into it.
				std::string filePath = dirName + "/" + file;

				listAllFiles( filePath );
			}
			else
			{
				// regular file found
				std::cout << "filename is: " << file << std::endl;
			}
		}

		closedir( dirp );
	}
}


int main(int argc, char * argv[]) {
	listAllFiles(".");
	
	return 0;
}