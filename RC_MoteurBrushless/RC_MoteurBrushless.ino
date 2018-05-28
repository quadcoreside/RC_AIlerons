#include <Servo.h>

Servo esc;   // Création de l'objet permettant le contrôle de l'ESC
int val = 0;

#define pinCanalMoteur   6

void setup() {
   esc.attach(9); // On attache l'ESC au port numérique 9 (port PWM obligatoire)
   delay(15);
   Serial.begin(9600);
 
   esc.write(0);
   delay(1000);
   esc.write(180);
   delay(1000);
   esc.write(0);
 
   Serial.println("Nombre 0 et 179");
}
 
void loop() {
    /*int chMoteur = pulseIn(pinCanalMoteur, HIGH);
    Serial.print("PulseIn = ");
    Serial.println(chMoteur);
    int val = map(chMoteur,  1109,  1987, 0, 179);
    Serial.print("Val = ");
    Serial.println(val);

    esc.write(val);*/

    if (Serial.available() > 0) {
      val = Serial.parseInt();   // lecture de la valeur passée par le port série
      Serial.println(val);
      esc.write(val);            // 
      delay(15);
      }
}
