/* Il Pollaio di Cleto

*/

// bottoni in ingresso
#define MANUAL 12
#define OPEN   10
#define CLOSE  9

// fotoresistore
#define LIGHT_METER 0

// uscite
#define MOTOR 11
#define DIRECTION 13

// tempi in ms
#define FORWARD_TIME 3000
#define BACKWARD_TIME 3000

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
