#include "FichierUtilisateur.h"

int estPresent(const char* nom)
{
  UTILISATEUR Utilisateur;
  int Position = 1;

  int Fichier = open(FICHIER_UTILISATEURS, O_RDONLY);

  if(Fichier == -1)
  {
    //perror("Le fichier n'existe pas OU Erreur d'ouverture du fichier\n");
    return -1;
  }

  while(read(Fichier, &Utilisateur, sizeof(UTILISATEUR)) > 0)
  {
    if(strcmp(Utilisateur.nom, nom) == 0)
    {
      close(Fichier);
      return Position;
    }

    Position++;
  }

  close(Fichier);

  return -1;

}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
    int somme = 0, i, pos;

    for (i = 0, pos = 1; motDePasse[i] != '\0'; i++, pos++) 
    {
        somme = somme + pos * motDePasse[i];
    }

    somme = somme % 97;

    return somme;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteUtilisateur(const char* nom, const char* motDePasse)
{
  UTILISATEUR Utilisateur;

  int Fichier = open(FICHIER_UTILISATEURS, O_WRONLY | O_APPEND);

  if(Fichier == -1)
  {
    Fichier = open(FICHIER_UTILISATEURS, O_WRONLY | O_CREAT | O_EXCL, 0777);    
  }

  strcpy(Utilisateur.nom, nom);

  Utilisateur.hash = hash(motDePasse);

  write(Fichier, &Utilisateur, sizeof(UTILISATEUR));

  close(Fichier);

}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  UTILISATEUR Utilisateur;
  int Fichier = open(FICHIER_UTILISATEURS, O_RDONLY);

    if (!Fichier) 
    {
        return -1; 
    }

    
    lseek(Fichier, (pos - 1) * sizeof(UTILISATEUR), SEEK_SET);

    if (read(Fichier, &Utilisateur, sizeof(UTILISATEUR)) != sizeof(UTILISATEUR)) 
    {
        close(Fichier);
        return -1; 
    }

    close(Fichier);

    return (hash(motDePasse) == Utilisateur.hash) ? 1 : 0;

}


////////////////////////////////////////////////////////////////////////////////////
int listeUtilisateurs(UTILISATEUR *vecteur) // le vecteur doit etre suffisamment grand
{
  UTILISATEUR Utilisateur;
  int Nombre_Utilisateurs = 0;

  int Fichier = open(FICHIER_UTILISATEURS, O_RDONLY);

  if(!Fichier)
  {
    perror("Le fichier n'existe pas OU Erreur d'ouverture du fichier\n");
    return -1;
  }

  while(read(Fichier, &Utilisateur, sizeof(UTILISATEUR)) != 0)
  {
    vecteur[Nombre_Utilisateurs] = Utilisateur;

    Nombre_Utilisateurs++;
  }

  close(Fichier);

  return Nombre_Utilisateurs;
  
}

int ChangerMotDePasse(int Position, const char *MotDePasse2)
{
  UTILISATEUR Utilisateur;

  int Fichier;

  if ((Fichier = open(FICHIER_UTILISATEURS, O_RDWR) == -1)&& strcmp(MotDePasse2, "") == 0)
  {
    return -1;
  }

  lseek (Fichier, (Position - 1) *sizeof(UTILISATEUR), SEEK_SET);

  Utilisateur.hash = hash(MotDePasse2);

  if (write(Fichier, &Utilisateur, sizeof(UTILISATEUR)) != sizeof(UTILISATEUR))
  {
    close(Fichier);

    return -1;
  }

  close(Fichier);
}
