/* 
 
 Il Pollaio di Cleto
 
 Autore: Alessandro alessandro.gentilini@gmail.com
 Data  : 4 agosto 2010

 */
 
#define SER_DBG_PRINT 1 // commentare la riga per non avere i print su seriale

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

#define OPEN_DIR HIGH
#define CLOSE_DIR LOW

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

  int light;

  unsigned long start_open;
  unsigned long start_close;

  bool operator!= ( const Status& rhs )
  {
    bool same = door==rhs.door && period==rhs.period && open_btn==rhs.open_btn && close_btn==rhs.close_btn 
      && manual_btn==rhs.manual_btn;
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
  
  #ifdef SER_DBG_PRINT
  Serial.begin(9600);
  Serial.println("");
  Serial.println("*** PROGRAMMA AVVIATO ***");
  Serial.println("");
  #endif
  
  digitalWrite( RED_LED, HIGH );
  digitalWrite( GREEN_LED, LOW );  
}




void loop()
{
  digitalWrite( RED_LED, HIGH );//se resta acceso indica hang
  
  s.open_btn   = digitalRead( OPEN_BTN );
  s.close_btn  = digitalRead( CLOSE_BTN );
  s.manual_btn = digitalRead( MANUAL_BTN );



  s.light = analogRead( LIGHT_METER ); 

  if ( s.light < 50 )   
  {
    s.period = night;
  }   
  else   
  {
    s.period = day;
  }

  if ( s.manual_btn != PRESSED )  
  {
      unsigned long now = millis();
      if ( digitalRead( DIRECTION ) == OPEN_DIR )
      {
        if ( s.manual_btn != PRESSED && digitalRead( MOTOR ) == GO && (now - s.start_open) > MAX_OPEN_TIME )
        {
          digitalWrite( MOTOR, STOP );  
          digitalWrite( DIRECTION, CLOSE_DIR );// bobina rele' non eccitata
          s.door = opened;
        }
      }
      else
      {
        if ( s.manual_btn != PRESSED && digitalRead( MOTOR ) == GO && (now - s.start_close) > MAX_CLOSE_TIME )
        {
          digitalWrite( MOTOR, STOP );       
          digitalWrite( DIRECTION, CLOSE_DIR );// bobina rele' non eccitata       
          s.door = closed;          
        }
      }
  }

  #ifdef SER_DBG_PRINT
  if ( s != sp ) 
  {
    print_status();
  }
  #endif

  if ( s.manual_btn == PRESSED )  
  {
    if (   s.open_btn == PRESSED && s.close_btn == PRESSED 
      || s.open_btn == RELEASED && s.close_btn == RELEASED )    
    {
      digitalWrite( MOTOR, STOP );
      digitalWrite( DIRECTION, CLOSE_DIR );// bobina rele' non eccitata
    }  

    if ( s.open_btn == PRESSED && s.close_btn == RELEASED )    
    {
      digitalWrite( DIRECTION, OPEN_DIR );
      digitalWrite( MOTOR, GO );
      
      s.start_open = millis();
      s.door = unknown_status;

      #ifdef SER_DBG_PRINT      
      Serial.println("");
      Serial.println("COMANDO APERTURA ATTIVO");
      Serial.println("");      
      #endif
    }

    if ( s.open_btn == RELEASED && s.close_btn == PRESSED )    
    {
      digitalWrite( DIRECTION, CLOSE_DIR );      
      digitalWrite( MOTOR, GO );
      
      s.start_close = millis();
      s.door = unknown_status;
      
      #ifdef SER_DBG_PRINT
      Serial.println("");
      Serial.println("COMANDO CHIUSURA ATTIVO");
      #endif
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
        #ifdef SER_DBG_PRINT
        Serial.println("");
        Serial.println("APERTURA AUTOMATICA PORTA");
        Serial.println("");
        #endif
        
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
        #ifdef SER_DBG_PRINT
        Serial.println("");
        Serial.println("CHIUSURA AUTOMATICA PORTA");        
        Serial.println("");
        #endif
        
        delay(INRUSH_TIME);// per dare tempo al finecorsa di disimpegnarsi
        s.start_close = millis();
      }      
      s.door = unknown_status;
    }      
  }

  sp = s;
  
  digitalWrite( RED_LED, LOW );// se non spengo sono in hang
}


void error( char* msg )
{
  #ifdef SER_DBG_PRINT
  Serial.print("ERRORE: ");
  Serial.println(msg);
  Serial.print("RESETTARE");
  #endif
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

