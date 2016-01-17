/* 
 
 Il Pollaio di Cleto
 
 Autore: Alessandro alessandro.gentilini@gmail.com
 Data  : 24 luglio 2010

 */

// tempi in ms
#define MAX_OPEN_TIME 90000
#define MAX_CLOSE_TIME 90000
#define INRUSH_TIME 1000

// bottoni in ingresso
#define MANUAL_BTN 12
#define OPEN_BTN   10
#define CLOSE_BTN  9

// finecorsa
#define LIMIT_SWT 8
#define REACHED LOW

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

#define GREEN_LED 7
#define RED_LED 6




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
  
  pinMode( RED_LED, OUTPUT );
  pinMode( GREEN_LED, OUTPUT );

  digitalWrite( MOTOR, STOP );
  digitalWrite( DIRECTION, OPEN_DIR );

  s.door = unknown_status;

  s.period = unknown_period;
  s.start_open = millis();
  s.start_close = millis();
  
  Serial.begin(9600);
  Serial.println("");
  Serial.println("*** PROGRAMMA AVVIATO ***");
  Serial.println("");
  
  digitalWrite( RED_LED, HIGH );
  digitalWrite( GREEN_LED, LOW );  
}




void loop()
{
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

  if ( s.manual_btn != PRESSED )  {
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
        if ( s.manual_btn != PRESSED && digitalRead( MOTOR ) == GO && (now - s.start_open) > MAX_OPEN_TIME )
        {
          digitalWrite( MOTOR, STOP );  
          error("L'APERTURA HA IMPIEGATO TROPPO TEMPO");
        }
      }
      else
      {
        if ( s.manual_btn != PRESSED && digitalRead( MOTOR ) == GO && (now - s.start_close) > MAX_CLOSE_TIME )
        {
          digitalWrite( MOTOR, STOP );              
          error("LA CHIUSURA HA IMPIEGATO TROPPO TEMPO");
        }      
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
      
      s.start_open = millis();
      s.door = unknown_status;
      
      Serial.println("");
      Serial.println("COMANDO APERTURA ATTIVO");
      Serial.println("");      
    }

    if ( s.open_btn == RELEASED && s.close_btn == PRESSED )    
    {
      digitalWrite( DIRECTION, CLOSE_DIR );      
      digitalWrite( MOTOR, GO );
      
      s.start_close = millis();
      s.door = unknown_status;
      
      Serial.println("");
      Serial.println("COMANDO CHIUSURA ATTIVO");
    }

  }  
  else   
  {
    if ( ( s.door == closed || s.door == unknown_status ) && s.period == day )    
    {
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      if ( s.door == closed ) 
      {
        Serial.println("");
        Serial.println("APERTURA AUTOMATICA PORTA");
        Serial.println("");
        delay(INRUSH_TIME);// per dare tempo al finecorsa di disimpegnarsi
        s.start_open = millis();
      }      
      s.door = unknown_status;
    }

    if ( ( s.door == opened || s.door == unknown_status ) && s.period == night )    
    {
      digitalWrite( DIRECTION, CLOSE_DIR );
      digitalWrite( MOTOR, GO );
      if ( s.door == opened ) 
      {
        Serial.println("");
        Serial.println("CHIUSURA AUTOMATICA PORTA");        
        Serial.println("");
        delay(INRUSH_TIME);// per dare tempo al finecorsa di disimpegnarsi
        s.start_close = millis();
      }      
      s.door = unknown_status;
    }      
  }

  sp = s;
}


void error( char* msg )
{
  Serial.print("ERRORE: ");
  Serial.println(msg);
  Serial.print("RESETTARE");
  while(true){};
}

void print_status()
{
  Serial.println("-----------------------------------------");
  Serial.println("INGRESSI:");
  Serial.print("LUCE=");  Serial.print(s.light,DEC);  
  Serial.print(" ");
  switch(s.period)  
  {
  case day:   Serial.println("GIORNO");  break;
  case night: Serial.println("NOTTE");   break;
  default:     error("PERIODO IGNOTO");  break;
  }

  if ( s.open_btn == PRESSED ){Serial.println("PULSANTE APRI PREMUTO");}else{Serial.println("PULSANTE APRI NON PREMUTO");}

  if ( s.close_btn == PRESSED ){Serial.println("PULSANTE CHIUDI PREMUTO");}else{Serial.println("PULSANTE CHIUDI NON PREMUTO");}

  if ( s.manual_btn == PRESSED ){Serial.println("PULSANTE MANUALE PREMUTO");}else{Serial.println("PULSANTE MANUALE NON PREMUTO");}  

  if ( s.limit_swt == REACHED ){Serial.println("FINECORSA IMPEGNATO");}else{Serial.println("FINECORSA LIBERO");}

  Serial.println("");
  Serial.println("USCITE:");
  if ( digitalRead( MOTOR ) == GO ){Serial.println("MOTORE IN MOVIMENTO");}else{Serial.println("MOTORE FERMO");}

  if ( digitalRead( DIRECTION ) == OPEN_DIR ){Serial.println("DIREZIONE APERTURA");}else{Serial.println("DIREZIONE CHIUSURA");}

  switch(s.door)  
  {
  case opened: Serial.println("PORTA APERTA"); break;
  case closed: Serial.println("PORTA CHIUSA"); break;
  case unknown_status: Serial.println("PORTA ??????"); break;
  default: error("STATO PORTA INASPETTATO");   break;
  }

  Serial.println("-----------------------------------------");
}

