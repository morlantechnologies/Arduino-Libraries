/* 
 *  ===============================================
  Example sketch using the Intel CurieIMU library and the General Vision CurieNeurons library 
  for Intel(R) Curie(TM) devices

  Motion [ax,ay,az,gx,gy,gz] is converted into a simple vector by the extractFeatureVector function.
  Feel free to modify this function and test more sophisticated feature extrcations 

  After calibration is made,
  Start moving the board along the vertical direction,
  Use the serial monitor to enter "1", or the arbitrary code for the "vertical" category
  Learning occurs when you press Enter
  Start moving the board along the horizontal direction,
  Use the serial monitor to enter "2", or the arbitrary code for the "horizontal" category
  Learning occurs when you press Enter
  
  Note that this "snapshot" approach is simplistic and you will have to teach a given motion
  several times so the neurons store models with different amplitudes, acceleration, etc.
  For example, you will have to teach vertical Up-Down and Down-Up, etc

  Please understand that this example is very academic and assemble a pattern which
  should be more sophisticated for real-life system taking a calibration into account,
  integrating a sampling rate adequate for the type of motion and profiling the waveforms
  more selectively (distances between peaks and zero crossing, etc)
  
  ===============================================
*/
#include "CurieIMU.h"

int ax, ay, az;         // accelerometer values
int gx, gy, gz;         // gyrometer values

int calibrateOffsets = 1; // int to determine whether calibration takes place or not

#include <CurieNeurons.h>
CurieNeurons hNN;

int catL=0; // category to learn
int prevcat=0; // previously recognized category
int dist, cat, nid, nsr, ncount; // response from the neurons

//
// Variables used for the calculation of the feature vector
//
#define sampleNbr 10  // number of samples to assemble a vector
#define signalNbr  6  // ax,ay,az,gx,gy,gz
int raw_vector[sampleNbr*signalNbr]; // vector accumulating the raw sensor data
byte vector[sampleNbr*signalNbr]; // vector holding the pattern to learn or recognize
int mina=0xFFFF, maxa=0, ming=0xFFFF, maxg=0, da=0, dg=0;

void setup() 
{
  Serial.begin(9600); // initialize Serial communication
  while (!Serial);    // wait for the serial port to open

  // initialize device
  Serial.println("Initializing IMU device...");
  CurieIMU.begin();

  // use the code below to calibrate accel/gyro offset values
  if (calibrateOffsets == 1) 
  {    
    Serial.println("About to calibrate. Make sure your board is stable and upright");
    delay(5000);
    Serial.print("Starting Gyroscope calibration and enabling offset compensation...");
    CurieIMU.autoCalibrateGyroOffset();
    Serial.println(" Done");
    Serial.print("Starting Acceleration calibration and enabling offset compensation...");
    CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
    CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
    Serial.println(" Done");

    CurieIMU.setAccelerometerRange(8);
    CurieIMU.setGyroRange(1000);
  }
  // Initialize the neurons and set a conservative Max Influence Field
  if (hNN.begin()==0) Serial.print("\nNeural network is initialized!");
  else Serial.print("\nNeural network is NOT properly connected!");
  hNN.forget(500); //set a conservative  Max Influence Field prior to learning

  Serial.print("\n\nEntering loop...");
  Serial.print("\nMove the module vertically or horizontally...");
  Serial.print("\ntype 1 + Enter if vertical motion");
  Serial.print("\ntype 2 + Enter if horizontal motion");
  Serial.print("\ntype 0 + Enter for any other motion");
}

void loop() 
{   
    // Learn if push button depressed and report if a new neuron is committed
    //
    // wait for a keyboard input of 1 digit between 0-2
    // WARNING: make sure the serial printer settings is "new line"
    //
    if (Serial.available() == 2) 
    {
      catL = Serial.read();
      Serial.read(); // to empty serial buffer of the newline char
      catL = catL - 48;
      if (catL<3) //expected category input (1-vertical, 2-horizontal, 0-still)
      {
        Serial.print("\nLearning motion category "); Serial.print(catL);
        //
        // learn 5 consecutive sample vectors
        // (make sure to keep moving the 101 board accordingly)
        //
        for (int i=0; i<5; i++)
        {
          extractFeatureVector(); // the vector array is a global
          //Optional display of the vector in the serial monitor
          //Serial.print("\nVector = ");
          //for (int i=0; i<signalNbr*sampleNbr; i++) {Serial.print(vector[i]);Serial.print("\t");}
          ncount=hNN.learn(vector, sampleNbr*signalNbr, catL);
        }
        Serial.print("\tNeurons="); Serial.print(ncount);
      }    
    }
    else
    {
      // Recognize
      extractFeatureVector(); // the vector array is a global
      hNN.classify(vector, sampleNbr*signalNbr,&dist, &cat, &nid);
      if (cat!=prevcat)
      {
        if (cat!=0x7FFF)
        {
          Serial.print("\nMotion category #"); Serial.print(cat);
        }
        else Serial.print("\nMotion unknown");      
        prevcat=cat;
      }
    }
}  

//  The following function is very academic and assemble a pattern which
//  should be more sophisticated for real-life system taking a calibration into account,
//  integrating a sampling rate adequate for the type of motion and profiling the waveforms
//  more selectively (distances between peaks and zero crossing, etc)

void extractFeatureVector()
{
  // sensor output [ax,ay,az,gx, gy,gz] is converted into a byte array as follows:
  // [ax'1, ay'1, az'1, gx'1,gy'1, gz'1, ax'2, ay'2, az'2, gx'2, gy'2, gz'2, ...] over a number of time samples.
  // a' and g' are normalized using their respective min and max values.
  //
  // the reset of the min and max values is optional depending if you want to
  // use a running min and max from the launch of the script or not
  mina=0xFFFF, maxa=0, ming=0xFFFF, maxg=0, da=0, dg=0;
  
  for (int sampleId=0; sampleId<sampleNbr; sampleId++)
  {
    //Build the vector over sampleNbr and broadcast to the neurons
    CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);
    
    // update the running min/max for the a signals
    if (ax>maxa) maxa=ax; else if (ax<mina) mina=ax;
    if (ay>maxa) maxa=ay; else if (ay<mina) mina=ay;
    if (az>maxa) maxa=az; else if (az<mina) mina=az;    
    da= maxa-mina;
    
    // update the running min/max for the g signals
    if (gx>maxg) maxg=gx; else if (gx<ming) ming=gx;
    if (gy>maxg) maxg=gy; else if (gy<ming) ming=gy;
    if (gz>maxg) maxg=gz; else if (gz<ming) ming=gz;   
    dg= maxg-ming;

    // accumulate the sensor data
    raw_vector[sampleId*signalNbr]= ax;
    raw_vector[(sampleId*signalNbr)+1]= ay;
    raw_vector[(sampleId*signalNbr)+2]= az;
    raw_vector[(sampleId*signalNbr)+3]= gx;
    raw_vector[(sampleId*signalNbr)+4]= gy;
    raw_vector[(sampleId*signalNbr)+5]= gz;
  }
  
  // normalize vector
  for(int sampleId=0; sampleId < sampleNbr; sampleId++)
  {
    for(int i=0; i<3; i++)
    {
      vector[sampleId*signalNbr+i]  = (((raw_vector[sampleId*signalNbr+i] - mina) * 255)/da) & 0x00FF;
      vector[sampleId*signalNbr+3+i]  = (((raw_vector[sampleId*signalNbr+3+i] - ming) * 255)/dg) & 0x00FF;
    }
  }
}
