/* 
 
 Il Pollaio di Cleto
 
 Autori: Alessandro alessandro.gentilini@gmail.com, Daniele C.
 Data  : 6 agosto 2010

*/

#include <Narcoleptic.h>
 
// #define SER_DBG_PRINT // commentare la riga per non avere i print su seriale
#define ENABLE_VALIDATION // commentare la riga per non avere il meccanismo di validazione giorno notte

// tempi in ms
#define TIMEOUT_COMMAND     130000
#define TIMEOUT_VALIDATION  300000

// bottoni in ingresso
#define MANUAL_BTN 12
#define OPEN_BTN   10
#define CLOSE_BTN  9

// finecorsa
#define LIMIT_SWT 8
#define REACHED LOW

// fotoresistore
#define LIGHT_METER 0
#define LIGHT_THRESHOLD 50

// uscite
#define MOTOR 11
#define DIRECTION 13

#define PRESSED LOW
#define RELEASED HIGH

#define GO HIGH
#define STOP LOW

#define OPEN_DIR HIGH
#define CLOSE_DIR LOW
#define STOP_DIR CLOSE_DIR

#define GREEN_LED 7
#define RED_LED 6

#define OPERATION_LED GREEN_LED
#define ERROR_LED RED_LED

#define SLEEP_TIME 500

#define OPERATION_LED_PERIOD 20
#define OPERATION_LED_DUTY 1

enum door_status_t
{
	status_start,///< Stato iniziale
	closed, ///< Porta chiusa
	opened,///< Porta aperta
	closing,///< Porta in chiusura
	opening,///< Porta in apertura
	closing_validation,///< Attesa validazione comando chiusura
	opening_validation,///< Attesa validazione comando apertura
};

enum period_t
{
	night,
	day,
};

enum motor_t
{
	stop,
	close,
	open
};

class Status{
public:
	door_status_t door;
	period_t period;

	byte open_btn;
	byte close_btn;
	byte manual_btn;

	int light;

	unsigned long timer_start;

	bool operator!= ( const Status& rhs )
	{
		bool same = door==rhs.door && period==rhs.period && open_btn==rhs.open_btn && close_btn==rhs.close_btn 
		&& manual_btn==rhs.manual_btn;
		return !same;
	}
};

Status s, sp;

unsigned long cycles = 0;

static inline unsigned long totalMillis ()
{
	millis() + Narcoleptic.millis();
}

static inline void animateOperationLed ()
{
	digitalWrite (OPERATION_LED, cycles % OPERATION_LED_PERIOD < OPERATION_LED_DUTY ? HIGH : LOW);
}

static inline void readInputs ()
{
	s.open_btn   = digitalRead( OPEN_BTN );
	s.close_btn  = digitalRead( CLOSE_BTN );
	s.manual_btn = digitalRead( MANUAL_BTN );

	s.light = analogRead( LIGHT_METER ); 
	s.period = s.light < LIGHT_THRESHOLD ? night : day;
}

static void setMotor (int motor)
{
	switch (motor)
	{
		case stop: digitalWrite( MOTOR, STOP ); digitalWrite( DIRECTION, STOP_DIR ); return;
		case open: digitalWrite( DIRECTION, OPEN_DIR ); digitalWrite( MOTOR, GO ); return;
		case close: digitalWrite( DIRECTION, CLOSE_DIR ); digitalWrite( MOTOR, GO ); return;
	}
}

static inline void manualLoop()
{
	// Nessun premuto o conflitto
	if (  s.open_btn ==  s.close_btn )   
	{
		// Modifica manuale dello stato: perdo ogni certezza devo ricominciare da 0 quando torno in manuale
		if (s.door != opened && s.door != closed)
			s.door = status_start;
		
		return setMotor (stop);
	}

	// Richiesta apertura
	if ( s.open_btn == PRESSED  )
	{	
		// Modifica manuale dello stato: perdo ogni certezza devo ricominciare da 0 quando torno in manuale
		if (s.door != opened)
			s.door = status_start;
		
		return setMotor (open);
	}
	
	// Richiesta chiusura
	if ( s.close_btn == PRESSED )    
	{
		// Modifica manuale dello stato: perdo ogni certezza devo ricominciare da 0 quando torno in manuale
		if (s.door != closed)
			s.door = status_start;
		
		return setMotor (close);
	}
}

static inline void autoLoop()
{
	switch (s.door)
	{
		case opened:
			if (s.period == night)
			{
				s.timer_start = totalMillis();
#ifdef ENABLE_VALIDATION
				s.door = closing_validation;
#else
				setMotor (close);
				s.door = closing;
#endif
			}
		break;
		case closed:
			if (s.period == day)
			{
				s.timer_start = totalMillis();
#ifdef ENABLE_VALIDATION
				s.door = opening_validation;
#else
				setMotor (open);
				s.door = opening;
#endif
			}
		break;
		case status_start: 
			s.timer_start = totalMillis();
			if (s.period == night)
			{
#ifdef ENABLE_VALIDATION
				s.door = closing_validation;
#else
				setMotor (close);
				s.door = closing;
#endif
			}
			else
			{
#ifdef ENABLE_VALIDATION
				s.door = opening_validation;
#else
				setMotor (open);
				s.door = opening;
#endif
			}
		break;
		case opening:
			if (totalMillis() - s.timer_start >= TIMEOUT_COMMAND)
			{
				setMotor (stop);
				s.door = opened;
			}
		break;
		case closing:
			if (totalMillis() - s.timer_start >= TIMEOUT_COMMAND) 
			{
				setMotor (stop);
				s.door = closed;
			}
		break;
		case opening_validation:
			if (totalMillis() - s.timer_start >= TIMEOUT_VALIDATION)
			{
				s.timer_start = totalMillis();
				setMotor (open);
				s.door = opening;
			}
			else if (s.period == night)
			{
				s.timer_start = totalMillis();
				s.door = closing_validation;
			}
		break;
		case closing_validation:
			if (totalMillis() - s.timer_start >= TIMEOUT_VALIDATION)
			{
				s.timer_start = totalMillis();
				setMotor (close);
				s.door = closing;
			}
			else if (s.period == day)
			{
				s.timer_start = totalMillis();
				s.door = opening_validation;
			}
		break;
		default:
			error ("Stato ignoto");
	}
}

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

        setMotor( stop );

        digitalWrite( RED_LED, LOW );
	digitalWrite( GREEN_LED, LOW );  

	s.door = status_start;
	s.period = day;
	s.timer_start = totalMillis();

#ifdef SER_DBG_PRINT
	Serial.begin(9600);
	Serial.println("");
	Serial.println("*** PROGRAMMA AVVIATO ***");
	Serial.println("");
#else
	Narcoleptic.disableSerial();
#endif
}

void loop()
{
	animateOperationLed ();
	
	readInputs ();

	if ( s.manual_btn == PRESSED )
		manualLoop ();
	else
		autoLoop ();

	#ifdef SER_DBG_PRINT
	if ( s != sp ) 
		print_status();
	#endif

	sp = s;
	++cycles;

        Narcoleptic.delay (SLEEP_TIME);
}


void error( char* msg )
{
	digitalWrite (OPERATION_LED, LOW);
	digitalWrite (ERROR_LED, HIGH);
	#ifdef SER_DBG_PRINT
	Serial.print("ERRORE: ");
	Serial.println(msg);
	Serial.print("RESETTARE");
	#endif
	while(true){};
}

#ifdef SER_DBG_PRINT
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
		case closing: Serial.println("PORTA IN CHIUSURA"); break;
		case opening: Serial.println("PORTA IN APERTURA"); break;
		case closing_validation: Serial.println("VALIDAZIONE CHIUSURA"); break;
		case opening_validation: Serial.println("VALIDAZIONE APERTURA"); break;
		case status_start: Serial.println("STATO INIZIALE"); break;
		default: error("STATO PORTA INASPETTATO");   break;
	}

	Serial.println("-----------------------------------------");
}

#endif
