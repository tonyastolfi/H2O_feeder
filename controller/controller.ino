/*
 * 
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;
int valve = 4;
int empty_sensor = 8;
int full_sensor = 9;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  Serial.begin(9600);
  pinMode(led, OUTPUT);     
  pinMode(valve, OUTPUT);     
  pinMode(empty_sensor, INPUT_PULLUP);     
  pinMode(full_sensor, INPUT_PULLUP);     
}

#define MS_PER_CYCLE 100
#define MS_PER_SECOND 1000

// Keep the valve open for a maximum of 30 seconds. (1 cycle == 0.1s)

int32_t cycles_to_seconds(int32_t c) {
  return c * MS_PER_CYCLE / MS_PER_SECOND;
}
int32_t seconds_to_cycles(int32_t s) {
  return s * MS_PER_SECOND / MS_PER_CYCLE;
}

const int32_t MAX_FILLING_SECONDS = 60 * 2;
const int32_t MAX_FILLING_CYCLES = seconds_to_cycles(MAX_FILLING_SECONDS);
const int32_t MIN_IDLE_SECONDS = 60 * 10;
const int32_t MIN_IDLE_CYCLES = seconds_to_cycles(MIN_IDLE_SECONDS);

bool filling = false;
bool waiting_to_fill = false;
int32_t cycles_filling = 0;
int32_t cycles_idle = MIN_IDLE_CYCLES;

void water_on() {
  cycles_idle = 0;
  if (!filling) {
    Serial.println("Turning water on");
  }
  waiting_to_fill = false;
  filling = true;
  digitalWrite(valve, HIGH);  
  digitalWrite(led, HIGH);
}
void water_off() {
  cycles_filling = 0;
  if (filling) {
    Serial.println("Turning water off");
  }
  filling = false;
  digitalWrite(valve, LOW);
  digitalWrite(led, LOW);
}

int cycle = 0;

// the loop routine runs over and over again forever:
void loop() {
  bool is_empty = !digitalRead(empty_sensor);
  bool is_full = digitalRead(full_sensor);
  bool bad_sensor_reading = false;

  if (is_empty) {
    if (is_full) {
      Serial.println("Empty and Full sensors both active; something is wrong!");
      bad_sensor_reading = true;
      water_off();      
    } else {
      if (!filling) {
        if (cycles_idle < MIN_IDLE_CYCLES) {
          if (!waiting_to_fill) {
            waiting_to_fill = true;
            Serial.println("Empty sensor active but water has not been off long enough.  Waiting...");
          }
        } else {
          water_on();
        }
      }
    }
  } else if (is_full) {
    water_off();    
  }
  if (!bad_sensor_reading) {
    if (filling) {
      ++cycles_filling;
      if (cycles_filling > MAX_FILLING_CYCLES) {
        Serial.println("Water on for too long.");
        water_off();
        int led_on = LOW;
        for (;;) {
          led_on = (HIGH + LOW) - led_on;
          digitalWrite(led, led_on);
          delay(700);
        }
      }
    } else {
      if (cycles_idle < MIN_IDLE_CYCLES) {
        ++cycles_idle;
        if (cycles_idle == MIN_IDLE_CYCLES) {
          Serial.println("Water off timer reset.");
        }
      }
    }
  }
  delay(MS_PER_CYCLE);
  if ((cycle % (1 * MS_PER_SECOND / MS_PER_CYCLE)) == 0) {
    if (cycles_filling > 0 && cycles_idle > 0) {
      Serial.println("Filling and idle!  This is impossible.");    
    } else if (cycles_filling > 0) {
      Serial.print("filling for ");
      Serial.println(cycles_to_seconds(cycles_filling));
    } else {
      Serial.print("idle for ");
      Serial.println(cycles_to_seconds(cycles_idle));
    }
  }
  if (waiting_to_fill) {
    switch (cycle % 10) {
      case 1:
      case 3:
        digitalWrite(led, HIGH);
        break;
      default:
        digitalWrite(led, LOW);
        break;
    }
  }
  ++cycle;
}
