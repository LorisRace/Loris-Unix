#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <mysql.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h"

int idQ,idSem;

struct sembuf Operations[2];

int semAttente();
int semEssaiAttente();
int semPost();

int main()
{
  /*semAttente();
  semEssaiAttente();
  semPost();*/

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CONSULTATION %d) Recuperation de l'id de la file de messages\n",getpid());

  if((idQ = msgget(CLE,0)) == -1)
  {
    perror("Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant du sémaphore
  if ((idSem = semget(CLE,0,0)) == -1)
  {
    perror("Erreur de semget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE Consulter;
  msgrcv(idQ, &m, sizeof(MESSAGE) - sizeof(long), getpid(), 0);
  // Lecture de la requête CONSULT
  fprintf(stderr,"(CONSULTATION %d) Lecture requete CONSULT\n",getpid());

  // Tentative de prise bloquante du semaphore 0

  fprintf(stderr,"(CONSULTATION %d) Prise bloquante du sémaphore 0\n",getpid());
  semAttente();

  // Connexion à la base de donnée
  MYSQL *connexion = mysql_init(NULL);
  fprintf(stderr,"(CONSULTATION %d) Connexion à la BD\n",getpid());
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(CONSULTATION) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Recherche des infos dans la base de données
  fprintf(stderr,"(CONSULTATION %d) Consultation en BD (%s)\n",getpid(),m.data1);
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  char requete[200];
  sprintf(requete,"select * from UNIX_FINAL where nom like '%s';",m.data1);

  mysql_query(connexion,requete),
  resultat = mysql_store_result(connexion);
  // if ((tuple = mysql_fetch_row(resultat)) != NULL) ...
  if ((tuple = mysql_fetch_row(resultat)) != NULL)
  {
    strcpy(Consulter.data1,"OK");
    strcpy(Consulter.data2,tuple[2]);
    strcpy(Consulter.texte,tuple[3]);
  }
  else
  {
    strcpy(Consulter.data1,"KO");
  }

  // Construction et envoi de la reponse
    Consulter.type=m.expediteur;
    Consulter.expediteur=getpid();
    Consulter.requete=CONSULT;


    msgsnd(idQ,&Consulter,sizeof(MESSAGE) - sizeof(long), 0);
    kill(Consulter.type,SIGUSR1);

  // Deconnexion BD
  mysql_close(connexion);

  // Libération du semaphore 0
  fprintf(stderr,"(CONSULTATION %d) Libération du sémaphore 0\n",getpid());

  fprintf(stderr,"(CONSULTATION %d) Libération du sémaphore 0\n",getpid());
  semPost();


  fprintf(stderr,"\n(%d |2| %d)%s %s\n",m.type,m.expediteur,m.data1,m.data2);   
  fprintf(stderr,"\n(%d |2| %d)%s %s\n",m.type,m.expediteur,m.data1,m.data2);

  exit(0);
}

int semAttente()
{
  struct sembuf Operations={0,-1,SEM_UNDO};
  return semop(idSem,&Operations,1);
}

int semEssaiAttente()
{
  struct sembuf Operations={0,-1,SEM_UNDO | IPC_NOWAIT};
  return semop(idSem,&Operations,1);
}

int semPost()
{
  struct sembuf Operations={0,1,SEM_UNDO};
  return semop(idSem,&Operations,1);
}