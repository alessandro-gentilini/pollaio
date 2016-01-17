/* 
 
 Il Pollaio di Cleto
 
 Autore: Alessandro alessandro.gentilini@gmail.com
 Data  : 30 maggio 2010
 
 Descrizione
 -----------
 
 Il pulsante rosso e' indicato con MANUAL.
 
 I due pulsanti neri sono indicati con OPEN e CLOSE.
 
 Se MANUAL e' premuto e si preme contemporaneamente OPEN (rispettivamente CLOSE) 
 il motore si muove nella direzione FORWARD (BACKWARD) fintanto che OPEN (CLOSE)
 e' premuto.
 
 Se MANUAL non e' premuto, la pressione di OPEN (rispettivamente CLOSE) muove il
 motore nella direzione FORWARD (BACKWARD) fino ad avere LIMIT REACHED
 se door era CLOSED (OPENED).
 
 Se MANUAL non e' premuto, e OPEN e CLOSE sono non premuti, e door e' CLOSED 
 (rispettivamente OPENED) allora sunrise==true (sunset==true) muove il motore  
 nella direzione FORWARD (BACKWARD) fino ad avere LIMIT REACHED. 
 
 
 Note
 ----
 
 Ho provato il programma alimentando Arduino con la porta USB (+5VDC) e fornendo 
 sui fili arancioni la +12VDC  di alimentazione del motore (i due fili esterni
 arancioni, attenzione alla polarita'). 
 I due fili arancioni interni vanno al motore.
 In questa situazione di doppia alimentazione affinche' tutto funzioni e' necessario
 collegare lo zero di riferimento della +5VDC allo zero di riferimento della +12VDC.
 Nella versione finale anche Arduino sara' alimentato a +12VDC.
 
 Collegando Arduino con USB ad un computer che abbia installati i driver FTDI
 alla USB verra' associata una porta seriale COM, con hyperterminal si puo'
 vedere l'output prodotto da Arduino collegandosi a 9600bps 8-1 none, altrimenti
 scaricare l'IDE da www.arduino.cc
 
 Provare a leggere lo stato di un pin dichiarato come output per vedere se
 si può evitare la variabile motor_is_moving.
 
 Il riconoscimento di alba e tramonto non e' corretto in tutte le situazioni: 
 attualmente funziona solamente se la transizione buio/luce (luce/buio) e'
 "repentina".
 
 L'inglese e' un po' infelice ;-)
 
 */

// tempi in ms
#define FORWARD_TIME 3000
#define BACKWARD_TIME 3000

// bottoni in ingresso
#define MANUAL 12
#define OPEN   10
#define CLOSE  9

// finecorsa
#define LIMIT 8
#define REACHED 0

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
bool motor_is_moving;
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
  motor_is_moving = false;
  Serial.begin(9600);
}

byte open_btn;
byte close_btn;
byte manual_btn;
byte limit;



void loop()
{
  open_btn = digitalRead( OPEN );
  close_btn = digitalRead( CLOSE );
  manual_btn = digitalRead( MANUAL );

  limit = digitalRead( LIMIT );

  if ( manual_btn == PRESSED )
  {
    if (   open_btn == PRESSED && close_btn == PRESSED 
      || open_btn == RELEASED && close_btn == RELEASED )
    {
      digitalWrite( MOTOR, STOP );
      motor_is_moving = false;      
      return;
    }  

    if ( open_btn == PRESSED && close_btn == RELEASED )
    {
      digitalWrite( MOTOR, GO );
      digitalWrite( DIRECTION, FORWARD );
      motor_is_moving = true;
      return;
    }

    if ( open_btn == RELEASED && close_btn == PRESSED )
    {
      digitalWrite( MOTOR, GO );
      digitalWrite( DIRECTION, BACKWARD );
      motor_is_moving = true;      
      return;        
    }

  } 
  else {
    if ( door == OPENED ) {
      Serial.println("PORTA APERTA");
    } 
    else {
      Serial.println("PORTA CHIUSA");
    }

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
      motor_is_moving = true;
    }

    if ( door == OPENED && ( close_btn == PRESSED || sunset ) )
    {
      digitalWrite( DIRECTION, BACKWARD );
      digitalWrite( MOTOR, GO );
      Serial.println("LA PORTA SI STA CHIUDENDO");
      motor_is_moving = true;
    }      

    if ( motor_is_moving && limit == REACHED ) {
      digitalWrite( MOTOR, STOP );
      if ( door == OPENED ) door = CLOSED;
      if ( door == CLOSED ) door = OPENED;
      motor_is_moving = false;
    }
  }
}

