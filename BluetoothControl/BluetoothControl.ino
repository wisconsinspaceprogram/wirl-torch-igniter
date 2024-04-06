//Valve pins
#define methMainPin 3 // Main methane tank 
#define oxMainPin 4   // Main oxygen tank
#define nMethPin 5    // Nitrogen on methane side
#define nOxPin 6      // Nitrogren on ox side
#define methIgnitorPin 7  // Methane side valve by combustion
#define oxIgnitorPin 8    // Ox side valve by combustion

//Spark plug pin
#define sparkPin 9    //Spark plug pin

//Thermocouples
#define thermo1Pin A0    //Methane side first
#define thermo2Pin A1    //Methane side second
#define thermo3Pin A2    //Ox side first
#define thermo4Pin A3    //Ox side second

//Pressure readings
#define pressure1Pin A4    //Methane side first
#define pressure2Pin A5    //Methane side second
#define pressure3Pin A6    //Ox side first
#define pressure4Pin A7    //Ox side second

//Variables to smooth out temperature and pressure readings
float oldPressure1 = 0;
float oldPressure2 = 0;
float oldPressure3 = 0;
float oldPressure4 = 0;
float oldTemperature1 = 0;
float oldTemperature2 = 0;
float oldTemperature3 = 0;
float oldTemperature4 = 0;


//System State Variables.
// 0 => idle, 
// 1 => firing, 
// 2=> ox vent, 
// 3=> methane vent, 
// 4=> full vent, 
// 5=> toggle valve 1, meth main, 
// 6=> toggle valve 2, ox main, 
// 7=> toggle valve 3, nitrogen meth, 
// 8=> toggle valve 4, nitrogen ox, 
// 9=> toggle valve 5, meth final,
// 10=> toggle valve 6, ox final,
// 11=> dry firing,
int systemState = 0;  //System State of firing vs idle:


float t0 = 0;           //Timer variable for each state

int valveStates[] = {0, 0, 0, 0, 0, 0}; //Meth main, ox main, nitrogen meth, nitrogen ox, meth final, ox final
int valveCnt = 6;

//Defining serial output so it works with bluetooth module
#define ser Serial1

void setup() {
  ser.begin(9600);
  Serial.begin(9600);

  Serial.println("Start");
  Serial.println("Start");
  Serial.println("Start");

  //Configuring pins
  pinMode(methMainPin, OUTPUT);
  pinMode(oxMainPin, OUTPUT);
  pinMode(nMethPin, OUTPUT);
  pinMode(nOxPin, OUTPUT);
  pinMode(methIgnitorPin, OUTPUT);
  pinMode(oxIgnitorPin, OUTPUT); 
  pinMode(sparkPin, OUTPUT);

  //Making sure we start with everything shut and disabled
  digitalWrite(sparkPin, LOW);
  setValveStates(0, 0, 0, 0, 0, 0);

}

//Incoming messages
String incomingCommand = "";
String command = "";

void loop() {
  delay(100);

  //Reading any incoming bluetooth messages
  command = "";
  while(ser.available()){
    char nextChar = char(ser.read());
    if(nextChar == 's'){
      incomingCommand = "";
    } else if(nextChar == 'e'){
      command = incomingCommand;
    } else{
      incomingCommand += nextChar;
    }
  }

  //Outputting command to local serial connection (not bluetooth) for debugging
  if(command != ""){
    Serial.println(millis());
    Serial.println(command);
  }

  //Getting data from sensors
  float thermo1 = get_temperature(thermo1Pin, oldTemperature1);
  float thermo2 = get_temperature(thermo2Pin, oldTemperature2);
  float thermo3 = get_temperature(thermo3Pin, oldTemperature3);
  float thermo4 = get_temperature(thermo4Pin, oldTemperature4);

  float pressure1 = get_pressure(pressure1Pin, oldPressure1);
  float pressure2 = get_pressure(pressure2Pin, oldPressure2);
  float pressure3 = get_pressure(pressure3Pin, oldPressure3);
  float pressure4 = get_pressure(pressure4Pin, oldPressure4);

  //Updating old data
  oldPressure1 = pressure1;
  oldPressure2 = pressure2;
  oldPressure3 = pressure3;
  oldPressure4 = pressure4;
  oldTemperature1 = thermo1;
  oldTemperature2 = thermo2;
  oldTemperature3 = thermo3;
  oldTemperature4 = thermo4;

  float t = millis()/1000.0;
  
  //Updating the system state based on the command given
  if(systemState == 0 && command != ""){
    systemState = command.toInt();
    t0 = t;
    
  }

  //Logging info:
  ser.print(t);
  ser.print(",");
  ser.print(systemState);
  ser.print(",");

  //Forcing spark pin off
  if(systemState == 0){
    digitalWrite(sparkPin, LOW);
  }

  //Valve toggle updates
  if(systemState >= 5 && systemState <= 10){
    toggleValve(systemState - 5); //Valve index 0 is system state 5, so this handles all the toggles
    systemState = 0;              //Setting our state back to 0, done with this operation
  }

  //Ox vent
  if(systemState == 2){
    digitalWrite(sparkPin, LOW);
    if(t - t0 < 1){
      setValveStates(0, 0, 0, 0, 0, 1);

    } else if(t - t0 < 2.5){
      setValveStates(0, 0, 0, 1, 0, 1);

    } else if(t - t0 > 2.5){
      setValveStates(0, 0, 0, 0, 0, 0);
      systemState = 0;
    }

    
  }

  //Meth vent
  if(systemState == 3){
    digitalWrite(sparkPin, LOW);
    if(t - t0 < 1){
      setValveStates(0, 0, 0, 0, 1, 0);

    } else if(t - t0 < 2.5){
      setValveStates(0, 0, 1, 0, 1, 0);

    } else if(t - t0 > 2.5){
      setValveStates(0, 0, 0, 0, 0, 0);
      systemState = 0;
    }

    
  }

  //Full vent
  if(systemState == 4){
    digitalWrite(sparkPin, LOW);
    if(t - t0 < 1){
      setValveStates(0, 0, 0, 0, 1, 1);

    } else if(t - t0 < 2.5){
      setValveStates(0, 0, 1, 1, 1, 1);

    } else if(t - t0 > 2.5){
      setValveStates(0, 0, 0, 0, 0, 0);
      systemState = 0;
    }

    
  }

  //Dry Fire
  if(systemState == 11){
    digitalWrite(sparkPin, LOW);
    if(t - t0 < 1){
      setValveStates(1, 1, 0, 0, 0, 0);

    } else if(t - t0 < 2.5){
      setValveStates(1, 1, 0, 0, 1, 1);

    } else if(t - t0 < 3.5){
      setValveStates(0, 0, 0, 0, 1, 1);
      
    } else if(t - t0 < 4.5){
      setValveStates(0, 0, 1, 1, 1, 1);

    } else if(t - t0 > 4.5){
      setValveStates(0, 0, 0, 0, 0, 0);
      systemState = 0;
    }
  }

  //Fire!!!!
  if(systemState == 1){
    if(t - t0 < 1){
      digitalWrite(sparkPin, LOW);
      setValveStates(1, 1, 0, 0, 0, 0);

    } else if(t - t0 < 2.5){
      digitalWrite(sparkPin, HIGH);
      setValveStates(1, 1, 0, 0, 1, 1);

    } else if(t - t0 < 3.5){
      digitalWrite(sparkPin, LOW);
      setValveStates(0, 0, 0, 0, 1, 1);
      
    } else if(t - t0 < 4.5){
      digitalWrite(sparkPin, LOW);
      setValveStates(0, 0, 1, 1, 1, 1);

    } else if(t - t0 > 4.5){
      digitalWrite(sparkPin, LOW);
      setValveStates(0, 0, 0, 0, 0, 0);
      systemState = 0;
    }
  }


  //Updating valves based on desired state
  setValves();
  
  //Logging more info
  for(int i = 0; i < valveCnt; i++){
    ser.print(valveStates[i]);
    if(i != valveCnt-1){
      ser.print(":");
    }
  }

  ser.print(",");
  ser.print(thermo1);
  ser.print(",");
  ser.print(thermo2);
  ser.print(",");
  ser.print(thermo3);
  ser.print(",");
  ser.print(thermo4);
  ser.print(",");
  ser.print(pressure1);
  ser.print(",");
  ser.print(pressure2);
  ser.print(",");
  ser.print(pressure3);
  ser.print(",");
  ser.print(pressure4);
  ser.print("|"); //End of message seperator
}

void toggleValve(int index){
  valveStates[index] = valveStates[index] == 0 ? 1 : 0;
}

void setValveStates(int state1, int state2, int state3, int state4, int state5, int state6){
  valveStates[0] = state1;
  valveStates[1] = state2;
  valveStates[2] = state3;
  valveStates[3] = state4;
  valveStates[4] = state5;
  valveStates[5] = state6;
}

void setValves(){
  digitalWrite(methMainPin, valveStates[0] == 0 ? LOW : HIGH);
  digitalWrite(oxMainPin,  valveStates[1] == 0 ? LOW : HIGH);
  digitalWrite(nMethPin,  valveStates[2] == 0 ? LOW : HIGH);
  digitalWrite(nOxPin,  valveStates[3] == 0 ? LOW : HIGH);
  digitalWrite(methIgnitorPin,  valveStates[4] == 0 ? LOW : HIGH);
  digitalWrite(oxIgnitorPin,  valveStates[5] == 0 ? LOW : HIGH);
}

float get_temperature(int pin, float oldTemp) {
  float refVoltage = 5;
  float resolution = 10;
  float reading = analogRead(pin);
  float voltage = reading * (refVoltage / (pow(2, resolution)-1)); 
  float newTemp = (voltage - 1.25) / 0.005;
  return newTemp * 0.1 + oldTemp * 0.9;
}

float get_pressure(int pin, float oldPsi){
  float reading = analogRead(pin);
  float psi = (reading/1024.0*5.0 - 0.5) * 75 + 1.98 +.25;
  return psi * 0.2 + oldPsi * 0.8;
}
