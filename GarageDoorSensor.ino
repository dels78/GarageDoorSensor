/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Martin Delisle
 * 
 * DESCRIPTION
 * This is my Garage Door sensor using a distance sensor to determine if the door is open
 * 
 * Based off of:
 * http://www.mysensors.org/build/distance
 */

#include <SPI.h>
#include <MySensor.h>  
#include <NewPing.h>


//**** SET THOSE ****
#define CHILD_ID 1      // This is my Garage door distance monitor
#define TRIGGER_PIN  6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
unsigned long SLEEP_TIME = 2000; // Sleep time between reads (in milliseconds)
int doorClosedDist = 115;  //distance to the door when closed (in inches)
int doorFullyOpenedDist = 32; //distance to the door when fully opened (in inches)
int doorStatus = 0; //0=closed, 1=fully opened, 3=opened but not fully
//**** END Settings ****

MySensor gw;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
MyMessage msg(CHILD_ID, V_DISTANCE);

int lastDist;
int lastStatus;
int loopsBeforeSend = 15; //since we read every 2 seconds
boolean metric = false; 

void setup()  
{ 
  gw.begin();

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Garage Door Sensor", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(CHILD_ID, S_DISTANCE);
  //metric = gw.getConfig().isMetric;
}

void loop()      
{     
  int readDist = metric?sonar.ping_cm():sonar.ping_in();
  Serial.print("Ping: ");
  Serial.print(readDist); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.println(metric?" cm":" in");

  Serial.print("Status: ");
  Serial.println(doorStatus);

  if (readDist != lastDist) { //no need to do anything unless something changed.
      
      //logic for door status
      if (readDist < doorClosedDist && readDist != 0) { //door is not closed
        if (readDist <= doorFullyOpenedDist) { //door is fully opened
          doorStatus = 1;
        }
        else { //door is either opening or not fully open
          doorStatus = 2;
        }
      }
      else{ //well, either door is closed or gone! ;)
        doorStatus = 0;
      }

      //send to gw if status changed
      if(lastStatus != doorStatus){
        gw.send(msg.set(doorStatus));
        loopsBeforeSend = 16;
      }
      //set statuses
      lastDist = readDist;
      lastStatus = doorStatus;
  }

  //send the status every 30 seconds no matter what
  if ( loopsBeforeSend == 0 ){
    loopsBeforeSend = 16; //reset the counter
    gw.send(msg.set(doorStatus));
  }
  loopsBeforeSend--;
  Serial.print("Loops left before sending: ");
  Serial.println(loopsBeforeSend);
  gw.sleep(SLEEP_TIME);
}


