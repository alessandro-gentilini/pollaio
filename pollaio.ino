/* 
 
 Il Pollaio di Cleto
 
 Autore: Alessandro alessandro.gentilini@gmail.com
 Data  : 30 maggio 2010
 
 Descrizione
 -----------
 
 Il pulsante rosso e' indicato con MANUAL_BTN.
 
 I due pulsanti neri sono indicati con OPEN_BTN e CLOSE_BTN.
 
 Se MANUAL_BTN e' premuto e si preme contemporaneamente OPEN_BTN (rispettivamente CLOSE_BTN) 
 il motore si muove nella direzione OPEN_DIR (CLOSE_DIR) fintanto che OPEN_BTN (CLOSE_BTN)
 e' premuto.
 
 Se MANUAL_BTN non e' premuto, la pressione di OPEN_BTN (rispettivamente CLOSE_BTN) muove il
 motore nella direzione OPEN_DIR (CLOSE_DIR) fino ad avere LIMIT_SWT REACHED
 se door era CLOSED (OPENED).
 
 Se MANUAL_BTN non e' premuto, e OPEN_BTN e CLOSE_BTN sono non premuti, e door e' CLOSED 
 (rispettivamente OPENED) allora sunrise==true (sunset==true) muove il motore  
 nella direzione OPEN_DIR (CLOSE_DIR) fino ad avere LIMIT_SWT REACHED. 
 
 
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
#define MANUAL_BTN 12
#define OPEN_BTN   10
#define CLOSE_BTN  9

// finecorsa
#define LIMIT_SWT 8
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

#define OPEN_DIR LOW
#define CLOSE_DIR HIGH



enum door_status_t
{
  closed,
  opened,
  unknown_status
};

enum period_t
{
  night,
  day,
  unknown_period
};

door_status_t door;
period_t period;

void setup() 
{
  pinMode( MANUAL_BTN, INPUT );  
  pinMode( OPEN_BTN  , INPUT );  
  pinMode( CLOSE_BTN , INPUT );  

  pinMode( LIMIT_SWT, INPUT );

  pinMode( MOTOR    , OUTPUT );
  pinMode( DIRECTION, OUTPUT );

  digitalWrite( MOTOR, STOP );
  digitalWrite( DIRECTION, OPEN_DIR );

  door = unknown_status;
  period = unknown_period;
  Serial.begin(9600);
}

byte open_btn;
byte close_btn;
byte manual_btn;
byte limit;

void error( char* msg )
{
  char buf[64];
  sprintf(buf,"ERRORE: %s", msg );
  Serial.println(buf);
}

void loop()
{
  delay(500);

  open_btn   = digitalRead( OPEN_BTN );
  close_btn  = digitalRead( CLOSE_BTN );
  manual_btn = digitalRead( MANUAL_BTN );

  limit = digitalRead( LIMIT_SWT );

  int light = analogRead( LIGHT_METER ); 
  if ( light < 512 ) 
  {
    period = night;
  } 
  else 
  {
    period = day;
  }

  if ( limit == REACHED ) 
  {
    digitalWrite( MOTOR, STOP );      
    int dir = digitalRead( DIRECTION );
    if ( dir == OPEN_DIR )
    {
      door = opened;
    } 
    else 
    {
      door = closed;
    }
  }

  char b[16];
  sprintf(b,"LUCE=%d",light);
  Serial.println(b);  

  switch(period)
  {
  case day: 
    Serial.println("GIORNO");
    break;
  case night: 
    Serial.println("NOTTE");
    break;
  default: 
    error("PERIODO IGNOTO");
    break;
  }

  switch(door)
  {
  case opened:  
    Serial.println("PORTA APERTA"); 
    break;
  case closed:  
    Serial.println("PORTA CHIUSA"); 
    break;
  case unknown_status: 
    Serial.println("PORTA ??????"); 
    break;
  default: 
    error("STATO PORTA INASPETTATO"); 
    break;
  }


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
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      door = unknown_status;
      Serial.println("COMANDO APERTURA ATTIVO");
      return;
    }

    if ( open_btn == RELEASED && close_btn == PRESSED )
    {
      digitalWrite( DIRECTION, CLOSE_DIR );      
      digitalWrite( MOTOR, GO );
      door = unknown_status;
      Serial.println("COMANDO CHIUSURA ATTIVO");
      return;        
    }

  } 
  else 
  {
    if ( door == closed && ( open_btn == PRESSED || period == day ) )
    {
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      door = unknown_status;
      Serial.println("INIZIATA APERTURA AUTOMATICA PORTA");
    }

    if ( door == opened && ( close_btn == PRESSED || period == night ) )
    {
      digitalWrite( DIRECTION, CLOSE_DIR );
      digitalWrite( MOTOR, GO );
      door = unknown_status;
      Serial.println("INIZIATA CHIUSURA AUTOMATICA PORTA");
    }      

  }
}





