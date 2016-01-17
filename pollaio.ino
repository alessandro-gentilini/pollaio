/* 

Il Pollaio di Cleto

Autore: Alessandro alessandro.gentilini@gmail.com
Data  : 5 maggio 2010

Descrizione
-----------

Il pulsante rosso è indicato con MANUAL.

I due pulsanti neri sono indicati con OPEN e CLOSE.

Se MANUAL è premuto e si preme contemporaneamente OPEN (rispettivamente CLOSE) 
il motore si muove nella direzione FORWARD (BACKWARD) fintanto che OPEN (CLOSE)
è premuto.

Se MANUAL non è premuto, la pressione di OPEN (rispettivamente CLOSE) muove il
motore nella direzione FORWARD (BACKWARD) per il tempo FORWARD_TIME (BACKWARD_TIME) 
se door era CLOSED (OPENED).

Se MANUAL non è premuto, e OPEN e CLOSE sono non premuti, e door è CLOSED 
(rispettivamente OPENED) allora sunrise==true (sunset==true) muove il motore  
nella direzione FORWARD (BACKWARD) per il tempo FORWARD_TIME (BACKWARD_TIME). 


Note
----

Ho provato il programma alimentando Arduino con la porta USB (+5VDC) e fornendo 
sui fili arancioni la +12VDC  di alimentazione del motore (i due fili esterni
arancioni, attenzione alla polarità). 
I due fili arancioni interni vanno al motore.
In questa situazione di doppia alimentazione affinché tutto funzioni è necessario
collegare lo zero di riferimento della +5VDC allo zero di riferimento della +12VDC.
Nella versione finale anche Arduino sarà alimentato a +12VDC.

Collegando Arduino con USB ad un computer che abbia installati i driver FTDI
la alla USB verrà associata una porta seriale COM, con hyperterminal si può
vedere l'output prodotto da Arduino collegandosi a 9600bps 8-1 none, altrimenti
scaricare l'IDE da www.arduino.cc

Mancano i fine corsa: il programma assegna alla variabile door i valori OPENED e 
CLOSED senza conoscere l'effettivo stato della porta del pollaio.

Il riconoscimento di alba e tramonto non è corretto in tutte le situazioni: 
attualmente funziona solamente se la transizione buio/luce (luce/buio) è
"repentina".

L'inglese è un po' infelice ;-)

*/

// tempi in ms
#define FORWARD_TIME 3000
#define BACKWARD_TIME 3000

// bottoni in ingresso
#define MANUAL 12
#define OPEN   10
#define CLOSE  9

// fotoresistore
#define LIGHT_METER 0

// uscite
#define MOTOR 11
#define DIRECTION 13

#define PRESSED LOW
#define RELEASED HIGH

#define GO HIGH
#define STOP LOW

#define FORWARD LOW
#define BACKWARD HIGH

#define OPENED false
#define CLOSED true

bool door;
int prev_light;

void setup() 
{
  pinMode( MANUAL, INPUT );  
  pinMode( OPEN,INPUT );  
  pinMode( CLOSE, INPUT );  
  
  pinMode( MOTOR, OUTPUT );
  pinMode( DIRECTION, OUTPUT );
  
  digitalWrite( MOTOR, STOP );
  digitalWrite( DIRECTION, FORWARD );
  
  door = CLOSED;
  prev_light = 0;
  Serial.begin(9600);
}

byte open_btn;
byte close_btn;
byte manual_btn;



void loop()
{
    open_btn = digitalRead( OPEN );
    close_btn = digitalRead( CLOSE );
    manual_btn = digitalRead( MANUAL );

    if ( manual_btn == PRESSED )
    {
      if (   open_btn == PRESSED && close_btn == PRESSED 
          || open_btn == RELEASED && close_btn == RELEASED )
      {
        digitalWrite( MOTOR, STOP );
        return;
      }  
  
      if ( open_btn == PRESSED && close_btn == RELEASED )
      {
        digitalWrite( MOTOR, GO );
        digitalWrite( DIRECTION, FORWARD );
        return;
      }
      
      if ( open_btn == RELEASED && close_btn == PRESSED )
      {
        digitalWrite( MOTOR, GO );
        digitalWrite( DIRECTION, BACKWARD );
        return;        
      }
      
    } else {
      if ( door == OPENED ) {
        Serial.println("PORTA APERTA");
      } else {
        Serial.println("PORTA CHIUSA");
      }
      digitalWrite( MOTOR, STOP );
      
      int light_now = analogRead( LIGHT_METER ); 
      char b[8];
      sprintf(b,"%d",light_now);
      Serial.println(b);
      delay(500);
      bool sunset = false;
      bool sunrise = false;
      
      if ( light_now - prev_light > 300 ) {
        sunrise = true;
        Serial.println("ALBA :-)");
      }
      
      if ( prev_light - light_now > 300 ) {
        sunset = true;
        Serial.println("TRAMONTO :-(");        
      }
      
      prev_light = light_now;
      
      if ( sunset && sunrise ) return;
      
      if ( door == CLOSED && ( open_btn == PRESSED || sunrise ) )
      {
        digitalWrite( DIRECTION, FORWARD );
        digitalWrite( MOTOR, GO );
        Serial.println("LA PORTA SI STA APRENDO");
        delay( FORWARD_TIME );
        digitalWrite( MOTOR, STOP );
        door = OPENED;
        return;
      }
      
      if ( door == OPENED && ( close_btn == PRESSED || sunset ) )
      {
        digitalWrite( DIRECTION, BACKWARD );
        digitalWrite( MOTOR, GO );
        Serial.println("LA PORTA SI STA CHIUDENDO");
        delay( BACKWARD_TIME );
        digitalWrite( MOTOR, STOP );
        door = CLOSED;
        return;
      }      
    }
}
