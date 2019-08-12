// Sketch for Particle Photon - PIR Sensor / Motion Detection
// By Anton

/********************************/
/********* PIR SET UP ***********/
/********************************/

int inputPin = D0;              // choose the input pin (for PIR sensor)
int ledPin = D1;                // LED Pin
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

int calibrateTime = 5000;       // wait for the thingy to calibrate

/********************************/
/**** SOUND DETECTOR SET UP *****/
/********************************/
int sample_rate = 1000;         //time between samples (in miliseconds)
unsigned long time_now = 0;
const int array_size = 1200;    // 1000/50=20 * 60=1200
int snd_array[array_size] = {};
int snd_max, prev_max = 0;
int snd_min, prev_min = 4096;
double snd_avg = 2048;

const int blink_thresh = 50;

unsigned long broadcast_interval = 3000;
unsigned long last_broadcast = 0;

void averageReading(int value) {

    // Shift all the values right by 1
    for(int i = array_size-1; i >= 1; i--) 
    {
        snd_array[i] = snd_array[i-1]; 
        if((snd_array[i] < snd_min) && (snd_array[i] != 0))
        {
            snd_min = snd_array[i];
            
        }
        if(snd_array[i] > snd_max)
        {
            snd_max = snd_array[i];
            
        }
    }

    snd_array[0] = value; 

    // Average array
    float avg_sum = 0; 
    int size = 0 ;
    for (int a=0; a <= array_size; a++) 
    {
        if(snd_array[a] > 0)
        {
            size++ ;
            avg_sum  = avg_sum + snd_array[a];
        }
    }
    snd_avg = avg_sum / size;
    
    Particle.variable("snd_avg",snd_avg); //pull to google sheets
    Particle.variable("snd_min",snd_min); 
    Particle.variable("snd_max",snd_max); 
}

void blinkMic(int reading) {
    if(reading > blink_thresh) {
        digitalWrite(D7, HIGH);
        Particle.publish("Sound Sensor", "Sound Detected", PRIVATE);
    } else {
        digitalWrite(D7, LOW);
    }
}


void checkBroadcast() {
    unsigned long now = millis();
    if((now - last_broadcast) > broadcast_interval) {
        Serial.print("Avg: "); Serial.println(snd_avg);
        Serial.print("Min: "); Serial.println(snd_min);
        Serial.print("Max: "); Serial.println(snd_max);

        snd_avg = 0;
        snd_min = 4096;
        snd_max = 0;
        snd_array[array_size] = {};
        last_broadcast = now;
    }
}
/***********************************************/
/***********************************************/
/***********************************************/

void setup() {
    /********* PIR **********/
    pinMode(ledPin, OUTPUT);
    pinMode(inputPin, INPUT);     // declare sensor as input
    
    /**** SOUND DETECTOR ****/
    Serial.begin(9600);
    pinMode(A0, INPUT); // mic AUD connected to Analog pin 0
    pinMode(D7, OUTPUT); // flash on-board LED
}

void loop() {
  /********* PIR **********/
  // if the sensor is calibrated
  if (calibrated()) {
  // get the data from the sensor
    readTheSensor();

    // report it out, if the state has changed
    reportTheData();
    }
  
  /**** SOUND DETECTOR ****/
    
  if(millis() > time_now + period){
    time_now = millis();
    int mic_reading = analogRead(0); //Serial.println(mic_reading);
    blinkMic(mic_reading);
    averageReading(mic_reading); 
    checkBroadcast();  
  }    
    
    delay(sample_rate);
}

void readTheSensor() {
    val = digitalRead(inputPin);
}

//bool calibrated_sound)

bool calibrated() {
    return millis() - calibrateTime > 0;
}

void setLED(int state) {
    digitalWrite(ledPin, state);
}

void reportTheData() {
    if (val == HIGH) {
        // the current state is no motion
        // i.e. it's just changed
        // announce this change by publishing an event
        if (pirState == LOW) {
          // we have just turned on
          Particle.publish("PhotonMotion", "Motion Detected", PRIVATE);
          // Update the current state
          pirState = HIGH;
          setLED(pirState);
        }
    } else {
        if (pirState == HIGH) {
          // we have just turned of
          // Update the current state
          Particle.publish("PhotonMotion", "Off", PRIVATE);
          pirState = LOW;
          setLED(pirState);
        }
    }
}


