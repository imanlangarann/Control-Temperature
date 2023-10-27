#include <TM1637Display.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define CLK 3
#define DT 4
#define SW 5

#define OUT 13

byte set_temprature = 25;
byte env_temprature = 0;
byte heat_on_time = 5;
byte heat_off_time = 7;
byte heat_count = 0;
byte currentStateCLK;
byte lastStateCLK;
unsigned long lastButtonPress = 0;
unsigned long lastTempRead = 0;

uint8_t data[] = { 0x00, 0x00, 0x00, 0x00 };

bool is_going_up = false;

TM1637Display display(12, 11);

OneWire oneWire(2);
DallasTemperature sensors(&oneWire);

enum stt {current_temp, set_temp, on_time, off_time} stt = current_temp;

void setup() {

  display.setBrightness(0x0f);

  sensors.begin();
        
	// Set encoder pins as inputs
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	pinMode(SW, INPUT_PULLUP);
  pinMode(OUT,OUTPUT);

	// Setup Serial Monitor
	Serial.begin(9600);

	// Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);
  update_display();
}

void loop() {
  if(millis() > lastTempRead + 500){
    lastTempRead = millis();
    readtemp();
    check_out();
  }

  readrot();

	delay(1);
}

void check_out(){
  // if(env_temprature > set_temprature){
  //   digitalWrite(OUT, 0); //ON
  // }
  // else{
  //   digitalWrite(OUT, 1); //OFF
  // }

  if(!is_going_up){
    if (heat_count != 0) heat_count--;
    if(env_temprature < set_temprature && heat_count == 0){
      is_going_up = true;
      digitalWrite(OUT, 1);
    }
  }
  else if(is_going_up){
    heat_count++;
    if(env_temprature >= set_temprature || heat_count == (heat_on_time*2)){
      is_going_up = false;
      heat_count = (heat_off_time*2);
      digitalWrite(OUT, 0);
    }

  }
}

void readtemp(){
  sensors.requestTemperatures();
  env_temprature = sensors.getTempCByIndex(0);
  update_display();
}


void readrot(){
          
	// Read the current state of CLK
	currentStateCLK = digitalRead(CLK);

	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
      if(stt == set_temp)
			  set_temprature--;
      else if(stt == on_time)
        heat_on_time--;
      else if(stt == off_time)
        heat_off_time--;
		} else {
			if(stt == set_temp)
			  set_temprature++;
      else if(stt == on_time)
        heat_on_time++;
      else if(stt == off_time)
        heat_off_time++;
		}

		update_display();
	}

	// Remember last CLK state
	lastStateCLK = currentStateCLK;

	// Read the button state
	int btnState = digitalRead(SW);

	//If we detect LOW signal, button is pressed
	if (btnState == LOW) {
		//if 50ms have passed since last LOW pulse, it means that the
		//button has been pressed, released and pressed again
		if (millis() - lastButtonPress > 50) {
			next_stt();
		}

		// Remember last button press event
		lastButtonPress = millis();
	}
}

void next_stt(){
  if (stt == current_temp)
    stt = set_temp;
  else if (stt == set_temp)
    stt = on_time;
  else if (stt == on_time)
    stt = off_time;
  else if (stt == off_time)
    stt = current_temp; 
  update_display();
}

void update_display(){
  if(stt == current_temp){
    // display.showNumberDec(env_temprature);
    data[0] = 0x00;

    data[2] = display.encodeDigit(env_temprature/10);
    data[3] = display.encodeDigit(env_temprature%10);

    display.setSegments(data);
  }
  else if (stt == set_temp){
    // display.showNumberDec(set_temprature);

    data[0] = display.encodeDigit(1);

    data[2] = display.encodeDigit(set_temprature/10);
    data[3] = display.encodeDigit(set_temprature%10);

    display.setSegments(data);
  }
  else if (stt == on_time){

    data[0] = display.encodeDigit(2);

    data[2] = display.encodeDigit(heat_on_time/10);
    data[3] = display.encodeDigit(heat_on_time%10);

    display.setSegments(data);
  }
  else if (stt == off_time){

    data[0] = display.encodeDigit(3);

    data[2] = display.encodeDigit(heat_off_time/10);
    data[3] = display.encodeDigit(heat_off_time%10);

    display.setSegments(data);
  }
}
