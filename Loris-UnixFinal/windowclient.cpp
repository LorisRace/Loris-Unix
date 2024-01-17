#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/shm.h>
#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include "dialogmodification.h"
#include <unistd.h>
#include <fcntl.h>

extern WindowClient *w;

#include "protocole.h"

int idQ, idShm;

PUBLICITE *PubLLMC;
#define TIME_OUT 150
int timeOut = TIME_OUT;

void handlerSIGUSR1(int sig);
void HandlerSIGUSR2(int sig);
void HandlerSIGALRM(int sig);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowClient::WindowClient(QWidget *parent):QMainWindow(parent),ui(new Ui::WindowClient)
{
    ui->setupUi(this);
    ::close(2);
    logoutOK();

    // Recuperation de l'identifiant de la file de messages
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    if ((idQ = msgget(CLE,0)) == -1)
      {
      perror("Erreur lors de la création de la file de messages");
      exit(1);
      }


    // Recuperation de l'identifiant de la mémoire partagée
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    if((idShm =shmget(CLE,0,0))==-1)
    {
      perror("Erreur lors de la création de la mémoire partagée");
      exit(1);
    }

    // Attachement à la mémoire partagée
    if((PubLLMC = (PUBLICITE*) shmat(idShm, NULL, SHM_RDONLY)) == (PUBLICITE*) - 1)
    {
      perror("Erreur lors du rattachement à la mémoire partagée");
    }

    // Armement des signaux
    struct sigaction User1;
    User1.sa_handler = handlerSIGUSR1;
    User1.sa_flags = 0;
    sigemptyset(&User1.sa_mask);
    sigaction(SIGUSR1, &User1, NULL);

    struct sigaction User2;
    User2.sa_handler = HandlerSIGUSR2;
    User1.sa_flags = 0;
    sigemptyset(&User2.sa_mask);
    sigaction(SIGUSR2, &User2, NULL);

    struct sigaction Alarm;
    Alarm.sa_handler = HandlerSIGALRM;
    Alarm.sa_flags = 0;
    sigemptyset(&Alarm.sa_mask);
    sigaction(SIGALRM, &Alarm, NULL);

    // Envoi d'une requete de connexion au serveur
    MESSAGE Connecte;
    Connecte.type = 1;
    Connecte.expediteur = getpid();
    Connecte.requete = CONNECT;

     if(msgsnd(idQ,&Connecte,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("Erreur lors de l'envoi du message");
      exit(1);
    }

}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(connectes[0],ui->lineEditNom->text().toStdString().c_str());
  return connectes[0];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauChecked()
{
  if (ui->checkBoxNouveau->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTimeOut(int nb)
{
  ui->lcdNumberTimeOut->display(nb);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setAEnvoyer(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditAEnvoyer->clear();
    return;
  }
  ui->lineEditAEnvoyer->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getAEnvoyer()
{
  strcpy(aEnvoyer,ui->lineEditAEnvoyer->text().toStdString().c_str());
  return aEnvoyer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPersonneConnectee(int i,const char* Text)
{
  if (strlen(Text) == 0 )
  {
    switch(i)
    {
        case 1 : ui->lineEditConnecte1->clear(); break;
        case 2 : ui->lineEditConnecte2->clear(); break;
        case 3 : ui->lineEditConnecte3->clear(); break;
        case 4 : ui->lineEditConnecte4->clear(); break;
        case 5 : ui->lineEditConnecte5->clear(); break;
    }
    return;
  }
  switch(i)
  {
      case 1 : ui->lineEditConnecte1->setText(Text); break;
      case 2 : ui->lineEditConnecte2->setText(Text); break;
      case 3 : ui->lineEditConnecte3->setText(Text); break;
      case 4 : ui->lineEditConnecte4->setText(Text); break;
      case 5 : ui->lineEditConnecte5->setText(Text); break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getPersonneConnectee(int i)
{
  QLineEdit *tmp;
  switch(i)
  {
    case 1 : tmp = ui->lineEditConnecte1; break;
    case 2 : tmp = ui->lineEditConnecte2; break;
    case 3 : tmp = ui->lineEditConnecte3; break;
    case 4 : tmp = ui->lineEditConnecte4; break;
    case 5 : tmp = ui->lineEditConnecte5; break;
    default : return NULL;
  }

  strcpy(connectes[i],tmp->text().toStdString().c_str());
  return connectes[i];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteMessage(const char* personne,const char* message)
{
  // Choix de la couleur en fonction de la position
  int i=1;
  bool trouve=false;
  while (i<=5 && !trouve)
  {
      if (getPersonneConnectee(i) != NULL && strcmp(getPersonneConnectee(i),personne) == 0) trouve = true;
      else i++;
  }
  char couleur[40];
  if (trouve)
  {
      switch(i)
      {
        case 1 : strcpy(couleur,"<font color=\"red\">"); break;
        case 2 : strcpy(couleur,"<font color=\"blue\">"); break;
        case 3 : strcpy(couleur,"<font color=\"green\">"); break;
        case 4 : strcpy(couleur,"<font color=\"darkcyan\">"); break;
        case 5 : strcpy(couleur,"<font color=\"orange\">"); break;
      }
  }
  else strcpy(couleur,"<font color=\"black\">");
  if (strcmp(getNom(),personne) == 0) strcpy(couleur,"<font color=\"purple\">");

  // ajout du message dans la conversation
  char buffer[300];
  sprintf(buffer,"%s(%s)</font> %s",couleur,personne,message);
  ui->textEditConversations->append(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNomRenseignements(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNomRenseignements->clear();
    return;
  }
  ui->lineEditNomRenseignements->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNomRenseignements()
{
  strcpy(nomR,ui->lineEditNomRenseignements->text().toStdString().c_str());
  return nomR;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setGsm(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditGsm->clear();
    return;
  }
  ui->lineEditGsm->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setEmail(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditEmail->clear();
    return;
  }
  ui->lineEditEmail->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setCheckbox(int i,bool b)
{
  QCheckBox *tmp;
  switch(i)
  {
    case 1 : tmp = ui->checkBox1; break;
    case 2 : tmp = ui->checkBox2; break;
    case 3 : tmp = ui->checkBox3; break;
    case 4 : tmp = ui->checkBox4; break;
    case 5 : tmp = ui->checkBox5; break;
    default : return;
  }
  tmp->setChecked(b);
  if (b) tmp->setText("Accepté");
  else tmp->setText("Refusé");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveau->setEnabled(false);
  ui->pushButtonEnvoyer->setEnabled(true);
  ui->pushButtonConsulter->setEnabled(true);
  ui->pushButtonModifier->setEnabled(true);
  ui->checkBox1->setEnabled(true);
  ui->checkBox2->setEnabled(true);
  ui->checkBox3->setEnabled(true);
  ui->checkBox4->setEnabled(true);
  ui->checkBox5->setEnabled(true);
  ui->lineEditAEnvoyer->setEnabled(true);
  ui->lineEditNomRenseignements->setEnabled(true);
  setTimeOut(TIME_OUT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditNom->setText("");
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->lineEditMotDePasse->setText("");
  ui->checkBoxNouveau->setEnabled(true);
  ui->pushButtonEnvoyer->setEnabled(false);
  ui->pushButtonConsulter->setEnabled(false);
  ui->pushButtonModifier->setEnabled(false);
  for (int i=1 ; i<=5 ; i++)
  {
      setCheckbox(i,false);
      setPersonneConnectee(i,"");
  }
  ui->checkBox1->setEnabled(false);
  ui->checkBox2->setEnabled(false);
  ui->checkBox3->setEnabled(false);
  ui->checkBox4->setEnabled(false);
  ui->checkBox5->setEnabled(false);
  setNomRenseignements("");
  setGsm("");
  setEmail("");
  ui->textEditConversations->clear();
  setAEnvoyer("");
  ui->lineEditAEnvoyer->setEnabled(false);
  ui->lineEditNomRenseignements->setEnabled(false);
  setTimeOut(TIME_OUT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Clic sur la croix de la fenêtre ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
    timeOut = TIME_OUT;
    MESSAGE Deconnecte;
    Deconnecte.type = 1;
    Deconnecte.expediteur = getpid();
    Deconnecte.requete = DECONNECT;

    if(msgsnd(idQ,&Deconnecte,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("Erreur lors de l'envoi du message");
      exit(1);
    }

    MESSAGE Utilisateur_Deconnecte;
    Utilisateur_Deconnecte.type = 1;
    Utilisateur_Deconnecte.expediteur = getpid();
    Utilisateur_Deconnecte.requete = LOGOUT;

    msgsnd(idQ, &Utilisateur_Deconnecte, sizeof(MESSAGE) - sizeof(long), 0);
    QApplication::exit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    timeOut = TIME_OUT;

    MESSAGE Utilisateur_Connecte;

    sprintf(Utilisateur_Connecte.data1,"%d", isNouveauChecked());
    strcpy(Utilisateur_Connecte.data2, getNom());
    strcpy(Utilisateur_Connecte.texte, getMotDePasse());

    Utilisateur_Connecte.type = 1;
    Utilisateur_Connecte.expediteur = getpid();
    Utilisateur_Connecte.requete = LOGIN;

    msgsnd(idQ, &Utilisateur_Connecte, sizeof(MESSAGE) - sizeof(long), 0);    

}

void WindowClient::on_pushButtonLogout_clicked()
{
    timeOut = TIME_OUT;

    MESSAGE Utilisateur_Deconnecte;
    Utilisateur_Deconnecte.type = 1;
    Utilisateur_Deconnecte.expediteur = getpid();
    Utilisateur_Deconnecte.requete = LOGOUT;

    msgsnd(idQ, &Utilisateur_Deconnecte, sizeof(MESSAGE) - sizeof(long), 0);
    logoutOK();
}

void WindowClient::on_pushButtonEnvoyer_clicked()
{
    timeOut = TIME_OUT;

    MESSAGE EnvoiMessage;
    EnvoiMessage.type = 1;
    EnvoiMessage.expediteur = getpid();
    strcpy(EnvoiMessage.texte, getAEnvoyer());
    EnvoiMessage.requete = SEND;

    msgsnd(idQ, &EnvoiMessage, sizeof(MESSAGE) - sizeof(long), 0);
    ajouteMessage("Vous : ", getAEnvoyer());
}

void WindowClient::on_pushButtonConsulter_clicked()
{
    timeOut = TIME_OUT;

    MESSAGE Consulte;
    Consulte.type = 1;
    Consulte.expediteur = getpid();
    strcpy(Consulte.data1, getNomRenseignements());
    Consulte.requete = CONSULT;

    if(msgsnd(idQ, &Consulte, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
      perror("Erreur lors de l'envoi du message");
      exit(1);
    }

    setGsm("---En attente−−−");
    setEmail("---En attente−−−");

}

void WindowClient::on_pushButtonModifier_clicked()
{
  timeOut = TIME_OUT;
  // Envoi d'une requete MODIF1 au serveur
  MESSAGE m;
  m.type = 1;
  m.expediteur = getpid();
  m.requete = MODIF1;

  msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0);

  // Attente d'une reponse en provenance de Modification
  fprintf(stderr,"(CLIENT %d) Attente reponse MODIF1\n",getpid());
  msgrcv(idQ, &m, sizeof(MESSAGE) - sizeof(long), getpid(), 0);

  // Verification si la modification est possible
  if (strcmp(m.data1,"KO") == 0 && strcmp(m.data2,"KO") == 0 && strcmp(m.texte,"KO") == 0)
  {
    QMessageBox::critical(w,"Problème...","Modification déjà en cours...");
    return;
  }

  // Modification des données par utilisateur
  DialogModification dialogue(this,getNom(),"",m.data2,m.texte);
  dialogue.exec();
  char motDePasse[40];
  char gsm[40];
  char email[40];
  strcpy(motDePasse,dialogue.getMotDePasse());
  strcpy(gsm,dialogue.getGsm());
  strcpy(email,dialogue.getEmail());

  // Envoi des données modifiées au serveur
  m.type = 1;
  m.expediteur = getpid();
  strcpy(m.data1, motDePasse);
  strcpy(m.data2, gsm);
  strcpy(m.texte, email);
  m.requete = MODIF2;
  msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les checkbox ///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_checkBox1_clicked(bool checked)
{
    timeOut = TIME_OUT;
    MESSAGE Utilisateur_Accepte;

    if (checked)
    {
        ui->checkBox1->setText("Accepté");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(1));
        Utilisateur_Accepte.requete = ACCEPT_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
    else
    {
        ui->checkBox1->setText("Refusé");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(1));
        Utilisateur_Accepte.requete = REFUSE_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
}

void WindowClient::on_checkBox2_clicked(bool checked)
{
    timeOut = TIME_OUT;
    MESSAGE Utilisateur_Accepte;

    if (checked)
    {
        ui->checkBox2->setText("Accepté");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(2));
        Utilisateur_Accepte.requete = ACCEPT_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
    else
    {
        ui->checkBox2->setText("Refusé");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(2));
        Utilisateur_Accepte.requete = REFUSE_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
}

void WindowClient::on_checkBox3_clicked(bool checked)
{
    timeOut = TIME_OUT;
    MESSAGE Utilisateur_Accepte;

    if (checked)
    {
        ui->checkBox3->setText("Accepté");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(3));
        Utilisateur_Accepte.requete = ACCEPT_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
    else
    {
        ui->checkBox3->setText("Refusé");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(3));
        Utilisateur_Accepte.requete = REFUSE_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
}

void WindowClient::on_checkBox4_clicked(bool checked)
{
    timeOut = TIME_OUT;
    MESSAGE Utilisateur_Accepte;

    if (checked)
    {
        ui->checkBox4->setText("Accepté");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(4));
        Utilisateur_Accepte.requete = ACCEPT_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
    else
    {
        ui->checkBox4->setText("Refusé");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(4));
        Utilisateur_Accepte.requete = REFUSE_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
}

void WindowClient::on_checkBox5_clicked(bool checked)
{
    timeOut = TIME_OUT;
    MESSAGE Utilisateur_Accepte;

    if (checked)
    {
        ui->checkBox5->setText("Accepté");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(5));
        Utilisateur_Accepte.requete = ACCEPT_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
    else
    {
        ui->checkBox5->setText("Refusé");
        Utilisateur_Accepte.type = 1;
        Utilisateur_Accepte.expediteur = getpid();
        strcpy(Utilisateur_Accepte.data1, w->getPersonneConnectee(5));
        Utilisateur_Accepte.requete = REFUSE_USER;
        msgsnd(idQ, &Utilisateur_Accepte, sizeof(MESSAGE) - sizeof(long), 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    (void)sig;
    MESSAGE m;
    int i;

    while(msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),IPC_NOWAIT)!=-1)
    {
      switch(m.requete)
      {
        case LOGIN :
                    if (strcmp(m.data1,"OK") == 0)
                    {
                      fprintf(stderr,"(CLIENT %d) Login OK\n",getpid());
                      w->loginOK();
                      w->dialogueMessage("Login...",m.texte);
                      timeOut = TIME_OUT;
                      alarm(1);
                    }
                    else w->dialogueErreur("Login...",m.texte);
                    break;

        case ADD_USER :
                    timeOut = TIME_OUT;

                    for(i = 1; i < 6; i++)
                    {
                      if((strcmp(w->getPersonneConnectee(i), "") == 0))
                      {
                        w->setPersonneConnectee(i, m.data1);
                        i = 6;
                      }
                    }
                    break;

        case REMOVE_USER :
                    timeOut = TIME_OUT;

                    for(i = 1; i < 6; i++)
                    {
                      if((strcmp(w->getPersonneConnectee(i), m.data1) == 0))
                      {
                        w->setPersonneConnectee(i,"");
                        i = 6;
                      }
                    }
                    break;

        case SEND :
                    timeOut = TIME_OUT;
                    w->ajouteMessage(m.data1, m.texte);
                    break;

        case CONSULT :
                  timeOut = TIME_OUT;

                  if(strcmp(m.data1, "OK") == 0)
                  {
                    w->setGsm(m.data2);
                    w->setEmail(m.texte);
                  }

                  else
                  {
                    w->setGsm("<< GSM Non Trouvé >>");
                    w->setEmail("<< Email Non Trouvé >>");
                  }
                  break;
      }
    }  
}

void HandlerSIGUSR2(int sig)
{
  (void)sig;
  w->setPublicite(PubLLMC->texte);
}

void HandlerSIGALRM(int sig)
{
  (void)sig;
  timeOut--;

  w->setTimeOut(timeOut);

  if(timeOut == 0)
  {
    MESSAGE Chrono;
    Chrono.type = 1;
    Chrono.expediteur = getpid();
    Chrono.requete = LOGOUT;

    msgsnd(idQ, &Chrono, sizeof(MESSAGE) - sizeof(long), 0);
    w->logoutOK();
  }

  else
  {
    alarm(1);
  }
}
