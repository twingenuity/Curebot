// This #include statement was automatically added by the Particle IDE.
#include "lib1.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunRHT03/SparkFunRHT03.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_SI1145/Adafruit_SI1145.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunPhant/SparkFunPhant.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMAX17043/SparkFunMAX17043.h"



/******************************************************************************



    RTH03 Humidity/Temp Sensor Photon
        VCC (Red) ------------- 3.3V (VCC)
        GND (Black) ----------- GND
        SIG (White) ----------- D4

    SI1145 UV/Infrared/Visible Light Sensor
    from Adafruit.com
        VCC ------------------- 3.3V (VCC)
        GND ------------------- GND
        SDA ------------------- D0
        SCL ------------------- D1



  Development environment specifics:
  	IDE: Particle Dev
  	Hardware Platform: Particle Photon
                       Particle Core



*******************************************************************************/

#define RHT03_DATA_PIN D4
//#define SOIL_MOIST A1
//#define SOIL_MOIST_POWER D5

Adafruit_SI1145 uv = Adafruit_SI1145();
RHT03 rht;

PRODUCT_ID(2312);  
PRODUCT_VERSION(1);  

//Global Variables
String device_uid = ""; // photon hard coded device id, UID
double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
float humidity = 0;//humidity from rht03
float tempC = 0;// temperature in C from RHT03
float tempF = 0;// temperature in F from RHT03
float UVIndex = 0;
int VisibleLight = 0;
int IRLight = 0;
//float soilmoisture = 0;//used for percent soil moisture
//int soilMoisture = 0;//used for integer reading from soil moisture A1
int count = 0;//This triggers a post and print on the first time through the loop


////////////PHANT STUFF//////////////////////////////////////////////////////////////////
//const char server[] = "data.sparkfun.com";
//const char publicKey[] = "NJ33XWba67HajGqpQXYr";
//const char privateKey[] = "5dNN5Pm6RwT79VRvmezN";
//Phant phant(server, publicKey, privateKey);
/////////////////////////////////////////////////////////////////////////////////////////


////////////PHANT STUFF//////////////////////////////////////////////////////////////////
const char server[] = "ec2-52-40-13-117.us-west-2.compute.amazonaws.com";
const char path[] = "api/metrics/cure"; //old = QGyxppE3dKFd4RrmbrXg
const char port[] = "8080"; // old = Jqyx44PGleUzWRr2Mr4A
//String myIDStr = "notsetyet";
PhantRest phant(server, path, port);
///////////////////////////////////////////////////////////////////////////////////////// 

//---------------------------------------------------------------
void setup()
{
    Serial.begin(9600);   // open serial over USB
    
    device_uid.reserve(30);
      

    { /// get deviceID. code block isolated in brackets
      device_uid=System.deviceID();
      Serial.print("Deviceid: ");
      Serial.println(device_uid);
    }    
    // RHT03 initializtion
    rht.begin(RHT03_DATA_PIN);
    //rht.update();

  
    Serial.println("SI1145 Begin");

    while(! uv.begin()) {
    Serial.println("Didn't find SI1145");
    delay(1000);
  }

  Serial.println("SI1145_OK!");

  // Set up PArticle variables (voltage, soc, and alert):
	Particle.variable("voltage", &voltage, DOUBLE);
	Particle.variable("soc", &soc, DOUBLE);

	// To read the values from a browser, go to:
	// http://api.particle.io/v1/devices/{DEVICE_ID}/{VARIABLE}?access_token={ACCESS_TOKEN}

	// Set up the MAX17043 LiPo fuel gauge:
	lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

	// Quick start restarts the MAX17043 in hopes of getting a more accurate
	// guess for the SOC.
	lipo.quickStart();

	// We can set an interrupt to alert when the battery SoC gets too low.
	// We can alert at anywhere between 1% - 32%:
	lipo.setThreshold(20); // Set alert threshold to 20%.

}
//---------------------------------------------------------------
void loop()
{
    // Call rht.update() to get new humidity and temperature values from the sensor.
	int updateRet = rht.update();
	
	// If successful, the update() function will return 1.
	// If update fails, it will return a value <0
	if (updateRet == 1)
	{
		// The humidity(), tempC(), and tempF() functions can be called -- after 
		// a successful update() -- to get the last humidity and temperature
		// value 
		float humidity = rht.humidity();
		float tempC = rht.tempC();
		float tempF = rht.tempF();
		
		// Now print the values:
		Serial.println("Humidity: " + String(humidity, 1) + " %");
		Serial.println("Temp (F): " + String(tempF, 1) + " deg F");
		Serial.println("Temp (C): " + String(tempC, 1) + " deg C");
	}
	else
	{
		// If the update failed, try delaying for RHT_READ_INTERVAL_MS ms before
		// trying again.
		delay(RHT_READ_INTERVAL_MS);
	}
	
	delay(2000);

    getSensorData();//Get readings from all sensors

    printInfo();//print readings to serial line
    postToPhant();//upload data to Phant

   delay(20000);//stay awake for 20 seconds to allow for App updates
    //Power down between sends to save power, measured in seconds.
   System.sleep(SLEEP_MODE_DEEP, 120);  //for Particle Photon 2 minute
    //(includes 20 sec update delay) between postings-change this to alter update rate
   
}

//---------------------------------------------------------------
void printInfo()
{
  //This function prints the sensor data out to the default Serial Port
      Serial.print("Humidity:");
      Serial.print(humidity);
      Serial.print("H, ");
      Serial.print("Temp C:");
      Serial.print(tempC);
      Serial.print("C, ");
      Serial.print("Temp F:");
      Serial.print(tempF);
      Serial.print("F, ");
      Serial.print("UV_Index:");
      Serial.print(UVIndex);
      Serial.print(", ");
      Serial.print("Infrared Light:");
      Serial.print(IRLight);
      Serial.print(", ");
      Serial.print("Visible Light:");
      Serial.print(VisibleLight);
      Serial.print(", ");
      Serial.print("Voltage: ");
	  Serial.print(voltage);  // Print the battery voltage
	  Serial.println(" V");
      Serial.print("Percentage: ");
      Serial.print(soc); // Print the battery state of charge
      Serial.println(" %");
	  Serial.println();

}
//---------------------------------------------------------------
void getLight()
{
  Adafruit_SI1145 uv = Adafruit_SI1145();
  UVIndex = uv.readUV();
  // the index is multiplied by 100 so to get the
  // integer index, divide by 100!
  UVIndex /= 100.0;//standard UV Index number

  VisibleLight = uv.readVisible();
  IRLight = uv.readIR();
}

//---------------------------------------------------------------

//---------------------------------------------------------------

//---------------------------------------------------------------

//---------------------------------------------------------------
void getrht()
{
    rht.update();
    humidity = rht.humidity(); // Read humidity into a variable
    tempC = rht.tempC(); // Read celsius temperature into a variable
    tempF = rht.tempF(); // Read farenheit temperature into a variable
}

//---------------------------------------------------------------
void getSensorData()
{

    getrht();//Read the RHT03 humidity temp sensor
    getLight();//Read UV,Visible and IR light intensity
    getBattery();//Updates battery voltage, state of charge and alert threshold



}

//---------------------------------------------------------------
void getBattery()
{
  // lipo.getVoltage() returns a voltage value (e.g. 3.93)
voltage = lipo.getVoltage();
// lipo.getSOC() returns the estimated state of charge (e.g. 79%)
soc = lipo.getSOC();


}
//---------------------------------------------------------------

//---------------------------------------------------------------
int postToPhant()//sends datat to data.sparkfun.com
{
    
    rht.update();
    
    phant.add("battery", soc);
    phant.add("deviceid", device_uid);
    phant.add("infrared", IRLight);
    phant.add("uvindex", UVIndex);
    phant.add("visible", VisibleLight);
    phant.add("humidity", rht.humidity(), 1); 
    phant.add("tempc", rht.tempC(), 1); 
    phant.add("tempf", rht.tempF(), 1);
    phant.add("volts", voltage);

    TCPClient client;
    char response[512];
    int i = 0;
    int retVal = 0;

    if (client.connect(server, 8080))
    {
        Serial.println("Posting!");
        client.print(phant.post());
        delay(1000);
        while (client.available())
        {
            char c = client.read();
            Serial.print(c);
            if (i < 512)
                response[i++] = c;
        }
        if (strstr(response, "200 OK"))
        {
            Serial.println("Post success!");
            retVal = 1;
        }
        else if (strstr(response, "400 Bad Request"))
        {
            Serial.println("Bad request");
            retVal = -1;
        }
        else
        {
            retVal = -2;
        }
    }
    else
    {
        Serial.println("connection failed");
        retVal = -3;
    }
    client.stop();
    return retVal;

}
