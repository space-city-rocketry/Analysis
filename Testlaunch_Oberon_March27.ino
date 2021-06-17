#include <Wire.h>           // Library for communication with I2C devices using the SDA (data line) and SCL (clock line)
#include <Adafruit_MPRLS.h> // Library for the MPRLS (pressure sensor)
#include <MPU6050.h>        // Library for the MPU6050 (accelerometer)
#include <SD.h>             // Library for SD card communication
#include <SPI.h>            // Library for communication with SPI devices (teensy being the master device)

// You dont *need* a reset and EOC pin for most uses, so we set to -1 and don't connect
#define RESET_PIN -1 // Set to any GPIO pin # to hard-reset on begin()
#define EOC_PIN -1   // Set to any GPIO pin to read end-of-conversion by pin

Adafruit_MPRLS mpr = Adafruit_MPRLS(RESET_PIN, EOC_PIN); // MPRLS (pressure sensor) declaration
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
float pressure_hPa; // MPRLS (pressure sensor) variable

void PrintToFile()
{ // function used to log data to the file in the micro SD card
    myFile = SD.open("Data2.txt", FILE_WRITE);

  if (count == 0){ 
    myFile.println((String) "Time,Pressure,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Temperature"); // creating the table for data values
    myFile.println((String) "[ms],[lbf/in2],[m/s2],[m/s2],[m/s2],[°/s],[°/s],[°/s],[°C]");
    count = 1;
  }          
  myFile.println((String)current_time + "," + pressure_hPa + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  myFile.close();
}
void PrintToSafeFile()
{ // function used to log data to the safe file in the micro SD card
  SafeFile = SD.open("SafeData2.csv", FILE_WRITE);
  SafeFile.println((String)current_time + "," + pressure_hPa + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  Serial.println((String)current_time + "," + pressure_hPa + "," + axf + "," + ayf + "," + azf + "," + gxf + "," + gyf + "," + gzf + "," + Temp);
  SafeFile.close();
}
void DataCollecttion()
{                            // Function to collect data from sensors and process them for file writing
    current_time = millis(); // Time of data collection

    Period = current_time - last_time; // Period is the time in milli-seconds since last data collection

    pressure_hPa = mpr.readPressure();          // Pressure data
    pressure_hPa = pressure_hPa / 68.947572932; //

    accel.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // Initial accelerometer data

    // Convert raw acccelerometer data to acceleration in m/s^2
    axf = ax / 16384.0 * 9.81;
    ayf = ay / 16384.0 * 9.81;
    azf = (az / 16384.0 * 9.81) - 9.81; // Remove acceleration due to gravity from data
    // Convert raw gyroscope data to angles
    gxf = gx / 131.0;
    gyf = gy / 131.0;
    gzf = gz / 131.0;

    Temp = accel.getTemperature(); // get temperature from MPU6050

    Temp = (Temp / 340.00) + 36.53; // convert temperature to celcius

    PrintToSafeFile(); // Every time the program collects data from sensors, the data is written into the safe file in the micro SD card
}


void setup()
{ // Setup of the program (SD card file setup)
    
      Serial.begin(9600);
      while (!Serial) {
        ; // wait for serial port to connect.
      }
    
    if (!SD.begin(BUILTIN_SDCARD))
    {
        Serial.println("initialization failed!");
        
    }
    Serial.println("initialization done.");
    
    start_time = millis(); // Saving the starting
    
    state = 0;
    
}
void loop()
{ //Main program function, start with state = 0 (loop case 0)
    switch (state)
    {
    case 0: //check connections of sensors <> Make functiomns out of cases

        Serial.println("Initializing devices...");

        if (!mpr.begin())
        {
            Serial.print("Failed to communicate with MPRLS sensor, check wiring.");
            while (1)
            {
                delay(10);
            }
        }

        accel.initialize();
        success = accel.testConnection();
        if (success)
        {
            accel.setXAccelOffset(-62); // Offset of accelerometer data
            accel.setYAccelOffset(-1101);
            accel.setZAccelOffset(980);
            accel.setXGyroOffset(32);
            accel.setYGyroOffset(-11);
            accel.setZGyroOffset(30);
            state = 10; // Connections to sensors are good, change to pre-launch state
            Serial.println("MPU6050 successful connection and calibration!");
            Serial.println("MPRLS successful connection!");
        }
        else
        {
            Serial.println("MPU6050 connection failed! Acceleration will not be logged!");
        }

        break;
    case 10: //Pre-launch initiation
        last_time = current_time;
        DataCollecttion();  // Collect data from sensor
        if (count == 0)                                                                                  
        { // Write to SD card only the initial data values from sensors
            PrintToFile();
            count = 1; //Increment count by 1 to not log initial data values again
        }

        if (abs(axf) <2) Time = 0;
        if (abs(axf) >= 2 /*|| Serial.read() > -1*/) Time += Period;
        if (abs(axf) >= 2 && Time >= 200)
        { // 0.2 SECS OF XACCEL OVER 5 METER/SECSQUARED
            launch_time = current_time;
            myFile.println(" Launch at ");
            myFile.print(launch_time / 1000);
            myFile.print("[sec]");
            state = 50;
            Time = 0;
            Period = 0;
        }
        break;
    case 50: // Post-launch and pre-burnout stage
        last_time = current_time;
        DataCollecttion(); // Collect data from sensors
        PrintToFile();      // Log data into the micro SD card

        if (abs(axf) > 2) Time = 0;
        if (abs(axf) <= 2) Time += Period;
        if (abs(axf) <= 2 && Time >= 300)
        {
            burnout_time = current_time - launch_time;
            myFile.print("Burnout at ");
            myFile.print(burnout_time / 1000);
            myFile.print("[sec] from launch.");
            state = 100;
            Time = 0;
            Period = 0;
        }
        break;

    case 100: // Identify when landing occurs and end datalogging
        last_time = current_time;
        DataCollecttion(); // Collect data from sensors
        PrintToFile();     // Write data to SD card

        if (abs(axf) > 0.3) Time = 0;
        if (abs(axf) <= 0.3 && abs(ayf) <= 0.3 && abs(axf) <= 0.3) Time += Period;
        if (abs(axf) <= 0.3 && abs(ayf) <= 0.3 && abs(axf) <= 0.3 && Time >= 500)
        {
            land_time = current_time - launch_time;
            myFile.print("Land at ");
            myFile.print(land_time / 1000);
            myFile.print("[sec] from launch.");
            state = 101;
            // close file?
        }
        break;
    case 101: // Datalogging end
        DataCollecttion(); // ensure we have data being captured if algorithm fails
        break;
    }
}
