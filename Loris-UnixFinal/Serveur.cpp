#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <mysql.h>
#include <setjmp.h>
#include <fcntl.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierUtilisateur.h"

int idQ,idShm,idSem;

int Retour;

int Id_Memoire_Partagee;
int Publicite_Pub;
sigjmp_buf flag;


TAB_CONNEXIONS *tab;
void afficheTab();
MYSQL* connexion;

void HandlerSigint(int sig);
void HandlerSigChld(int sig);

union semun
{
  int Valeur;
  struct semid_ds *Buffer;
  unsigned short *Array;
} argument;


int main()
{
  // Connection à la BD
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Armement des signaux
  struct sigaction LLMC_INT;
  LLMC_INT.sa_handler = HandlerSigint;
  sigemptyset(&LLMC_INT.sa_mask);
  LLMC_INT.sa_flags = 0;
  sigaction(SIGINT, &LLMC_INT, NULL);

  struct sigaction LLMC_CHLD;
  LLMC_CHLD.sa_handler = HandlerSigChld;
  sigemptyset(&LLMC_CHLD.sa_mask);
  LLMC_CHLD.sa_flags = 0;
  sigaction(SIGCHLD, &LLMC_CHLD, NULL);

  // Creation des ressources
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600);
  if (idQ == -1)  
  {
    perror("(SERVEUR) Une erreur est survenue lors de la création de la file de messages");
    exit(1);
  }

  fprintf(stderr,"(SERVEUR %d) Creation de la sémaphore\n",getpid());
  idSem = semget(CLE, 1, IPC_CREAT | IPC_EXCL | 0600);
  if (idSem == -1)
  {
    perror("(SERVEUR) Une erreur est survenue lors de la création de la sémaphore");
    exit(1);
  }
  semctl(idSem, 0, SETVAL, 1);

  fprintf(stderr,"(SERVEUR %d) Creation de la mémoire partagée\n",getpid());
  Id_Memoire_Partagee = shmget(CLE, 200, IPC_CREAT | IPC_EXCL | 0600);
  if(Id_Memoire_Partagee == -1)
  {
    perror("(SERVEUR) Une erreur est survenue lors de la création de la mémoire partagée");
  }


  /*connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }*/

  // Initialisation du tableau de connexions
  fprintf(stderr,"(SERVEUR %d) Initialisation de la table des connexions\n",getpid());
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    for (int j=0 ; j<5 ; j++) tab->connexions[i].autres[j] = 0;
    tab->connexions[i].pidModification = 0;
  }
  tab->pidServeur1 = getpid();
  tab->pidServeur2 = 0;
  tab->pidAdmin = 0;
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite
  Publicite_Pub =  fork();

  if(Publicite_Pub == 0)
  {
    execl("./Publicite", "Publicite", (char *)NULL);
  }

  int i, j, k;
  int Temporaire, i1, i2;
  MESSAGE m;
  MESSAGE reponse;
  char requete[200];
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  PUBLICITE pub;

  while(1)
  {
  	if ((Retour = sigsetjmp(flag,1)) != 0)
    {
      printf("\nRetour du saut %d...\n",Retour);
    }

    fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT : { 
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      i = 0;

                      while(tab->connexions[i].pidFenetre != 0 && i < 6)
                      {
                          i++;
                      }

                      if (i < 6)
                      {
                        tab->connexions[i].pidFenetre = m.expediteur;
                      }
                      break;
                    } 

      case DECONNECT :  
                      {
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      i = 0;

                      while(m.expediteur != tab->connexions[i].pidFenetre && i < 6)
                      {
                          i++;
                      }

                      if (m.expediteur == tab->connexions[i].pidFenetre)
                      {
                        tab->connexions[i].pidFenetre = 0;
                      }
                      break; 
                    }

      case LOGIN :  {
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%s--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.texte);
                      i = 0;
                      bool Connexion_Effectuee = false;

                      while (Connexion_Effectuee == false && i < 6)
                      {
                        if (strcmp(tab->connexions[i].nom, m.data2) == 0)
                        {
                          Connexion_Effectuee = true;
                        }

                        i++;
                      }

                      if (Connexion_Effectuee == true)
                      {
                        strcpy(reponse.data1, "KO");
                        strcpy(reponse.texte, "L'utilisateur est déjà connecté");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);
                        break;
                      }

                      bool Nouvel_Utilisateur = (strcmp(m.data1, "1") == 0); 

                      int Position = estPresent(m.data2);

                      if (Nouvel_Utilisateur && Position > 0)
                      {
                        strcpy(reponse.data1, "KO");
                        strcpy(reponse.texte, "L'utilisateur existe déjà");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);
                        break;
                      }


                      if (Nouvel_Utilisateur)
                      {
                        ajouteUtilisateur(m.data2, m.texte);
                        strcpy(reponse.data1, "OK");
                        strcpy(reponse.texte, "L'utilisateur vient d'être créé");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);
                        

                        i = 0;
                        while(tab->connexions[i].pidFenetre != m.expediteur && i < 6)
                        {
                          i++;
                        }

                        strcpy(tab->connexions[i].nom, m.data2);
                        sprintf(requete,"insert into UNIX_FINAL values (NULL,'%s','---','---');",m.data2);
                        mysql_query(connexion, requete);
                        fprintf(stderr, "Insertion effectuée avec succès\n");

                        for(j = 0; j < 6; j++)
                        {
                          if(tab->connexions[i].pidFenetre && tab->connexions[j].pidFenetre != m.expediteur && strcmp(tab->connexions[j].nom,""))
                          {
                            reponse.type = tab->connexions[j].pidFenetre;
                            reponse.requete = ADD_USER;
                            strcpy(reponse.data1, m.data2);
                            msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                            kill(m.expediteur, SIGUSR1);

                            reponse.type = m.expediteur;
                            strcpy(reponse.data1, tab->connexions[j].nom);
                            msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                          }
                        }
                        
                        kill(m.expediteur, SIGUSR1);
                        break;
                        
                      }

                      if (Position <= 0)
                      {
                        strcpy(reponse.data1, "KO");
                        strcpy(reponse.texte, "L'utilisateur n'existe pas");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);
                        break;
                      }

                      if (verifieMotDePasse(Position, m.texte) == 1)
                      {
                        strcpy(reponse.data1, "OK");
                        strcpy(reponse.texte, "L'utilisateur est connecté");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);

                        i = 0;

                        while(tab->connexions[i].pidFenetre != m.expediteur && i < 6)
                        {
                          i++;
                        }

                        strcpy(tab->connexions[i].nom, m.data2);

                        for (j = 0; j < 6; j++)
                        {
                          if(tab->connexions[i].pidFenetre && tab->connexions[j].pidFenetre != m.expediteur && strcmp(tab->connexions[j].nom, ""))
                          {
                            reponse.type = tab->connexions[j].pidFenetre;
                            reponse.requete = ADD_USER;
                            strcpy(reponse.data1, m.data2);
                            msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                            kill(tab->connexions[j].pidFenetre, SIGUSR1);

                            reponse.type = m.expediteur;
                            strcpy(reponse.data1, tab->connexions[j].nom);
                            msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                          }
                        }

                        kill(m.expediteur, SIGUSR1);
                        break;
                      }

                      if(verifieMotDePasse(Position, m.texte) != 1)
                      {
                        strcpy(reponse.data1, "KO");
                        strcpy(reponse.texte, "Le mot de passe est incorrect");
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();
                        reponse.requete = LOGIN;
                        msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                        kill(m.expediteur, SIGUSR1);
                        break;
                      }

                      break;
                    }
                       

      case LOGOUT :  {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      MESSAGE Deconnecte;

                      for(i = 0; i < 6; i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                        {
                          strcpy(Deconnecte.data1, tab->connexions[i].nom);
                          Deconnecte.requete = REMOVE_USER;
                          Deconnecte.expediteur = 1;
                          strcpy(tab->connexions[i].nom, "");

                          for(j = 0; j < 6; j++)
                          {
                            tab->connexions[i].autres[j] = 0;
                          }

                          i = 6;
                        }
                      }

                      for(i = 0; i < 6; i++)
                      {
                        for(j = 0; j < 6; j++)
                        {
                          if(tab->connexions[i].autres[j] == m.expediteur)
                          {
                            tab->connexions[i].autres[j] = 0;
                          }
                        }
                      }

                      for(i = 0; i < 6; i++)
                      {
                        if(tab->connexions[i].pidFenetre != 0 && strcmp(tab->connexions[i].nom, m.data2) != 0 && strcmp(tab->connexions[i].nom, "") != 0)
                        {
                          Deconnecte.type = tab->connexions[i].pidFenetre;
                          msgsnd(idQ, &Deconnecte, sizeof(MESSAGE) - sizeof(long), 0);
                          kill(Deconnecte.type, SIGUSR1);
                        }
                      }

                      break;
                     }
                      

      case ACCEPT_USER :

                        {
                          fprintf(stderr,"(SERVEUR %d) Requete ACCEPT_USER reçue de %d\n",getpid(),m.expediteur);
                          fprintf(stderr,"Notre pid:(%d)->(%d) ayant pout data1 %s ayant pour requete %d \n",m.expediteur,m.type,m.data1,m.requete);

                          for(k = 0; k < 6; k++)
                          {
                            if(tab->connexions[k].pidFenetre != 0 && strcmp(tab->connexions[k].nom, m.data1) == 0)
                            {
                              for(i = 0; i < 6; i++)
                              {
                                if(tab->connexions[i].pidFenetre == m.expediteur)
                                {
                                  for(j = 0; j < 5; j++)
                                  {
                                    if(tab->connexions[i].autres[j] == 0)
                                    {
                                      tab->connexions[i].autres[j] = tab->connexions[k].pidFenetre;
                                      j = 5;
                                    }
                                  }
                                }
                              }
                            }
                          }
                          break;
                        }
                      

      case REFUSE_USER :
                          {
                            fprintf(stderr,"(SERVEUR %d) Requete REFUSE_USER reçue de %d\n",getpid(),m.expediteur);

                            for(k = 0; k < 6; k++)
                            {
                              if(tab->connexions[k].pidFenetre != 0 && strcmp(tab->connexions[k].nom, m.data1) == 0)
                              {
                                for(i = 0; i < 6; i++)
                                {
                                  if(tab->connexions[i].pidFenetre = m.expediteur)
                                  {
                                    for(j = 0; j < 5; j++)
                                    {
                                      if(tab->connexions[i].autres[j] == tab->connexions[k].pidFenetre)
                                      {
                                        tab->connexions[i].autres[j] = 0;
                                        j = 5;
                                      }
                                    }
                                  }
                                }
                              }
                            }
                            break;
                          }
                      

      case SEND :  
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete SEND reçue de %d\n",getpid(),m.expediteur);
                        i = 0;

                        while(tab->connexions[i].pidFenetre != m.expediteur && i < 6)
                        {
                          i++;
                        }

                        for(j = 0; j < 5; j++)
                        {
                          if(tab->connexions[i].autres[j])
                          {
                            reponse.type = tab->connexions[i].autres[j];
                            reponse.expediteur = getpid();
                            reponse.requete = SEND;
                            strcpy(reponse.data1, tab->connexions[i].nom);
                            strcpy(reponse.texte, m.texte);
                            msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0);
                            kill(reponse.type, SIGUSR1);
                          }
                        }

                        break;
                      }
                       

      case UPDATE_PUB :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete UPDATE_PUB reçue de %d\n",getpid(),m.expediteur);
                        MESSAGE Publicite_Update;

                        Publicite_Update.expediteur = m.expediteur;

                        for(i = 0; i < 6; i++)
                        {
                          if(tab->connexions[i].pidFenetre != 0)
                          {
                            Publicite_Update.type = tab->connexions[i].pidFenetre;
                            msgsnd(idQ, &Publicite_Update, sizeof(MESSAGE) - sizeof(long), 0);
                            kill(Publicite_Update.type, SIGUSR2);
                          }
                        }
                        break;
                      }
                      

      case CONSULT :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                        MESSAGE Consulte;
                        int ProcessusConsultation;

                        ProcessusConsultation = fork();

                        if(ProcessusConsultation == 0)
                        {
                          execl("./Consultation", "Consultation", (char *)NULL);
                        }

                        m.type = ProcessusConsultation;

                        msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0);
                        fprintf(stderr,"\nLe pid de ce processus est : %d\n", ProcessusConsultation);

                        break;
                      }
                      

      case MODIF1 :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete MODIF1 reçue de %d\n",getpid(),m.expediteur);
                        int ProcessusModification1;

                        ProcessusModification1 = fork();

                        if(ProcessusModification1 == 0)
                        {
                          execl("./Modification", "Modification", (char *)NULL);
                        }

                        for(i = 0; i < 6; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            strcpy(m.data1, tab->connexions[i].nom);
                            m.type = ProcessusModification1;
                            tab->connexions[i].pidModification = m.type;
                            i = 6;
                          }
                        }

                        msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0);

                        fprintf(stderr, "\nLe pid de ce processus est : %d\n",ProcessusModification1);

                        break;
                      }
                      

      case MODIF2 :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete MODIF2 reçue de %d\n",getpid(),m.expediteur);

                        for(i = 0; i < 6; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidModification;
                            i = 6;
                          }
                        }

                        msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0);

                        break;
                      }
                      

      case LOGIN_ADMIN :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGIN_ADMIN reçue de %d\n",getpid(),m.expediteur);
                        break;
                      }
                      

      case LOGOUT_ADMIN :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGOUT_ADMIN reçue de %d\n",getpid(),m.expediteur);
                        break;
                      }
                      

      case NEW_USER :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete NEW_USER reçue de %d : --%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2);
                        break;
                      }
                      

      case DELETE_USER :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete DELETE_USER reçue de %d : --%s--\n",getpid(),m.expediteur,m.data1);
                        break;
                      }

      case NEW_PUB :
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                        break;
                      }
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur 1 : %d\n",tab->pidServeur1);
  fprintf(stderr,"Pid Serveur 2 : %d\n",tab->pidServeur2);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid Admin     : %d\n",tab->pidAdmin);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d %6d %6d %6d %6d - %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].autres[0],
                                                      tab->connexions[i].autres[1],
                                                      tab->connexions[i].autres[2],
                                                      tab->connexions[i].autres[3],
                                                      tab->connexions[i].autres[4],
                                                      tab->connexions[i].pidModification);
  fprintf(stderr,"\n");
}


void HandlerSigint(int sig)
{
  (void)sig;

  msgctl(idQ, IPC_RMID, NULL);
  shmctl(Id_Memoire_Partagee, IPC_RMID, NULL);
    
  
  if(semctl(idSem, 0, IPC_RMID) == -1)
  {
    perror("Erreur au niveau de la suppression de la sémaphore (1)");
    exit(1);
  }

  exit(0);
}

void HandlerSigChld(int sig)
{
  (void)sig;

  int i, Id, Statut;

  Id = wait(&Statut);
  fprintf(stderr,"Sig:%d, EX:%d \n",WIFSIGNALED(Statut),WIFEXITED(Statut));

  for(i = 0; i < 6; i++)
  {
    if(tab->connexions[i].pidModification == Id)
    {
      tab->connexions[i].pidModification = 0;
    }
  }

  fprintf(stderr,"(SERVEUR) Suspression du processus Zombie %d\n",Id);
  siglongjmp(flag,1);

}