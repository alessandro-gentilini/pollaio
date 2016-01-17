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
#define MAX_OPEN_TIME 3000
#define MAX_CLOSE_TIME 3000

// bottoni in ingresso
#define MANUAL_BTN 12
#define OPEN_BTN   10
#define CLOSE_BTN  9

// finecorsa
#define LIMIT_SWT 8
#define REACHED 1

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

class Status{
public:
  door_status_t door;
  period_t period;

  byte open_btn;
  byte close_btn;
  byte manual_btn;
  byte limit_swt;

  int light;

  unsigned long start_open;
  unsigned long start_close;

  bool operator!= ( const Status& rhs )
  {
    bool same = door==rhs.door && period==rhs.period && open_btn==rhs.open_btn && close_btn==rhs.close_btn 
      && manual_btn==rhs.manual_btn && limit_swt==rhs.limit_swt;
    return !same;
  }

//  void toString() const
//  {
//    Serial.print("\n");
//    Serial.print(door,DEC);
//    Serial.print(period,DEC);
//    Serial.print(open_btn,DEC);
//    Serial.print(close_btn,DEC);
//    Serial.print(manual_btn,DEC);
//    Serial.print(limit_swt,DEC);
//    Serial.print("\n");
//  }
};

Status s,sp;

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

  s.door = unknown_status;
  s.period = unknown_period;
  Serial.begin(9600);
}



void error( char* msg )
{
  Serial.print("ERRORE: ");
  Serial.println(msg);
}

void print_status()
{
  static char e1[]="FINECORSA IMPEGNATO E MOTORE IN MOVIMENTO";
  static char e2[]="FINECORSA LIBERO E MOTORE FERMO";
  char * error_str = 0;

  Serial.println("-----------------------------------------");
  Serial.println("INGRESSI:");
  Serial.print("LUCE=");
  Serial.print(s.light);  
  Serial.print(" ");
  switch(s.period)  
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

  if ( s.open_btn == PRESSED )
  {
    Serial.println("PULSANTE APRI PREMUTO");
  }
  else
  {
    Serial.println("PULSANTE APRI NON PREMUTO");    
  }

  if ( s.close_btn == PRESSED )
  {
    Serial.println("PULSANTE CHIUDI PREMUTO");
  }
  else
  {
    Serial.println("PULSANTE CHIUDI NON PREMUTO");    
  }

  if ( s.manual_btn == PRESSED )
  {
    Serial.println("PULSANTE MANUALE PREMUTO");
  }
  else
  {
    Serial.println("PULSANTE MANUALE NON PREMUTO");    
  }  

  if ( s.limit_swt == REACHED )  
  {
    if ( digitalRead( MOTOR ) == GO ) 
    {
      error_str = e1;
    }
    Serial.println("FINECORSA IMPEGNATO");
  }  
  else  
  {
    if ( digitalRead( MOTOR ) == STOP ) 
    {
      error_str = e2;
    }    
    Serial.println("FINECORSA LIBERO");
  }

  Serial.println("\nUSCITE:");
  if ( digitalRead( MOTOR ) == GO )
  {
    Serial.println("MOTORE IN MOVIMENTO");
  }
  else
  {
    Serial.println("MOTORE FERMO");
  }

  if ( digitalRead( DIRECTION ) == OPEN_DIR )
  {
    Serial.println("DIREZIONE APERTURA");
  }
  else
  {
    Serial.println("DIREZIONE CHIUSURA");
  }

  switch(s.door)  
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

  if ( error_str ) 
  {
    Serial.println("\nERRORI");
    error( error_str );
  }
  Serial.println("-----------------------------------------");
}

void loop()
{
  delay(500);

  s.open_btn   = digitalRead( OPEN_BTN );
  s.close_btn  = digitalRead( CLOSE_BTN );
  s.manual_btn = digitalRead( MANUAL_BTN );

  s.limit_swt = digitalRead( LIMIT_SWT );

  s.light = analogRead( LIGHT_METER ); 

  if ( s.light < 512 )   
  {
    s.period = night;
  }   
  else   
  {
    s.period = day;
  }

  if ( s.limit_swt == REACHED )   
  {
    digitalWrite( MOTOR, STOP );      
    if ( digitalRead( DIRECTION ) == OPEN_DIR )    
    {
      s.door = opened;
    }     
    else     
    {
      s.door = closed;
    }
  }
  else
  {
    unsigned long now = millis();
    if ( digitalRead( DIRECTION ) == OPEN_DIR )
    {
      if ( digitalRead( MOTOR ) == GO && (now - s.start_open) > MAX_OPEN_TIME )
      {
        digitalWrite( MOTOR, STOP );              
        error("L'APERTURA HA IMPIEGATO TROPPO TEMPO");
      }
    }
    else
    {
      if ( digitalRead( MOTOR ) == GO && (now - s.start_close) > MAX_CLOSE_TIME )
      {
        digitalWrite( MOTOR, STOP );              
        error("LA CHIUSURA HA IMPIEGATO TROPPO TEMPO");
      }      
    }
  }

  if ( s != sp ) 
  {
    print_status();
  }

  if ( s.manual_btn == PRESSED )  
  {
    if (   s.open_btn == PRESSED && s.close_btn == PRESSED 
      || s.open_btn == RELEASED && s.close_btn == RELEASED )    
    {
      digitalWrite( MOTOR, STOP );
    }  

    if ( s.open_btn == PRESSED && s.close_btn == RELEASED )    
    {
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      s.door = unknown_status;
      Serial.println("\nCOMANDO APERTURA ATTIVO\n");
    }

    if ( s.open_btn == RELEASED && s.close_btn == PRESSED )    
    {
      digitalWrite( DIRECTION, CLOSE_DIR );      
      digitalWrite( MOTOR, GO );
      s.door = unknown_status;
      Serial.println("\nCOMANDO CHIUSURA ATTIVO\n");
    }

  }  
  else   
  {
    if ( ( s.door == closed || s.door == unknown_status ) && ( s.open_btn == PRESSED || s.period == day ) )    
    {
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      if ( s.door == closed ) 
      {
        s.start_open = millis();
      }      
      s.door = unknown_status;
      Serial.println("\nAPERTURA AUTOMATICA PORTA IN CORSO\n");
    }

    if ( ( s.door == opened || s.door == unknown_status ) && ( s.close_btn == PRESSED || s.period == night ) )    
    {
      digitalWrite( DIRECTION, CLOSE_DIR );
      digitalWrite( MOTOR, GO );
      if ( s.door == opened ) 
      {
        s.start_close = millis();
      }      
      s.door = unknown_status;
      Serial.println("\nCHIUSURA AUTOMATICA PORTA IN CORSO\n");
    }      
  }

  sp = s;
}





















