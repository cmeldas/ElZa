//korekce zpoždění sestupné hrany
//vytvoří lineární fce z nuly do níže nastaveného bodu a přičte k předstihu
#define RPMCORRECTION   6000
#define ANGLECORRECTION 5

//zakomentováním vypne vypisování na serial
//#define SERIAL_D
#ifdef SERIAL_D
#define SERIALBAUDRATE 9600
#endif
//rychlost vypisování na serial
const unsigned long loopUpdateT = 500;

//Parametry termistoru
//Termistor na desce
#define B25_1 4190
#define R25_1 100000
#define A1_1 0.003354016
#define B1_1 0.0002460382
#define C1_1 0.000001034240
#define D1_1 0.0000001034240
//Termistor na motoru
#define B25_2 4190
#define R25_2 100000
#define A1_2 0.003354016
#define B1_2 0.0002460382
#define C1_2 0.000001034240
#define D1_2 0.0000001034240

//teplota kdy zane blikat zelená LED
#define WARNINGTEMPERATURE  90
//teplota kdy se zastaví motor, zelená LED bliká extrémně
#define STOPTEMPERATURE     110

//hodnota teploty, která bude vyhodnocena jako chyba, např upadlé čidlo
#define BADTERMISTOR 250
//debounce time
#define DEBMICROS 10
#define INTDELAY 30 //us zpoždění vstup na výstup
//max hodnota je 32700 (jinak přeteče timer1)
#define MAXTIMMINGT 32768
const unsigned int minRpm = 50;
#define CHECKMINRPM 200
