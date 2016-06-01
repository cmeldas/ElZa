#include <Wire.h>
#include <math.h>

//globální proměnné
unsigned int point = 0;
unsigned int speedStart = 0;
unsigned int start = 1;   //0 - normal run; 1 - loading min 3 edges; 2 - start
unsigned long lastTime = 0;
bool lastEdge = false;
//číslo poslední hrany na kotouči
unsigned int numEdge = 0;     //počítá počet hran při hledání polohy
//s otákama se snižuje
unsigned int multS1 = 18;
//s korekcí
unsigned int mult2 = 22;
//při startu bez korekce
unsigned int multS2= 25;
unsigned long timmingArray1 [60];    //uložena doba předstihu od hrany
unsigned long timmingArray2 [60];
unsigned long time [16];
//čas kdy se má sepnout cívky
unsigned long coil1OnT;
unsigned long coil2OnT;
bool c1wait = true;
bool c2wait = true;
unsigned long loopUpdateL = 0;
unsigned long rissingCorrection = 0;
unsigned long toSlowT = 100;
unsigned long LEDUpdateL= 0;
bool led = false;
// rychlost blikání při přehřátí
const unsigned long LEDUpdateT = 400;
// rychlost blikání při zastavení
const unsigned long LEDstop = 100;

// prototypy funkcí
void reloadTimming ();
unsigned long getTimming(unsigned int civka, unsigned int otacky);
int getTemperature1 ();
int getTemperature2 ();
float getVoltage();
float angleCorrection(unsigned int ot200);

//my files
#include "pins.h"
#include "configure.h"
#include "configure_adv.h"
#include "Interrupt_fce.h"

//---------------------------SETUP--------
void setup(){
  #ifdef SERIAL_D
  Serial.begin(SERIALBAUDRATE);
  Serial.print("Elza");
  Serial.print("\n");
  Serial.print("\n");
  #endif
  //nepoužívané piny sepne na LOW aby se snížilo rušení do procesoru
  for (int i = 0; i < (sizeof(unussedPin)/sizeof(int)); i++) {
    pinMode(unussedPin[i],OUTPUT);
    digitalWrite(unussedPin[i],LOW);
  }
  pinMode(A2,OUTPUT);
  digitalWrite(A2, LOW);
  pinMode(A4,OUTPUT);
  digitalWrite(A4, LOW);
  pinMode(A5,OUTPUT);
  digitalWrite(A5, LOW);
  pinMode(TL1_P,INPUT_PULLUP);
  pinMode(TL2_P,INPUT_PULLUP);
  pinMode(LED_R,OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(CIVKA1P,OUTPUT);
  pinMode(CIVKA2P,OUTPUT);
  digitalWrite(LED_R,LOW);
  digitalWrite(LED_G,HIGH);

  //výpočet minimální doby mezi pulzy z čidla z minimálních otáček
  toSlowT = (float)((1000.0/((float)(minRpm) / 60.0))/3.0);

  noInterrupts();
  pinMode(INT2_P,INPUT_PULLUP);
  pinMode(INT1_P,INPUT_PULLUP);

  //interupt na vzestupnou hranu napětí, to odpovídá fyzicky sestupné hraně na kotouči
  attachInterrupt(digitalPinToInterrupt(INT1_P), intFalling, RISING);
  //interupt na sestupnou hranu napětí, to odpovídá fyzicky vzestupné hraně na kotouči
  attachInterrupt(digitalPinToInterrupt(INT2_P), intRissing, FALLING);

  //vyplní pole hodnot nastavenou hodnotou
  #ifdef LOADDEFAULT_S_E
  for (int i = 0; i < (24); i++) {      // i <sizeof(a) / sizeof(unsigned int);
    sparkEnergy[i]=DEFAULTENERGY;
  }
  #endif
  #ifdef LOADDEFAULTTIMMING
  for (int i = 0; i < 60; i++) {
    angleArray[i]= DEFAULTANGLE;
  }
  #endif

  angleCorrection(0);
  reloadTimming();
  digitalWrite(LED_R, LOW);
  interrupts();
}

//---------------------------LOOP----------------------


void loop (){
  static float temp;
  if(lastTime + toSlowT < millis()){
    //pokud se motor přestane toit zane program hledat polohu
    loadPosition(true);
    digitalWrite(LED_G,led);
    if(lastTime + timeToOff < millis()){
      //vypne cívky pokud se motor přestane točit
      digitalWrite(CIVKA1P, LOW);
      digitalWrite(CIVKA2P, LOW);
    }
  }

  if (rpm()>MAXRPM) {
    c1wait = true;
    c2wait = true;
  }
  // nezapne cívku pokud je neznámá poloha, nebo jsou překročeny max otáčky
  if(start<1){// sepnutí cívek
    if(!c1wait ){
      if(coil1OnT<micros()){
        digitalWrite(CIVKA1P,HIGH);
        c1wait = true;
      }
    }
    if(!c2wait){
      if(coil2OnT<micros()){
        digitalWrite(CIVKA2P,HIGH);
        c2wait = true;
      }
    }
  }

  if (point > 12 ) reloadTimming();
  //zelená led svítí když motor stojí a je vše ok
  //zhasne když motor běží
  if(point < 2){ //načte teplotu jednou za čas, čtení analog vstupu trvá
    temp = getTemperature1();
  }

  if(temp>WARNINGTEMPERATURE){
    if(millis()>(LEDUpdateL+LEDUpdateT)){
      led=!led;
      LEDUpdateL = millis();
    }
    //při přehřátí vypne zapalování
    if(temp>STOPTEMPERATURE){
      while (1) {
        noInterrupts();
        loadPosition(1);
        delay(LEDstop);
        digitalWrite(LED_G,LOW);
        delay(LEDstop);
      }
    }
  }
  else{
    if(start>0) led= true;
    else led = false;
  }

  digitalWrite(LED_G,led);


  if(millis()>(loopUpdateL+loopUpdateT)){
    #ifdef SERIAL_D
    if(point<7){
      Serial.print("Otacky: ");
      Serial.print(rpm());
      Serial.print("\n");

      Serial.print("\n");
      Serial.print("\n");
    }
    loopUpdateL = millis();
    #endif
    digitalWrite(LED_R,LOW);
  }


}

unsigned long getTimming (unsigned int civka, unsigned int ot){
  unsigned long timming;
  unsigned int x;
  switch (civka) {
    case 1:
    if (ot>11800) {
      return timmingArray1[60];
    }
    x = ot/200;
    timming = map(ot,(x*200),((x+1)*200),timmingArray1[x],timmingArray1[x+1]);
    break;
    case 2:
    if (ot>11800) {
      return timmingArray2[60];
    }
    x = ot/200;
    timming = map(ot,(x*200),((x+1)*200),timmingArray2[x],timmingArray2[x+1]);
    break;
  }
  return timming;
}

void reloadTimming (){
  /*
  přepočítá úhel na microsekundy od hrany
  angleArray [] -> timmingArray[];
  */
  float angle1= DEFAULTEDGE1-(angleArray[1]+angleCorrection(1));
  float angle2 = DEFAULTEDGE2-(angleArray[1]+angleCorrection(1));
  timmingArray1[0]=(float)(((60000000.0*(angle1)))/((float)minRpm*360.0));
  timmingArray2[0]=(float)(((60000000.0*(angle2)))/((float)minRpm*360.0));
  for (int i = 1; i < 60; i++) {
    angle1= DEFAULTEDGE1-(angleArray[i]+angleCorrection(i));
    if(angle1<0.0) angle1 = 0.0;
    timmingArray1 [i] =(float)(((60000000.0*(angle1)))/(i*200.0*360.0));
    if(timmingArray1[i]<(INTDELAY)){
      timmingArray1[i]=1;
    }
    else{
      timmingArray1[i] = timmingArray1[i] - INTDELAY;
    }
    if(timmingArray1[i]>MAXTIMMINGT){
      timmingArray1[i]=MAXTIMMINGT;
    }

  }
  for (int i = 1; i < 60; i++) {
    angle2 = DEFAULTEDGE2-(angleArray[i]+angleCorrection(i));
    if(angle2<0.0) angle2 = 0.0;
    timmingArray2 [i] =(float)((60000000.0*(angle2))/((i*200.0)*360.0));
    if(timmingArray2[i]<(INTDELAY)){
      timmingArray2[i]=1;
    }
    else{
      timmingArray2[i] = timmingArray2[i] - INTDELAY;
    }
    if(timmingArray2[i]>MAXTIMMINGT){
      timmingArray2[i]=MAXTIMMINGT;
    }
  }
}

float angleCorrection (unsigned int i) {
  #ifdef TEMPERATURECORRECTION
  static float b ;
  static float k;
  #endif
  static float kc;
  unsigned int ot  = 200*i;
  float angleC = 0.0;
  //přičáítá hodnotu v ° k předstihu
  if(i<1){
    //vypočítá pouze pokud se zavolá funkce když motor neběží, tedy v setup()
    //toto staí zavolat pouze jednou
    #ifdef TEMPERATURECORRECTION
    static float b = LOWTANGLEC /(1-(LOWTEMPERATUREC/HIGHTEMPERATUREC));
    static float k = (-b)/HIGHTEMPERATUREC;
    #endif
    kc = ANGLECORRECTION / RPMCORRECTION;
    return 0.0;
  }
  //korekce podle teploty
  #ifdef TEMPERATURECORRECTION
  int temp = getTemperature2();
  if(temp<HIGHTEMPERATUREC){
    angleC = k* temp+b;
  }
  #endif
  //korekce chyby snímače na sestupné hraně
  //sestupná hrana má s otáčkami spoždění
  angleC= angleC + (kc* ot)+ angleTunning;
  return angleC;
}


int getTemperature1(){ //teplota MOSFETU
  //vypočítá teplotu, vzorec z datascheet
  //return 20; //zakomentovat při použití termistoru
  float u = analogRead(NTC1_P)*(5.0/1023.0);
  float r =((4700.0*5.0)/u)-4700.0;
  float t = pow(((A1_1)+(B1_1*log(r/R25_1))+(C1_1*pow(log(r/R25_1),2))+(D1_1*pow(log(r/R25_1),3))),-1);
  t = t - 273.15;
  if (t>BADTERMISTOR){
    return 0;
  }
  return t;
}

int getTemperature2(){
  //return 20; //zakomentovat při použití termistoru
  //vypočítá teplotu, vzorec z datascheet
  float u = analogRead(NTC2_P)*(5.0/1023.0);
  float r =((4700.0*5.0)/u)-4700.0;
  float t = pow(((A1_2)+(B1_2*log(r/R25_2))+(C1_2*pow(log(r/R25_2),2))+(D1_2*pow(log(r/R25_2),3))),-1);
  t = t - 273.15;
  if (t>BADTERMISTOR){
    #ifdef TEMPERATURECORRECTION
    return HIGHTEMPERATUREC;
    #endif
    return 0;
  }
  return t;
}

float getVoltage (){
  float u = analogRead(VOLTAGE_P)*(5.0/1023.0);
  u = u /0.32;
  return u;
}
