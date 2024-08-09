//Valve pins
#define methMainPin 6 // Main methane tank 6
#define oxMainPin 4   // Main oxygen tank 4
#define nMethPin   7    // Nitrogen on methane side 3
#define nOxPin 3      // Nitrogren on ox side 7
#define methIgnitorPin 8  // Methane side valve by combustion   8 
#define oxIgnitorPin 5    // Ox side valve by combustion 5

//Spark plug pin
#define sparkPin 2    //Spark plug pin

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

//Extra smoothed old values to help set the baseline
float oldPressure1Smooth = 0;
float oldPressure2Smooth = 0;
float oldPressure3Smooth = 0;
float oldPressure4Smooth = 0;
float oldTemperature1Smooth = 0;
float oldTemperature2Smooth = 0;
float oldTemperature3Smooth = 0;
float oldTemperature4Smooth = 0;

//Offset variables for calibration
float pressure1Offset = 0;
float pressure2Offset = 0;
float pressure3Offset = 0;
float pressure4Offset = 0;
float temperature1Offset = 0;
float temperature2Offset = 0;
float temperature3Offset = 0;
float temperature4Offset = 0;

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
// 12=> reload SD card
int systemState = 0;  //System State of firing vs idle:


float t0 = 0;           //Timer variable for each state
float fireDuration = 1; //Default duration for fire duration

int valveStates[] = {0, 0, 0, 0, 0, 0}; //Meth main, ox main, nitrogen meth, nitrogen ox, meth final, ox final
int valveCnt = 6;

//Defining serial output so it works with bluetooth module
#define bluetooth Serial1

//SD Card stuff
#include <SD.h> //For SD Card
File myFile;
const int chipSelect = 53;
double dataOut[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int dataOutLength = 27;
bool sdLoaded = false;

void setup() {
  bluetooth.begin(9600);
  Serial.begin(9600);

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

  //Starting the SD Card
  sdLoaded = (SD.begin(chipSelect));  //True if started properly, false if not detected
  myFile = SD.open("text.csv", FILE_WRITE);
  if(myFile){
    myFile.write("State, Meth Main, Ox Main, Meth N, Ox N, Meth Ign, Ox Ign, M Pre [C], M Post[C], Ox Pre[C], Ox Post[C], M Pre [psig], M Post[psig], Ox Pre[psig], Ox Post[psig], SD Loaded, Time, M_dot Methane [kg/s], M_dot Oxygen [kg/s], P1 Offset, P2 offset, P3 Offset, P4 Offset, T1 Offset, T2 Offset, T3 Offset, T4 Offset, Fire Duration");
  } else {
    sdLoaded = false;
  }

}

//Incoming messages
String incomingCommand = "";
String command = "";

void loop() {
  delay(100);

  //Reading any incoming bluetooth messages
  command = "";
  while(bluetooth.available()){
    char nextChar = char(bluetooth.read());
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
  float thermo1 = get_temperature(thermo1Pin, oldTemperature1, 0.1, temperature1Offset);
  float thermo2 = get_temperature(thermo2Pin, oldTemperature2, 0.1, temperature2Offset);
  float thermo3 = get_temperature(thermo3Pin, oldTemperature3, 0.1, temperature3Offset);
  float thermo4 = get_temperature(thermo4Pin, oldTemperature4, 0.1, temperature4Offset);

  float pressure1 = get_pressure(pressure1Pin, oldPressure1, 0.1, pressure1Offset);
  float pressure2 = get_pressure(pressure2Pin, oldPressure2, 0.1, pressure2Offset);
  float pressure3 = get_pressure(pressure3Pin, oldPressure3, 0.1, pressure3Offset);
  float pressure4 = get_pressure(pressure4Pin, oldPressure4, 0.1, pressure4Offset);

  float thermo1Smooth = get_temperature(thermo1Pin, oldTemperature1Smooth, 0.8, temperature1Offset);
  float thermo2Smooth = get_temperature(thermo2Pin, oldTemperature2Smooth, 0.8, temperature2Offset);
  float thermo3Smooth = get_temperature(thermo3Pin, oldTemperature3Smooth, 0.8, temperature3Offset);
  float thermo4Smooth = get_temperature(thermo4Pin, oldTemperature4Smooth, 0.8, temperature4Offset);

  float pressure1Smooth = get_pressure(pressure1Pin, oldPressure1Smooth, 0.8, pressure1Offset);
  float pressure2Smooth = get_pressure(pressure2Pin, oldPressure2Smooth, 0.8, pressure2Offset);
  float pressure3Smooth = get_pressure(pressure3Pin, oldPressure3Smooth, 0.8, pressure3Offset);
  float pressure4Smooth = get_pressure(pressure4Pin, oldPressure4Smooth, 0.8, pressure4Offset);

  //Updating old data
  oldPressure1 = pressure1;
  oldPressure2 = pressure2;
  oldPressure3 = pressure3;
  oldPressure4 = pressure4;
  oldTemperature1 = thermo1;
  oldTemperature2 = thermo2;
  oldTemperature3 = thermo3;
  oldTemperature4 = thermo4;

  oldPressure1Smooth = pressure1Smooth;
  oldPressure2Smooth = pressure2Smooth;
  oldPressure3Smooth = pressure3Smooth;
  oldPressure4Smooth = pressure4Smooth;
  oldTemperature1Smooth = thermo1Smooth;
  oldTemperature2Smooth = thermo2Smooth;
  oldTemperature3Smooth = thermo3Smooth;
  oldTemperature4Smooth = thermo4Smooth;

  //Calculating the mass flow rates
  //0.00124
  //0.00176
  float mDotMethane = 0.0015 * sqrt(max((pressure1 - pressure2) / pressure1, 0)) * sqrt(max(9 / 5 * thermo1 + 491.67, 0)) * (pressure1  / (thermo1 + 273.15));
  float mDotOxygen = 0.0015 * sqrt(max((pressure1 - pressure2) / pressure1, 0)) * sqrt(max(9 / 5 * thermo1 + 491.67, 0)) * (pressure1  / (thermo1 + 273.15));

  //If the run valve is shut, then we should probably say there isn't any mass flow am I right
  if(valveStates[4] != 1){
    mDotMethane = 0;
  }

  if(valveStates[5] != 1){
    mDotOxygen = 0;
  }

  float t = millis()/1000.0;
  
  //Updating the system state based on the command given
  if(systemState == 0 && command != "" && command.charAt(0) != '_'){
    systemState = command.toInt();
    t0 = t;
    
  }

  //Abort override
  if(systemState != 0 && command == "A"){
    systemState = 4;
    t0 = t;
  }
  
  if (systemState == 0 && command != "" && command.charAt(0) == '_'){
    //_ leading the command means it's one of the calibration commands
    // if the char after the _ is a P, it's a pressure calibration, if it's a T it's for temp/thermocouples

    if(command.charAt(1) == 'P'){
      //Pressure calibration

      //Desired value
      float desired = command.substring(2).toFloat();

      pressure1Offset += desired - pressure1Smooth;
      pressure2Offset += desired - pressure2Smooth;
      pressure3Offset += desired - pressure3Smooth;
      pressure4Offset += desired - pressure4Smooth;


    } else if (command.charAt(1) == 'T') {
      //Thermocouple calibration

      //Desired value
      float desired = command.substring(2).toFloat();

      temperature1Offset += desired - thermo1Smooth;
      temperature2Offset += desired - thermo2Smooth;
      temperature3Offset += desired - thermo3Smooth;
      temperature4Offset += desired - thermo4Smooth;
    } else {
      //Fire duration update
      float fireDuration = command.substring(2).toFloat();
    }
  }

  //Logging info:
  bluetooth.print(t);
  bluetooth.print(",");
  bluetooth.print(systemState);
  bluetooth.print(",");

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

    } else if(t - t0 < 3){
      setValveStates(0, 0, 0, 0, 1, 1);

    } else if(t - t0 > 3){
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

    } else if(t - t0 < (fireDuration + 1)){
      digitalWrite(sparkPin, HIGH);
      setValveStates(1, 1, 0, 0, 1, 1);

    } else if(t - t0 < (fireDuration + 2)){
      digitalWrite(sparkPin, LOW);
      setValveStates(0, 0, 0, 0, 1, 1);
      
    } else if(t - t0 >= (fireDuration + 2)){
      systemState = 4;
      t0 = t;
    }
  }

  //SD update
  if(systemState == 12){
    //Starting the SD Card
    sdLoaded = (SD.begin(chipSelect));  //True if started properly, false if not detected
    myFile = SD.open("text.csv", FILE_WRITE);
    if(myFile){
      myFile.write("State, Meth Main, Ox Main, Meth N, Ox N, Meth Ign, Ox Ign, M Pre [C], M Post[C], Ox Pre[C], Ox Post[C], M Pre [psig], M Post[psig], Ox Pre[psig], Ox Post[psig], SD Loaded, Time");
    } else {
      sdLoaded = false;
    }

    systemState = 0;
  }


  //Updating valves based on desired state
  setValves();
  
  //Logging more info
  for(int i = 0; i < valveCnt; i++){
    bluetooth.print(valveStates[i]);
    if(i != valveCnt-1){
      bluetooth.print(":");
    }
  }

  bluetooth.print(",");
  bluetooth.print(thermo1);
  bluetooth.print(",");
  bluetooth.print(thermo2);
  bluetooth.print(",");
  bluetooth.print(thermo3);
  bluetooth.print(",");
  bluetooth.print(thermo4);
  bluetooth.print(",");
  bluetooth.print(pressure1);
  bluetooth.print(",");
  bluetooth.print(pressure2);
  bluetooth.print(",");
  bluetooth.print(pressure3);
  bluetooth.print(",");
  bluetooth.print(pressure4);
  bluetooth.print(",");
  bluetooth.print(mDotMethane, 6);
  bluetooth.print(",");
  bluetooth.print(mDotOxygen, 6);
  bluetooth.print(",");
  bluetooth.print(sdLoaded);
  bluetooth.print(",");
  bluetooth.print(pressure1Offset);
  bluetooth.print(",");
  bluetooth.print(pressure2Offset);
  bluetooth.print(",");
  bluetooth.print(pressure3Offset);
  bluetooth.print(",");
  bluetooth.print(pressure4Offset);
  bluetooth.print(",");
  bluetooth.print(temperature1Offset);
  bluetooth.print(",");
  bluetooth.print(temperature2Offset);
  bluetooth.print(",");
  bluetooth.print(temperature3Offset);
  bluetooth.print(",");
  bluetooth.print(temperature4Offset);
  bluetooth.print(",");
  bluetooth.print(fireDuration);
  
  bluetooth.print("|"); //End of message seperator

  //Writing data to SD card
  dataOut[0] = systemState;
  dataOut[1] = valveStates[0];
  dataOut[2] = valveStates[1];
  dataOut[3] = valveStates[2];
  dataOut[4] = valveStates[3];
  dataOut[5] = valveStates[4];
  dataOut[6] = valveStates[5];
  dataOut[7] = thermo1;
  dataOut[8] = thermo2;
  dataOut[9] = thermo3; 
  dataOut[10] = thermo4;
  dataOut[11] = pressure1;
  dataOut[12] = pressure2;
  dataOut[13] = pressure3;
  dataOut[14] = pressure4;
  dataOut[15] = sdLoaded ? 1 : 0;
  dataOut[16] = t;
  dataOut[17] = mDotMethane;
  dataOut[18] = mDotOxygen;
  dataOut[19] = pressure1Offset;
  dataOut[20] = pressure2Offset;
  dataOut[21] = pressure3Offset;
  dataOut[22] = pressure4Offset;
  dataOut[23] = temperature1Offset;
  dataOut[24] = temperature2Offset;
  dataOut[25] = temperature3Offset;
  dataOut[26] = temperature4Offset;
  dataOut[27] = fireDuration;


  writeToSD("text.csv", dataOut);
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

float get_temperature(int pin, float oldTemp, float smoothing, float offset) {
  float refVoltage = 5;
  float resolution = 10;
  float reading = analogRead(pin);
  float voltage = reading * (refVoltage / (pow(2, resolution)-1)); 
  float newTemp = (voltage - 1.25) / 0.005 + offset;
  return newTemp * (1-smoothing) + oldTemp * smoothing;
}

float get_pressure(int pin, float oldPsi, float smoothing, float offset){
  float reading = analogRead(pin);
  float psi = (reading/1024.0*5.0 - 0.5) * 75 + offset;
  return psi * (1-smoothing) + oldPsi * smoothing;
}

bool writeToSD(char filename[], double out[]){
  myFile = SD.open(filename, FILE_WRITE);     
   // if the file opened okay, write to it:
   if (myFile) 
   {
     //Serial.println("Writing to file");
     for(int i = 0; i <= dataOutLength; i++){
      myFile.print(out[i]);
      //Serial.print(out[i]);
      
      if(i != dataOutLength){
        myFile.print(",");
        //Serial.print(",");
        
      }
     }
     myFile.println();
     //Serial.println();
     myFile.close();
   } 
   else 
   {
     Serial.println("error opening csv.txt");
     //SD.begin(chipSelect);
     sdLoaded = false;
   }
   
   return true;
}
