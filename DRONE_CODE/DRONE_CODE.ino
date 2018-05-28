#include <Servo.h> //Bibliotheque de classe destinée aux Servomoteurs
#include <Wire.h>
#include <FreeSixIMU.h>

/* Preprocesseur ramplace les mots par leurs valeurs correspondante */
#define pinCanalMoteur   6

#define pinCanalAileronX   6
#define pinCanalAileronY   7

#define piNAileronG   9
#define piNAileronD   10

#define pinServoX 9
#define pinServoY 10

#define X   0
#define Y   1
#define Z   2

/* Voici les tendances de joystick possible */
#define NA       0
#define MILIEU_X 1
#define MILIEU_Y 2
#define HAUT     3
#define BAS      4
#define GAUCHE   5
#define DROITE   6

/* Note: Il faut trouver un juste MILIEUx pour exploiter les battement */
#define D   0 // Battement max: 133 | 
#define G   1 // Battement max: 137 | mais 133 max

#define MIN   0
#define MAX   1

/* PARAMETRE JOYSTICK */
const int motorSpeed[] = { 1109, 1987 };
const int milieuX[] = { 1472, 1520 };
const int droiteX[] = { 1520, 1990 };
const int gaucheX[] = { 992, 1483 };
const int milieuY[] = { 1441, 1499 };
const int hautY[]   = { 1500, 1985 };
const int basY[]    = { 980, 1441 };

const int initPos[2] = { 94, 68 };

/* Déclaration des objets pour la manipulation des servomoteurs */
Servo srvAileron[2];
Servo srvCamera[2];
Servo esc;
int speed = 0;

int lastPosition[2];

bool tendance[8]; //Pour allouer la tendence relative des joystick

/* Position initiale des servomoteurs */
const int posInitX = 90;
const int posInitY = 90;

/* Variable pour la dernière position envoyée aux servomoteurs*/
int lastPositionX;
int lastPositionY;

ITG3200 gyro = ITG3200();
ADXL345 acc = ADXL345();

float  gx, gy, gz;
short  ax, ay, az;
int cax, cay, caz;
int cgx, cgy, cgz;

unsigned long time_1 = 0;
unsigned long time_2 = 0;

int ServoPosition;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  /*******CAMERA********/
  delay(100);
  acc.init(FIMU_ACC_ADDR);
  gyro.init(FIMU_ITG3200_DEF_ADDR); 
  Serial.print("Caibrage...");
  gyro.zeroCalibrate(500, 2); //nombre de mesures, millisecondes entre eux
  Serial.println("Terminer.");

  //Attachement des objets Servo au broche
  srvCamera[X].attach(pinServoX); 
  srvCamera[Y].attach(pinServoY);
  
  //Initialise la position des servomoteur
  srvCamera[X].write(posInitX);
  srvCamera[Y].write(posInitY);
  
  //Sauvegarde la derniere position
  lastPositionX = posInitX;
  lastPositionY = posInitY;

  /*******AILERONS********/

  pinMode(piNAileronG, INPUT);
  pinMode(piNAileronD, INPUT);

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

  /********MOTEUR BRUSHLESS********/
  esc.write(0);
  delay(1000);
  esc.write(180);
  delay(1000);
  esc.write(0);
}

void loop() {
  while(true) 
  {
    int chBrushless = pulseIn(pinCanalMoteur, HIGH);
    int chAileron[2] = { 0, 0};
    chAileron[X] = pulseIn(pinCanalAileronX, HIGH);
    chAileron[Y] = pulseIn(pinCanalAileronY, HIGH);

    int rcvSpeed = map(chBrushless,  motorSpeed[MIN],  motorSpeed[MAX], 0, 179);
    if(rcvSpeed != speed) {
      esc.write(rcvSpeed);
      speed = rcvSpeed;
    }

    aileronsManage(&chAileron[X], &chAileron[Y]);
    cameraStabilize();
  }
}

void cameraStabilize() {
  if(gyro.isRawDataReady()) {
    acc.readAccel(&ax, &ay, &az); 
    gyro.readGyro(&gx, &gy, &gz);
    
    time_1 = time_2;
    time_2 = micros();
    
    if (time_1 != 0) {
      cgx = cgx + 2 * gx * ((float)time_2 - (float)time_1) / 100000;  
      cgy = cgy + 2 * gy * ((float)time_2 - (float)time_1) / 1000000;

      cax = (2 * ax * ((float)time_2 - (float)time_1) / 100000);
      cay = (2 * ay * ((float)time_2 - (float)time_1) / 100000);
    }

    ServoPosition = 90 - cgx;
    correctVal(&ServoPosition, &cgx);
    srvCamera[X].write(ServoPosition);
    lastPositionX = ServoPosition;

    ServoPosition = 90 - cgy;
    correctVal(&ServoPosition, &cgy);
    srvCamera[Y].write(ServoPosition);
    lastPositionY = ServoPosition;
  }
}

void aileronsManage(int * chAileronX, int * chAileronY) {
  if(*chAileronX != 0 || *chAileronY != 0) {
    Serial.print("  chAileronX = ");
    Serial.println(*chAileronX);
    Serial.print("  chAileronY = ");
    Serial.println(*chAileronY);

    setJoystickTendance(*chAileronX, *chAileronY); //Met a jours le tableau "tendance" 

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

    } else if(tendance[MILIEU_Y] && tendance[DROITE]) {
      //Droite seulement
      psD = map(chAileronX, droiteX[MIN],  droiteX[MAX], initPos[D], 6);
      //BETA INVERSION
      psG = map(chAileronX, droiteX[MIN],  droiteX[MAX], 27, initPos[G]);
      Serial.println("  - Droite seulement");

    } else if(tendance[MILIEU_Y] && tendance[GAUCHE]) {
      //Gauche seulement
      psG = map(chAileronX, gaucheX[MIN],  gaucheX[MAX], 159, initPos[G]);
      //BETA INVERSION
      psD = map(chAileronX, gaucheX[MIN],  gaucheX[MAX], 138, initPos[D]);
      Serial.println("  - Gauche seulement");
      
    } 
    /*
    else if(tendance[MILIEU_X] && tendance[HAUT]) {
      //Haut seulement
      psD = map(chAileronY,  1425,  1920, initPos[G], 159);
      psG = map(chAileronY,  1425,  1920, initPos[D], 6);
      Serial.println("  - Haut seulement");
      
    } else if(tendance[MILIEU_X] && tendance[BAS]) {
      //Bas seulement
      psD = map(chAileronY,  993,  1375, initPos[G], 138);
      psG = map(chAileronY,  993,  1375, initPos[D], 27);
      Serial.println("  - Bas seulement");
      
    } else if(tendance[HAUT] && tendance[DROITE]) {
      //Haut droite
      psD = map(chAileronX,  1523,  1980, initPos[D], 27);
      psG = map(chAileronY,  1425,  1920, initPos[D], 27);
      Serial.println("  - Haut droite");
      
    } else if (tendance[HAUT] && tendance[GAUCHE]) {
      //Haut gauche
      psD = map(chAileronY,  1425,  1920, 162, initPos[G]);
      psG = map(chAileronX,  992,  1439, 162, initPos[G] );
      Serial.println("  - Haut gauche");
      
    } else if (tendance[BAS] && tendance[DROITE]) {
      //Bas droite
      psD = map(chAileronX,  1523,  1980, initPos[D], 27);
      psG = map(chAileronY,  993,  1375, initPos[D], 27);
      Serial.println("  - Bas droite");
      
    } else if (tendance[BAS] && tendance[GAUCHE]) {
      //Bas gauche
      psD = map(chAileronY,  993,  1375, 162, initPos[G]);
      psG = map(chAileronX,  992,  1439, 162, initPos[G] );
      Serial.println("  - Bas gauche");

    } else {
      Serial.println("-  Erreur");
      psD = initPos[D];
      psG = initPos[G];
    }*/

    srvAileron[D].write(psD);
    srvAileron[G].write(psG);
    
  }
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

void correctVal(int * srvPos, int * val) {
  if(*srvPos < 0){
    *srvPos = 0;
    *val = 90;
  } if(*srvPos > 180){
    *srvPos = 180;
    *val = -90;
  }
}
