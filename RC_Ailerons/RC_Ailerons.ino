#include <Servo.h> //Bibliotheque de classe destinée aux Servomoteurs

/* Preprocesseur */
#define pinCanalAileronX   6
#define pinCanalAileronY   7

#define piNAileronX   9
#define piNAileronY   10

#define X   0
#define Y   1

/* Voici les tendances de joystick */
#define NA       0
#define MILIEU_X 1
#define MILIEU_Y 2
#define HAUT     3
#define BAS      4
#define GAUCHE   5
#define DROITE   6

/* Note: Il faut trouver un juste MILIEUx pour les battement exploiter */
#define D   0 // Battement max: 133 | 
#define G   1 // Battement max: 137 | mais 133 max

#define MIN   0
#define MAX   1
const int milieuX[] = { 1472, 1520 };
const int droiteX[] = { 1520, 1990 };
const int gaucheX[] = { 992, 1483 };
const int milieuY[] = { 1441, 1499 };
const int hautY[]   = { 1500, 1985 };
const int basY[]    = { 980, 1441 };

/* Déclaration des objets pour la manipulation des servomoteurs */
Servo srvAileron[2];

const int initPos[2] = { 94, 68 };
int lastPosition[2];

bool tendance[8]; //Pour allouer la tendence relative des joystick

void ordonnerTableau(int tableau[], int tailleTableau);

void setup() {
  Serial.begin(9600); 
  pinMode(piNAileronX, INPUT);
  pinMode(piNAileronY, INPUT);

  //Attachement des objets Servo au broche 9 Droite et 10 Gauche
  srvAileron[D].attach(9);
  srvAileron[G].attach(10); 

  //Initialise la position des servomoteur 
  srvAileron[D].write(initPos[D]);
  srvAileron[G].write(initPos[G]);

  //Inislisaton des tendances du joysick
  for(int i = 0; i < 8; i++) {
    tendance[i] = false;
  }
  
  //delay(3000);
}

void loop() {
  
  int chAileron[2] = { 0, 0};
  chAileron[X] = pulseIn(pinCanalAileronX, HIGH);
  chAileron[Y] = pulseIn(pinCanalAileronY, HIGH);

  if(chAileron[X] != 0 || chAileron[Y] != 0) {
    Serial.print("  chAileron[X] = ");
    Serial.println(chAileron[X]);
    Serial.print("  chAileron[Y] = ");
    Serial.println(chAileron[Y]);

    setJoystickTendance(chAileron[X], chAileron[Y]); //Met a jours le tableau "tendance" 

    int psD = 0, psG = 0;
    /* Les combinaisons possible le joystick */
    /*

        HAUT MILEU = G HAUT, D HAUT
        HAUT DROITE = G HAUT, D BAS
        HAUT GAUCHE = G HAUT, D BAS

        BAS MILEU = G BAS, D BAS
        BAS DROITE = G HAUT, D BAS
        BAS GAUCHE = G BAS, D HAUT
     */

    //Serial.println("Actionement");
    if(tendance[MILIEU_Y] && tendance[MILIEU_X]) {
      //Milieu
      psD = initPos[D];
      psG = initPos[G];
      Serial.println("  - MILIEU X et Y");

    } 
    
    else if(tendance[MILIEU_Y] && tendance[DROITE]) {
      //Droite seulement
      psD = map(chAileron[X], droiteX[MIN],  droiteX[MAX], initPos[D], 6);
      //BETA INVERSION
      psG = map(chAileron[X], droiteX[MIN],  droiteX[MAX], 27, initPos[G]);
      Serial.println("  - Droite seulement");

    } 
    else if(tendance[MILIEU_Y] && tendance[GAUCHE]) {
      //Gauche seulement
      psG = map(chAileron[X], gaucheX[MIN],  gaucheX[MAX], 159, initPos[G]);
      //BETA INVERSION
      psD = map(chAileron[X], gaucheX[MIN],  gaucheX[MAX], 138, initPos[D]);
      Serial.println("  - Gauche seulement");
      
    } 

    /*
    else if(tendance[MILIEU_X] && tendance[HAUT]) {
      //Haut seulement
      psD = map(chAileron[Y],  1425,  1920, initPos[G], 159);
      psG = map(chAileron[Y],  1425,  1920, initPos[D], 6);
      Serial.println("  - Haut seulement");
      
    }

     
    else if(tendance[MILIEU_X] && tendance[BAS]) {
      //Bas seulement
      psD = map(chAileron[Y],  993,  1375, initPos[G], 138);
      psG = map(chAileron[Y],  993,  1375, initPos[D], 27);
      Serial.println("  - Bas seulement");
      
    } 
    
    else if(tendance[HAUT] && tendance[DROITE]) {
      //Haut droite
      psD = map(chAileron[X],  1523,  1980, initPos[D], 27);
      psG = map(chAileron[Y],  1425,  1920, initPos[D], 27);
      Serial.println("  - Haut droite");
      
    } else if (tendance[HAUT] && tendance[GAUCHE]) {
      //Haut gauche
      psD = map(chAileron[Y],  1425,  1920, 162, initPos[G]);
      psG = map(chAileron[X],  992,  1439, 162, initPos[G] );
      Serial.println("  - Haut gauche");
      
    } else if (tendance[BAS] && tendance[DROITE]) {
      //Bas droite
      psD = map(chAileron[X],  1523,  1980, initPos[D], 27);
      psG = map(chAileron[Y],  993,  1375, initPos[D], 27);
      Serial.println("  - Bas droite");
      
    } else if (tendance[BAS] && tendance[GAUCHE]) {
      //Bas gauche
      psD = map(chAileron[Y],  993,  1375, 162, initPos[G]);
      psG = map(chAileron[X],  992,  1439, 162, initPos[G] );
      Serial.println("  - Bas gauche");

    }
    else {
      Serial.println("-  Erreur");
      psD = initPos[D];
      psG = initPos[G];
    }*/

    srvAileron[D].write(psD);
    srvAileron[G].write(psG);
    
  }
  //delay(1000);
}



void setJoystickTendance(int x, int y) {

  //Serial.println("Mise a jours des tendances");
  
  for(int i = 0; i < 7; i++) {
	  tendance[i] = false;
  }

  /* GAUCHE/DROITE */
  if(x > milieuX[MIN] && x <= milieuX[MAX]) {
    tendance[MILIEU_X] = true;
  } else if(x >= droiteX[MIN] && x <= droiteX[MAX]) {
    tendance[DROITE] = true;
  } else if(x >= gaucheX[MIN] && x < gaucheX[MAX]) {
    tendance[GAUCHE] = true;
  }
  
  /* HAUT/BAS */
  if(y > milieuY[MIN] && y <= milieuY[MAX]){
    tendance[MILIEU_Y] = true;
  } else if(y >= hautY[MIN] && y <= hautY[MAX]) {
    tendance[HAUT] = true;
  } else if(y >= basY[MIN] && y <= basY[MAX]) {
    tendance[BAS] = true;
  }

  /*if(x > 1472 && x <= 1520) {
    tendance[MILIEU_X] = true;
  } else if(x >= 1520 && x <= 1990) {
    tendance[DROITE] = true;
  } else if(x >= 992 && x < 1483) {
    tendance[GAUCHE] = true;
  }
  
  if(y > 1441 && y <= 1499){
    tendance[MILIEU_Y] = true;
  } else if(y >= 1500 && y <= 1985) {
    tendance[HAUT] = true;
  } else if(y >= 980 && y <= 1441) {
    tendance[BAS] = true;
  }*/

  /*for(int i = 0; i < 8; i++) {
    Serial.print("Statut tableau = ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.println(tendance[i]);
  }*/ 
}

/*void autoJoystickCalibrage() {
  /*
  * Principe de fonctionement d'apres le milieu, le max et le min
  * On peut calibrer nos joystick: On connais: le min et le max
  * Methode: au demarage determine la valeur min et max du MILIEU et le tout et et jouer
  * REMARQUE: on ne doit pas toucher au joystick
  */
  
  /*int milieu_min, milieu_max = 0;
  int echantillonsX[100];
  int echantillonsY[100];
  
  for(int e = 0; e < 100; e++){
    echantillonsX[e] = pulseIn(pinCanalAileronX, HIGH);
    echantillonsY[e] = pulseIn(pinCanalAileronY, HIGH);
  }
  ordonnerTableau(echantillonsX, 100);
  ordonnerTableau(echantillonsY, 100);

  milieu_min = echantillonsX[0];
  milieu_max = echantillonsX[99];
  
  Serial.print("X MIN = ");
  Serial.println(milieu_min);
  
  Serial.print("X MIN = ");
  Serial.println(milieu_max);

  milieu_min = echantillonsY[0];
  milieu_max = echantillonsY[99];
  
  Serial.print("Y MIN = ");
  Serial.println(milieu_min);
  
  Serial.print("Y MIN = ");
  Serial.println(milieu_max);
}

void ordonnerTableau(int tableau[], int tailleTableau) 
{ 
  int i,t,k = 0; 

  for(t = 1; t < tailleTableau; t++) 
  { 
    for(i=0; i < tailleTableau - 1; i++) 
    { 
      if(tableau[i] > tableau[i+1]) 
      { 
        k= tableau[i] - tableau[i+1]; 
        tableau[i] -= k; 
        tableau[i+1] += k; 
      } 
    }
  }
}*/
