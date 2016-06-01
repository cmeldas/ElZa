//čas ustálení proudu v cívce (naproudění) v ms
const unsigned long maxEnergyTime = 12000;
//čas do vypnutí cívek v klidu (v ms)
const unsigned int timeToOff = 10000;

/*
Načtení konstantní hodnoty, nebo pole hodnot
*/
//#define LOADDEFAULT_S_E
#ifdef LOADDEFAULT_S_E
// hodnota v procentech
#define DEFAULTENERGY 100
unsigned int sparkEnergy [25]; //0 = 0ot/min; 1=500ot/min...
#else     //zde zadat tabulku hodnot pro energii jiskry
unsigned int sparkEnergy [25] = {
  300,
  200,
  100,//1000
  100,
  100,//2000
  100,
  100,//3000
  100,
  100,//4000
  100,
  100,//5000
  100,
  100,//6000
  100,
  100,//7000
  100,
  100,//8000
  100,
  100,//9000
  100,
  100,//10000
  100,
  100,//11000
  100,
  100 //12000
};
#endif

// přičte tuto hodnotu k předstihu, vhodné pro ladění
float angleTunning = 0.0;

//načte jednotný předstih pro všechny otáčky nastavený v DEFAULTANGLE
#define LOADDEFAULTTIMMING
#ifdef LOADDEFAULTTIMMING
#define DEFAULTANGLE 15.0
float angleArray [60];
#else
// nutno vždy zadávat jedno desetinné místo - např.: 12.0
// používá se desetiná tečka, čárka odděluje jednotlivé položky
float angleArray [60]= {
  8.0,
  11.0,
  11.0,//400
  11.0,
  12.0,
  12.0,//1000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//2000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//3000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//4000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//5000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//6000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//7000
  12.0,
  12.0,
  12.0,
  12.0,
  12.0,//8000
  12.0,
  23.0,
  23.0,
  23.0,
  23.0,//9000
  23.0,
  23.0,
  23.0,
  23.0,
  23.0,//10000
  23.0,
  23.0,
  23.0,
  23.0,
  23.0,//11000
  23.0,
  23.0,
  23.0,
  23.0//11800
};
#endif

//korekce předstihu pro studený motor
//zvětší předstih o daný úhel při nastavené těplotě
//lineárně s rostoucí teplotou se sníží
//zakomentovat pokud chceš vypnout

//#define TEMPERATURECORRECTION
#ifdef TEMPERATURECORRECTION
//startovací teplota a zvětšený předstih pro tu teplotu
#define LOWTEMPERATUREC     20.0
#define LOWTANGLEC          3.0
//teplota od které už bude normální předstih
#define HIGHTEMPERATUREC    50.0
#endif

//změnou lze přesně odladit úhel hrany a HÚ
//úhel od hrany č. 6 k HÚ
//FALLING
#define DEFAULTEDGE1 30.0    //taky = maximální předsith
//úhel od hrany č. 13 k HÚ
//RISSING
#define DEFAULTEDGE2 30.0

//maximální otáčky pak zapalování vypne
#define MAXRPM 5000
