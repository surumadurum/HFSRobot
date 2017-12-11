#define PIN_DEMUX_A_OE 32
#define PIN_DEMUX_A_S0 33
#define PIN_DEMUX_A_S1 25
#define PIN_DEMUX_A_S2 26
#define PIN_DEMUX_A_S3 27
#define PIN_DEMUX_A_SIG 34



void setup() {

      pinMode(PIN_DEMUX_A_OE,OUTPUT);
      pinMode(PIN_DEMUX_A_S0,OUTPUT);
      pinMode(PIN_DEMUX_A_S1,OUTPUT);
      pinMode(PIN_DEMUX_A_S2,OUTPUT);
      pinMode(PIN_DEMUX_A_S3,OUTPUT);
      pinMode(PIN_DEMUX_A_SIG,OUTPUT);

//      pinMode(PIN_DEMUX_A_SIG,ANALOG);


        Serial.begin(115200);

        
}

void loop() {
       digitalWrite(PIN_DEMUX_A_S0, 1);
      digitalWrite(PIN_DEMUX_A_S1, 1);
      digitalWrite(PIN_DEMUX_A_S2, 1);
      digitalWrite(PIN_DEMUX_A_S3, 1); 
      digitalWrite(PIN_DEMUX_A_OE, LOW);  //active low -> activate demux by default

      delay(1);
 
      Serial.println(analogRead(PIN_DEMUX_A_SIG));
      
      delay(1);

      digitalWrite(PIN_DEMUX_A_OE, HIGH);  //active low -> activate demux by default
}
