#include <SPI.h>
#include <SD.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin gyro definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>
#define MPU6050_ACCEL_XOUT_H       0x3B   // R  
#define MPU6050_ACCEL_XOUT_L       0x3C   // R  
#define MPU6050_GYRO_XOUT_H        0x43   // R  
#define MPU6050_GYRO_XOUT_L        0x44   // R  
#define MPU6050_PWR_MGMT_1         0x6B   // R/W
#define MPU6050_PWR_MGMT_2         0x6C   // R/W
#define MPU6050_WHO_AM_I           0x75   // R
#define MPU6050_I2C_ADDRESS 0x68

float sangle=0;
float anglefin=0;
typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  } reg;
  struct 
  {
    int x_accel;
    int y_accel;
    int z_accel;
    int temperature;
    int x_gyro;
    int y_gyro;
    int z_gyro;
  } value;
};

// Use the following global variables and access functions to help store the overall
// rotation angle of the sensor
unsigned long last_read_time;
float         last_x_angle;  // These are the filtered angles
float         last_y_angle;
float         last_z_angle;  
float         last_gyro_x_angle;  // Store the gyro angles to compare drift
float         last_gyro_y_angle;
float         last_gyro_z_angle;

void set_last_read_angle_data(unsigned long time, float x, float y, float z, float x_gyro, float y_gyro, float z_gyro) {
  last_read_time = time;
  last_x_angle = x;
  last_y_angle = y;
  last_z_angle = z;
  last_gyro_x_angle = x_gyro;
  last_gyro_y_angle = y_gyro;
  last_gyro_z_angle = z_gyro;
}

inline unsigned long get_last_time() {return last_read_time;}
inline float get_last_x_angle() {return last_x_angle;}
inline float get_last_y_angle() {return last_y_angle;}
inline float get_last_z_angle() {return last_z_angle;}
inline float get_last_gyro_x_angle() {return last_gyro_x_angle;}
inline float get_last_gyro_y_angle() {return last_gyro_y_angle;}
inline float get_last_gyro_z_angle() {return last_gyro_z_angle;}

float    base_x_accel;
float    base_y_accel;
float    base_z_accel;

float    base_x_gyro;
float    base_y_gyro;
float    base_z_gyro;


int read_gyro_accel_vals(uint8_t* accel_t_gyro_ptr) {
  
  accel_t_gyro_union* accel_t_gyro = (accel_t_gyro_union *) accel_t_gyro_ptr;
   
  int error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) accel_t_gyro, sizeof(*accel_t_gyro));

  uint8_t swap;
  #define SWAP(x,y) swap = x; x = y; y = swap

  SWAP ((*accel_t_gyro).reg.x_accel_h, (*accel_t_gyro).reg.x_accel_l);
  SWAP ((*accel_t_gyro).reg.y_accel_h, (*accel_t_gyro).reg.y_accel_l);
  SWAP ((*accel_t_gyro).reg.z_accel_h, (*accel_t_gyro).reg.z_accel_l);
  SWAP ((*accel_t_gyro).reg.t_h, (*accel_t_gyro).reg.t_l);
  SWAP ((*accel_t_gyro).reg.x_gyro_h, (*accel_t_gyro).reg.x_gyro_l);
  SWAP ((*accel_t_gyro).reg.y_gyro_h, (*accel_t_gyro).reg.y_gyro_l);
  SWAP ((*accel_t_gyro).reg.z_gyro_h, (*accel_t_gyro).reg.z_gyro_l);

  return error;
}

// The sensor should be motionless on a horizontal surface 
//  while calibration is happening
void calibrate_sensors() {
  int                   num_readings = 10;
  float                 x_accel = 0;
  float                 y_accel = 0;
  float                 z_accel = 0;
  float                 x_gyro = 0;
  float                 y_gyro = 0;
  float                 z_gyro = 0;
  accel_t_gyro_union    accel_t_gyro;
  
  
  // Discard the first set of values read from the IMU
  read_gyro_accel_vals((uint8_t *) &accel_t_gyro);
  
  // Read and average the raw values from the IMU
  for (int i = 0; i < num_readings; i++) {
    read_gyro_accel_vals((uint8_t *) &accel_t_gyro);
    x_accel += accel_t_gyro.value.x_accel;
    y_accel += accel_t_gyro.value.y_accel;
    z_accel += accel_t_gyro.value.z_accel;
    x_gyro += accel_t_gyro.value.x_gyro;
    y_gyro += accel_t_gyro.value.y_gyro;
    z_gyro += accel_t_gyro.value.z_gyro;
    delay(100);
  }
  x_accel /= num_readings;
  y_accel /= num_readings;
  z_accel /= num_readings;
  x_gyro /= num_readings;
  y_gyro /= num_readings;
  z_gyro /= num_readings;
  
  base_x_accel = x_accel;
  base_y_accel = y_accel;
  base_z_accel = z_accel;
  base_x_gyro = x_gyro;
  base_y_gyro = y_gyro;
  base_z_gyro = z_gyro;
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end gyro definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin current definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int currentPin = A1;
float Intensity= 0.0;
float intfin =0.0;

float adcValue = 0.0;
float currentValue = 0.0;
float offset_I = 0.0;
float real_I = 0.0;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end current definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin voltage definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int voltagePin = A2;
float voltage = 0.0;
float vol= 0.0;
float voltfin =0.0;
float voltfi =0.0;
float offset_V=0.0;
float adcValueV =0.0;
//
//int voltagePin_t = A4;
//float voltage_t = 0.0;
//float vol_t= 0.0;
//float voltfin_t =0.0;
//float voltfi_t =0.0;
//float offset_V_t=0.0;
//float adcValueV_t =0.0;
//float regen_on =0.0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end voltage definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin speed definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Pinspeed = 3;
float rpm;
volatile byte pulses;
unsigned long timeold;
float  pulseperturn = 8; //the number of pulses that we have per revolution
unsigned long timeaf;
unsigned long timebef;
float t0 = 0.0;
float t1 = 0.0;
float t=0;
void compteur(){
  t1=millis();
  if (t1-t0 > 12) {
    pulses++;
    t0=t1;
//    Serial.println("je suis dedans");
    
    }
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end speed definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin time definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long timeold2;
int N_loop = 1;
float N_max = 300;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end time definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin carte SD definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


File tot_file;


int pinCS = 53; // Pin 53 on Arduino Mega

int led=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end carte SD definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin pwm output definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int led_pin = 6;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end pwm output definition /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{      
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin gyro setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  int error;
  uint8_t c;
  // Initialize the 'Wire' class for the I2C-bus.
  Wire.begin();

  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);


  // Clear the 'sleep' bit to start the sensor.
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  
  //Initialize the angles
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end  gyro setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin  current setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  for (int i=0; i <= 999; i++){
    
    adcValue = float(analogRead(currentPin));
    offset_I = offset_I+adcValue;
  }
  offset_I = int(offset_I/1000.0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end  current setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin  voltage setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  for (int i=0; i <= 999; i++){
    
    adcValueV = analogRead(voltagePin);
    offset_V = offset_V+adcValueV;
  }
  offset_V = int(offset_V/1000.0);

//  for (int i=0; i <= 999; i++){
//    
//    adcValueV_t = analogRead(voltagePin_t);
//    offset_V_t = offset_V_t+adcValueV_t;
//  }
//  offset_V_t = int(offset_V_t/1000.0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end  voltage setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin  speed setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
pulses = 0;
rpm =0;
timeold = 0;

pinMode(Pinspeed,INPUT);
attachInterrupt(digitalPinToInterrupt(Pinspeed),compteur,RISING);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end speed setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin carte SD setup /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pinMode(5,OUTPUT); // pour la led

  
  // SD Card Initialization
  if (SD.begin())
  {
//    Serial.println("SD card is ready to use.");
    led=1;
  } else
  {
    //Serial.println("SD card initialization failed");
    led=0;
    return;
  }
  // close all the existing files
  SD.remove("one_file.txt");
  // Create/Open file 

  tot_file = SD.open("one_file.txt", FILE_WRITE);
  tot_file.close();
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end carte SD/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin pwm control setup/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

pinMode(led_pin, OUTPUT);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end pwm control  setup/////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Intensity= 0.0;
    intfin =0.0;
    adcValue = 0.0;
    currentValue = 0.0;
    real_I = 0.0;
    voltage = 0.0;
    vol= 0.0;
    voltfin =0.0;
    voltfi =0.0;
    adcValueV =0.0;
}

//////////// I hate arduino ////////////////
float offset_2V=0.0;
float offset_2V_t=0.0;
float offset_2I =0.0;
float RPM_out =0.0;
bool first_loop = true;
char myChar = ' ';
void loop()
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin gyro void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  int error;
  double dT;
  accel_t_gyro_union accel_t_gyro;
  
  // Read the raw values.
  error = read_gyro_accel_vals((uint8_t*) &accel_t_gyro);
  
  // Get the time of reading for rotation computations
  unsigned long t_now = millis();


  // Convert gyro values to degrees/sec
  float FS_SEL = 131;

  float gyro_x = (accel_t_gyro.value.x_gyro - base_x_gyro)/FS_SEL;
  float gyro_y = (accel_t_gyro.value.y_gyro - base_y_gyro)/FS_SEL;
  float gyro_z = (accel_t_gyro.value.z_gyro - base_z_gyro)/FS_SEL;
  
  
  // Get raw acceleration values
  //float G_CONVERT = 16384;
  float accel_x = accel_t_gyro.value.x_accel;
  float accel_y = accel_t_gyro.value.y_accel;
  float accel_z = accel_t_gyro.value.z_accel;
  
  // Get angle values from accelerometer
  float RADIANS_TO_DEGREES = 180/3.14159;
  float accel_vector_length = sqrt(pow(accel_x,2) + pow(accel_y,2) + pow(accel_z,2));
  float accel_angle_y = atan2(-1*accel_x,sqrt(pow(accel_y,2) + pow(accel_z,2)))*RADIANS_TO_DEGREES;
  float accel_angle_x = atan2(accel_y,sqrt(pow(accel_x,2) + pow(accel_z,2)))*RADIANS_TO_DEGREES;

  float accel_angle_z = 0;
  
  // Compute the (filtered) gyro angles
  float dt =(t_now - get_last_time())/1000.0;
  float gyro_angle_x = gyro_x*dt + get_last_x_angle();
  float gyro_angle_y = gyro_y*dt + get_last_y_angle();
  float gyro_angle_z = gyro_z*dt + get_last_z_angle();
  
  // Compute the drifting gyro angles
  float unfiltered_gyro_angle_x = gyro_x*dt + get_last_gyro_x_angle();
  float unfiltered_gyro_angle_y = gyro_y*dt + get_last_gyro_y_angle();
  float unfiltered_gyro_angle_z = gyro_z*dt + get_last_gyro_z_angle();
  
  // Apply the complementary filter to figure out the change in angle - choice of alpha is
  // estimated now.  Alpha depends on the sampling rate...
  float alpha = 0.98;
  float angle_x = alpha*gyro_angle_x + (1.0 - alpha)*accel_angle_x;
  float angle_y = alpha*gyro_angle_y + (1.0 - alpha)*accel_angle_y;
  float angle_z = gyro_angle_z;  //Accelerometer doesn't give z-angle
  
  // Update the saved data with the latest values
  set_last_read_angle_data(t_now, angle_x, angle_y, angle_z, unfiltered_gyro_angle_x, unfiltered_gyro_angle_y, unfiltered_gyro_angle_z);
  



// test of second way computing it ///

  

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end gyro void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin current void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
adcValue = float(analogRead(currentPin));
Intensity = abs(adcValue-float(offset_I)); // 5.6 is an existing offset that is staying
intfin= intfin + Intensity;
    
if (N_loop == N_max){
  real_I = intfin/N_max*0.0723/0.83-offset_2I;
  intfin=0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end current void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin voltage void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
voltage = analogRead(voltagePin);
vol=(voltage-float(offset_V))*50/(1025);
voltfi=vol+voltfi;
    
    
if (N_loop == N_max){
     voltfin=voltfi/N_max/37.7*41-offset_2V ;
     voltfi=0.0; 
}
//
//voltage_t = analogRead(voltagePin_t);
//vol_t=(voltage_t-float(offset_V_t))*5/(1025);
//voltfi_t=vol_t+voltfi_t;
    
    
//if (N_loop == N_max){
//     voltfin_t=voltfi_t/N_max-offset_2V_t ;
//     voltfi_t=0.0; 
//     if (voltfin_t > 1.0)
//     {
//      regen_on = 0.0;
//      }
//     else{
//      regen_on = 1.0;
//      }
//}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end voltage void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin speed void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//moved in next section
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end speed void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin time and writing in the file /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





if (N_loop == N_max){

float instant = millis()- timeold2;
timeold2 = millis();

// add of the speed before opening file
timeaf = millis();
detachInterrupt(digitalPinToInterrupt(Pinspeed));
//Serial.println(timeaf-timebef);
t=float(timeaf-timebef);
rpm = float(pulses)*60.0*1000.0/float(pulseperturn)/t/2;
RPM_out = rpm;
// actually writing in the file
//String stringinstant = (String)t;
//tot_file.print(stringinstant);
//tot_file.print(" ");
//String stringrpm = (String)rpm;
//tot_file.print(stringrpm);
//tot_file.print(" ");
//String stringvoltage = (String)voltfin;
//tot_file.print(stringvoltage);
//tot_file.print(" ");
//String stringintensity = (String)real_I;
//tot_file.print(stringintensity);
//tot_file.print(" ");
//String stringslope = (String)(angle_y-52.2);
//tot_file.println(stringslope);
if (first_loop) {
  offset_2V= voltfin;
//  offset_2V_t= voltfin_t;
  offset_2I =real_I;
  first_loop = false;
  tot_file = SD.open("one_file.txt", FILE_WRITE);
  if ( led==1){
  digitalWrite(5,HIGH);}
    
}

tot_file.print(instant);
tot_file.print(myChar);
tot_file.print(rpm);
tot_file.print(myChar);
tot_file.print(voltfin);
tot_file.print(myChar);
tot_file.print(real_I);
tot_file.print(myChar);
tot_file.println(angle_y-2.6);
//tot_file.print(myChar);
//tot_file.print(voltfin_t);
//tot_file.print(myChar);
//tot_file.println(regen_on);
tot_file.flush();
//tot_file.close();
N_loop = 1;

//Serial.println(1000000, 2);
////Serial.println(voltfin_t/30*41, 2);
////Serial.println(regen_on, 2);
//Serial.println(RPM_out/401.916*250, 2);
//Serial.println(voltfin, 2);
//Serial.println(Intensity, 2);
//
//Serial.println(real_I, 2);
//Serial.println(angle_y-2.6, 2);



// launching speed process
attachInterrupt(digitalPinToInterrupt(Pinspeed),compteur,RISING);
timebef=millis();
pulses=0;
}
else{
  N_loop = N_loop +1;
  }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end time void and writing in the file ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin pwm control void //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  analogWrite(led_pin, (RPM_out*0.1244/50*255)); // in km per hour with a max equal to 5 volts at 50 km/h
  analogWrite(9,109);
    //analogWrite(led_pin, (RPM_out*2*3.14*0.33*3.6)/(60)*255/500 ); // in km per hour 
    //analogWrite(led_pin, RPM_out*255/500);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end pwm control void /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



}






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// begin gyro external functions /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++]=Wire.read();
  }
  if ( i != size)
    return (-11);

  return (0);  // return : no error
}


int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);         // return : no error
}

int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;

  error = MPU6050_write(reg, &data, 1);

  return (error);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// end gyro external functions /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////