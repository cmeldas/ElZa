
unsigned int rpm();
unsigned long getEnergy (unsigned int otacky);
void coil1On (unsigned int otacky);
void coil2On (unsigned int otacky);
void coilOff (unsigned int usTimming);
void loadPosition (bool ano);
void intFalling();
void intRissing();
void intFallingDeb(unsigned long actualT);
void intRissingDeb(unsigned long actualT);


void loadPosition (bool s=true){
  if(s){
    start = 1;
    numEdge = 0;
    speedStart = 0;
    c1wait = true;
    c2wait= true;
    led = true;
  }
}

void intFalling (){
  unsigned long actualT = micros();
  delayMicroseconds(DEBMICROS);
  if((PIND & (1<<PIND3))){
    intFallingDeb(actualT);
  }
  else{
    //  digitalWrite(LED_R,HIGH);
  }
}

void intRissing (){
  unsigned long actualT = micros();
  delayMicroseconds(DEBMICROS);
  if(!(PIND & (1<<PIND2))){
    intRissingDeb(actualT);
  }
  else{ //zde to často zlobí
    //digitalWrite(LED_R,HIGH);
  }
}


void intFallingDeb(unsigned long actualT) {
  static unsigned long usTimming;
  if (lastEdge) {
    lastEdge = false;
  }
  else  { //zde to zlobí
    digitalWrite(LED_R,HIGH);
    return;
    //loadPosition(true);
  }

  switch (start) {
    case 0: //
    switch (point) {
      case 5:
      time[6]=actualT;
      point = 6;
      coilOff(usTimming);
      break;
      case 13:
      time[14]=time[0];//uloží minulou hodnotu pro zjištění otáček
      time[0]=actualT;
      point = 0;
      unsigned int ot;
      ot = rpm();
      if(ot>CHECKMINRPM){
        if((unsigned long)(10*(time[0]-time[13]))>(mult2*(time[13]-time[12]))){
          if (ot<MAXRPM) coil2On(ot);
        }
        else {
          digitalWrite(LED_R,HIGH);
          loadPosition(true);
          return;
        }
      }
      else{
        if(speedStart==2){
          speedStart=3;
          coil1On(ot);
        }
        coil2On(ot);
      }
      break;
      case 1:
      time[2]=actualT;
      point = 2 ;
      break;
      case 3:
      time[4]=actualT;
      point = 4;
      usTimming = (getTimming(1,rpm()));
      break;
      case 7:
      time[8]=actualT;
      point = 8;
      break;
      case 9:
      time[10]=actualT;
      point = 10;
      break;
      case 11:
      time[12]=actualT;
      point = 12;
      rissingCorrection = ((time[12]-time[10])/2)/(time[11]-time[10]);
      break;
    }
    break;

    case 1: //čeká na 2 hrany
    time[12]=time[0];
    time[0]=actualT;
    time[6]=actualT;
    numEdge++;
    if (numEdge>2) {
      start = 2;
    }
    break;

    case 2: //hledá polohu
    time[12]=time[0];
    time[0]= actualT;
    time[6]=actualT;
    if((unsigned long)((time[0]-time[13])*10)>(multS2*(time[13]-time[12]))){
      start = 0;
      point = 0;
      speedStart=1;
      coil2On(rpm());
    }
    break;
  }
  lastTime = millis();
}

void intRissingDeb(unsigned long actualT) {
  static unsigned long usTimming;
  if (!lastEdge) {
    lastEdge=true;
  }
  else  {
    digitalWrite(LED_R,HIGH);
    return;
    //loadPosition(true);
  }


  switch (start) {
    case 0: //normální běh

    switch (point) {
      case 12:
      time[13]=actualT;
      point = 13;
      rissingCorrection =(time[13]-time[12])*(rissingCorrection-1);
      time[13]=time[13]+rissingCorrection;
      coilOff(usTimming+rissingCorrection);
      break;
      case 0:
      time[1]=actualT;
      point = 1;
      break;
      case 2:
      time[3]=actualT;
      point = 3;
      break;
      case 4:
      time[5]=actualT;
      point = 5;
      break;
      case 6:
      time[15]=time[7];
      time[7]=actualT;
      point = 7;
      unsigned int ot;
      ot = rpm();
      if (ot<MAXRPM) coil1On(ot);
      if(speedStart==1){
        speedStart=3;
        coil2On(ot);
      }
      break;
      case 8:
      time[9]=actualT;
      point = 9;
      break;
      case 10:
      time[11]=actualT;
      point =11;
      usTimming = (getTimming(2,rpm())-DEBMICROS);
      break;
    }
    break;

    case 1: //čeká na 3 hrany
    time[13]=actualT;
    time[5]=time[7];
    time[7]=actualT;
    numEdge++;
    if (numEdge>2) {
      start = 2;
    }
    break;

    case 2: //začíná hleda polohu
    time[14]=actualT;
    time[5]=time[7];
    time[7]=actualT;
    if((unsigned long)((time[7]-time[6])*10)>(multS1*(time[6]-time[5]))){
      start = 0;
      point = 7;
      speedStart=2;
      coil1On(rpm());
    }
    break;
  }
  lastTime = millis();
}


void coil1On (unsigned int ot){
  if(ot<150){
    coil1OnT = micros();
    c1wait = false;
    return;
  }
  unsigned long waitT =((unsigned long)(50000000/ot)+getTimming(1,ot));
  unsigned long energyT =getEnergy(ot);
  if(waitT>energyT){
    coil1OnT = micros()+waitT-energyT;

  }
  else{
    coil1OnT = micros();
  }
  c1wait = false;
}

void coil2On (unsigned int ot){
  //nastaví coil1OnT
  if(ot<150){ //při nízkých otáčkách může být načasování problém
    coil2OnT = micros();
    c2wait = false;
    return;
  }
  unsigned long waitT =((unsigned long)(50000000/ot)+getTimming(2,ot));
  unsigned long energyT =getEnergy(ot);
  if(waitT>energyT){
    coil2OnT = micros()+waitT-energyT;
  }
  else{
    coil2OnT = micros();
  }
  c2wait = false;
}

unsigned int rpm (){
  unsigned int ot;
  switch (speedStart) {
    case 3:
    if(point<7){
      ot =60000000/((time[0]-time[14]));
    }
    else
    {
      ot =60000000/((time[7]-time[15]));
    }
    return ot ;
    break;
    case 0:
    return 0;
    break;
    case 1:
    switch(point){
      case 4:
      ot =6000000000/((time[4]-time[12])*225);
      break;
      case 5:
      ot =6000000000/((time[5]-time[13])*225);
      break;
      case 0:
      ot =600000000/((time[0]-time[12])*45);
      break;
      default:
      ot = 4*minRpm;
      break;
    }
    break;
    case 2:
    switch (point){
      case 11:
      ot =6000000000/((time[11]-time[5])*225);
      break;
      case 12:
      ot =6000000000/((time[12]-time[6])*225);
      break;
      case 7:
      ot =600000000/((time[7]-time[5])*45);
      break;
      default:
      ot = 4*minRpm;
      break;
    }
    break;

  }
  return ot ;
}

unsigned long getEnergy(unsigned int ot){
  unsigned int x;
  unsigned long energy;
  if(ot>12000) {
    energy = (maxEnergyTime * sparkEnergy[24])/100;
    return energy;
  }
  x=ot/500;
  energy = map(ot,(x*500),((x+1)*500),sparkEnergy[x],sparkEnergy[x+1]);
  energy = (maxEnergyTime * energy)/100;
  return energy;
}

void coilOff(unsigned int usTimming){
  usTimming=((usTimming)*2)-1;
  //aby náhodou nepřetekl
  //pro potřebu většího předstihu předělat prescaller
  if(usTimming>65534) usTimming = 65534;
  cli();
  TCCR1A = 0;// reset všeho na 0
  TCCR1B = 0;
  TCNT1  = 0;//initialize counter
  OCR1A = usTimming; //čas -1
  TCCR1B |= (1 << WGM12); //Prescaller 8, počítá po 0,5us
  TCCR1B |= (1 << CS11) ;
  // zapne timer interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect){
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  switch (point) {
    case 13:
    digitalWrite(CIVKA2P,LOW);
    break;
    case 6:
    digitalWrite(CIVKA1P,LOW);
    break;
    case 7:
    digitalWrite(CIVKA1P,LOW);
    break;
    case 0:
    digitalWrite(CIVKA2P,LOW);
    break;
  }
}
