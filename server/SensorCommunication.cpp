#include "SensorCommunication.h"
#include "SensorConfiguration.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
// system
#include <stdlib.h>
// fifo
#include <fcntl.h>
#include <sys/stat.h>
// read
#include <unistd.h>


Motion_t SensorCommunication::getMotion(SensorConfiguration conf) {
	char buffer[MAX_BUF];
	memset(buffer, 0, MAX_BUF);
	if (readMotion(conf, buffer) != 0) {
		std::cerr << "can't read motion" << std::endl;
	}
return convertMotion(buffer);
}


 void SensorCommunication::get_rawData(SensorConfiguration conf, char* rawData) {
	char buffer[MAX_BUF];
	memset(buffer, 0, MAX_BUF);
	if (readMotion(conf, buffer) != 0) {
		std::cerr << "can't read motion" << std::endl;
	}
	for(int i = 0; i < MAX_BUF; ++i){

		rawData[i] = buffer[i];
	}
	//return buffer;
}

Motion_t SensorCommunication::convertMotion(char* rawData) {
	Motion_t motion;

  float gyroX;
  float gyroY;
  float gyroZ;

  gyroX = sensorMpu9250GyroConvert(calcHex(rawData[0],rawData[1]));
  gyroY = sensorMpu9250GyroConvert(calcHex(rawData[2],rawData[3]));
  gyroZ = sensorMpu9250GyroConvert(calcHex(rawData[4],rawData[5]));

  //std::cout << gyroX << std::endl;
  //std::cout << gyroY << std::endl;
  //std::cout << gyroZ << std::endl;

  float accX;
  float accY;
  float accZ;

  accX = sensorMpu9250AccConvert(calcHex(rawData[6],rawData[7]));
  accY = sensorMpu9250AccConvert(calcHex(rawData[8],rawData[9]));
  accZ = sensorMpu9250AccConvert(calcHex(rawData[10],rawData[11]));

  //std::cout << accX << std::endl;
  //std::cout << accY << std::endl;
  //std::cout << accZ << std::endl;

  motion.gyro.x = gyroX;
  motion.gyro.y = gyroY;
  motion.gyro.z = gyroZ;
  motion.acc.x = accX;
  motion.acc.y = accY;
  motion.acc.z = accZ;

	/*
	std::cout << motion.gyro.x << std::endl;
  std::cout << motion.gyro.y << std::endl;
  std::cout << motion.gyro.z << std::endl;
	std::cout << motion.acc.x << std::endl;
  std::cout << motion.acc.y << std::endl;
  std::cout << motion.acc.z << std::endl;
	*/
	return motion;

  //calcHex(rawData[2],rawData[3]);

/* Umrechnung hier einfügen */
/*
GyroX[0:7],
GyroX[8:15],
GyroY[0:7],
GyroY[8:15],
GyroZ[0:7],
GyroZ[8:15],
AccX[0:7],
AccX[8:15],
AccY[0:7],
AccY[8:15],
AccZ[0:7],
AccZ[8:15]

Bit-Breite: 16
Gyro range: -250 bis +250
Acc range: -2 bis +2

Testdaten: 0d 01 6c fc d9 fc f4 fd 85 00 d2 0f
Erwartetes Ergebnis:
	gyro: 2.05344, -6.99237, -6.16031
	acc: -0.0319824, 0.00811768, 0.247192
*/
}

int SensorCommunication::writeMotionConfig(SensorConfiguration conf){
	//std::cout << "Write Config Platzhalter" <<std::endl;
    /* Activate the motion measurements */
return 0;
}

int SensorCommunication::readMotion(SensorConfiguration conf,char buffer[MAX_BUF]) {
	buffer[0] = 0x0d;
	buffer[1] = 0x01;	// 0x010d
	buffer[2] = 0x6c;
	buffer[3] = 0xfc;
	buffer[4] = 0xd9;
	buffer[5] = 0xfc;
	buffer[6] = 0xf4;
	buffer[7] = 0xfd;
	buffer[8] = 0x85;
	buffer[9] = 0x00;
	buffer[10] = 0xd2;
	buffer[11] = 0x0f;

/*
  for(short i = 0; i < 12; i++){
    int byte = buffer[i];
    std::cout << byte << std::endl;

  }
*/
            /* Read movement data and display it */

        //std::this_thread::sleep_for(std::chrono::seconds(1));
	return 0;
}


unsigned int SensorCommunication::calcHex(char lowOrder, char highOrder)
{

	//point int16_t to buffer


  //cast unsigned char integer promotion
  unsigned char highOrderC = highOrder;
  unsigned char lowOrderC = lowOrder;

  //cast to unsigned int -> value is now in dec
  unsigned int highOrderI = (int) highOrderC;
  unsigned int lowOrderI = (int) lowOrderC;

  //cast to string
  std::string highOrderS = std::to_string(highOrderI);
  std::string lowOrderS = std::to_string(lowOrderI);

  //to stringstream, changing value to hex again
  std::stringstream highOrderSS;
  std::stringstream lowOrderSS;

  highOrderSS << std::hex << std::stoi(highOrderS) << 0 << 0;
  lowOrderSS << std::hex << std::stoi(lowOrderS) << std::endl;

  //finally calc the correct hex values (base 16)
  unsigned int highOrderR = std::stoi(highOrderSS.str(),0,16);
  unsigned int lowOrderR = std::stoi(lowOrderSS.str(),0,16);

  unsigned int result = highOrderR + lowOrderR;

  return result;
}
