#include <Wire.h>// Library for communication with I2C devices using the SDA (data line) and SCL (clock line)
#include <Adafruit_MPRLS.h> // Library for the MPRLS (pressure sensor)
#include <MPU6050.h>        // Library for the MPU6050 (accelerometer)
#include <SD.h>             // Library for SD card communication
#include <SPI.h>            // Library for communication with SPI devices (teensy being the master device)
#define WIRE Wire
// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN -1 // Set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN -1   // Set to any GPIO pin to read end-of-conversion by pin

Adafruit_MPRLS mpr_0 = Adafruit_MPRLS(RESET_PIN, EOC_PIN); // MPRLS (pressure sensor 1) declaration
/// changed mpr to mpr_0 and added mpr_1
Adafruit_MPRLS mpr_1 = Adafruit_MPRLS(RESET_PIN, EOC_PIN); // MPRLS (pressure sensor 2) declaration
MPU6050 accel;                                            // MPU6050 (accelerometer) declaration

File myFile; // SD card file
File SafeFile; // SD card file for caution
const int chipSelect = BUILTIN_SDCARD;

// Variables initiated for code algorithm //
int state = 0;
bool success = 0;
int count = 0;
long start_time; // Time keeping variables
long current_time = 0;
long launch_time;
long burnout_time;
long land_time;
long last_time = 0;
long Time = 0;
long Period = 0;
int16_t ax, ay, az, gx, gy, gz; // Acceleration and Gyroscope varibles from the MPU6050
int16_t Temp;
float axf, ayf, azf, gxf, gyf, gzf;
float pressure_hPa_0, pressure_hPa_1; // MPRLS (pressure sensors) variables

/// added tcaselect function which acts as switch between sensors
#define TCAADDR 0x70
void tcaselect (uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
  //return;
}

void PrintToFile()
{ // function used to log data to the file in the micro SD card
    myFile = SD.open("Data.csv", FILE_WRITE);

  if (count == 0){ 
  /// changed pressure variable and added second variable to print lines
    myFile.println((String) "Time,Pressure1,Pressure2,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Temperature"); // creating the table for data values
    myFile.println((String) "[s],[lbf/in2],[lbf/in2],[m/s2],[m/s2],[m/s2],[°/s],[°/s],[°/s],[°C]");
    count = 1;
  }          
  current_time /= 1000;
  myFile.println((String)current_time + "," + pressure_hPa_0 + "," + pressure_hPa_1 + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  //Serial.println((String)current_time + "," + pressure_hPa_0 + "," + pressure_hPa_1 + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  current_time *= 1000;
  myFile.close();
}
void PrintToSafeFile()
{ // function used to log data to the safe file in the micro SD card
  SafeFile = SD.open("SafeData.csv", FILE_WRITE);
  current_time /= 1000;
  /// changed pressure variable and added second variable to print lines
  SafeFile.println((String)current_time + "," + pressure_hPa_0 + "," + pressure_hPa_1 + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  Serial.println((String)current_time + "," + pressure_hPa_0 + "," + pressure_hPa_1 + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  current_time *= 1000;
  SafeFile.close();
}
void DataCollection()
{                            // Function to collect data from sensors and process them for file writing
    current_time = millis(); // Time of data collection

    Period = current_time - last_time; // Period is the time in milli-seconds since last data collection

  /// implemented tcaselect and added info for second pressure sensor
  tcaselect(0);
    pressure_hPa_0 = mpr_0.readPressure();          // Pressure data
    pressure_hPa_0 = pressure_hPa_0 / 68.947572932; //

  tcaselect(1);
  pressure_hPa_1 = mpr_1.readPressure();          // Pressure data
  pressure_hPa_1 = pressure_hPa_1 / 68.947572932; //

  tcaselect(2);
    accel.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // Initial accelerometer data

    // Convert raw acccelerometer data to acceleration in m/s^2
    axf = ax / 2048 * 9.81;
    ayf = ay / 2048 * 9.81;
    azf = (az / 2048 * 9.81) - 9.81; // Remove acceleration due to gravity from data
    // Convert raw gyroscope data to angles
    gxf = gx / 131;
    gyf = gy / 131;
    gzf = gz / 131;

    Temp = accel.getTemperature(); // get temperature from MPU6050

    Temp = (Temp / 340.00) + 36.53; // convert temperature to celcius

    PrintToSafeFile(); // Every time the program collects data from sensors, the data is written into the safe file in the micro SD card
}


void setup()
{ // Setup of the program (SD card file setup)
    
      Serial.begin(9600);
      //File myFile = SD.open("test.txt", FILE_WRITE);
      while (!Serial) {
        ; // wait for serial port to connect.
      }
      WIRE.begin();
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("SD card initialization failed!");
        
    }
    Serial.println("Initializing devices...");
    
        tcaselect(0);
        if (!mpr_0.begin())
        {
            Serial.println("Failed to communicate with MPRLS sensor 0, check wiring.");
        }
        tcaselect(1);
        if (!mpr_1.begin())
        {
            Serial.println("Failed to communicate with MPRLS sensor 1, check wiring.");
        }
        tcaselect(2);
        accel.initialize();
        
        success = accel.testConnection();
        if (success)
        {
            accel.setXAccelOffset(0); // Offset of accelerometer data
            accel.setYAccelOffset(0);
            accel.setZAccelOffset(0);
            accel.setXGyroOffset(0);
            accel.setYGyroOffset(0);
            accel.setZGyroOffset(0);
            state = 10; // Connections to sensors are good, change to pre-launch state
            Serial.println("MPU6050 successful connection and calibration!");
            Serial.println("MPRLS successful connection!");
        }
        else
        {
            Serial.println("MPU6050 connection failed! Acceleration will not be logged!");
            
        }
    Serial.println("initialization done.");
     
    start_time = millis(); // Saving the starting
    
    state = 10;
    
}
void loop()
{ //Main program function, start with state = 0 (loop case 0)
    switch (state)
    {
    case 10: //Pre-launch initiation
        last_time = current_time;
        DataCollection();  // Collect data from sensor
        if (count == 0)                                                                                  
        { // Write to SD card only the initial data values from sensors
            PrintToFile();
            count = 1; //Increment count by 1 to not log initial data values again
        }

        if (abs(axf) < 5) Time = 0; break;
        if (abs(axf) >= 5 /*|| Serial.read() > -1*/) Time += Period;
        if (abs(axf) >= 5 && Time >= 200)
        { // 0.2 SECS OF XACCEL OVER 5 METER/SECSQUARED
            launch_time = current_time;
            myFile.println(" Launch at ");
            myFile.print(launch_time / 1000);
            myFile.print("[sec]");
            count = 0;
            state = 50;
            Time = 0;
            Period = 0;
        }
        break;
    case 50: // Post-launch and pre-burnout stage
        last_time = current_time;
        DataCollection(); // Collect data from sensors
        PrintToFile();      // Log data into the micro SD card

        if (abs(axf) > 2) Time = 0; break;
        if (abs(axf) <= 2) Time += Period;
        if (abs(axf) <= 2 && Time >= 300)
        {
            burnout_time = current_time - launch_time;
            myFile.print("Burnout at ");
            myFile.print(burnout_time / 1000);
            myFile.print("[sec] from launch.");
            count = 0; //reset counter for algorithm
            state = 100;
            Time = 0;
            Period = 0;
        }
        break;

    case 100: // Identify when landing occurs and end datalogging
        last_time = current_time;
        DataCollection(); // Collect data from sensors
        PrintToFile();     // Write data to SD card

        if (abs(axf) > 0.3) Time = 0; break;
        if (abs(axf) <= 0.3 && abs(ayf) <= 0.3 && abs(axf) <= 0.3) Time += Period;
        if (abs(axf) <= 0.3 && abs(ayf) <= 0.3 && abs(axf) <= 0.3 && Time >= 500)
        {
            land_time = current_time - launch_time;
            myFile.print("Land at ");
            myFile.print(land_time / 1000);
            myFile.print("[sec] from launch.");
            state = 101;
        }
        break;
    case 101: // Datalogging end
        DataCollection(); // ensure we have data being captured if algorithm fails
        break;
    }
}
