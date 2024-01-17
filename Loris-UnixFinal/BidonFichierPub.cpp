#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocole.h"

int main()
{
  int fd;

  if ((fd = open("publicites.dat",O_CREAT | O_WRONLY, 0644)) == -1)
  {
    perror("Erreur de open");
    exit(1);
  }

  PUBLICITE pub[] = 
  {
    {"Les profs d'UNIX et de C++ sont les meilleurs !",5},
    {"Emplacement libre pour publicité pour seulement un rein",7},
    {"En C++, une étape par semaine et c'est gagné !",4},
    {"Le Labo UNIX, c'est pas compliqué...",6},
    {"Loris aura son permis de conduire avant 2025",5},
    {"Je vends une Fiat Multipla verte pour 800€", 4},
    {"Il a neigé le mercredi 17 janvier 2024, je l'ai vu", 5}
  };

  for (int i=0 ; i<7 ; i++)
  	if (write(fd,&pub[i],sizeof(PUBLICITE)) != sizeof(PUBLICITE))
  	{
  		perror("Erreur de write");
  		exit(1);
  	}
  
  close(fd);
}