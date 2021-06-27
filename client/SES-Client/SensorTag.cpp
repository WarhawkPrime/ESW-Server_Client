#include "SensorTag.h"

int SensorTag::initRead(){
	return initializeSensortag();
}

int SensorTag::disconnect(){
	return disconnectSensorTag();
}

int SensorTag::writeMovementConfig(){
	return SensorCommunication::writeMotionConfig(*this);
}

Motion_t SensorTag::getMotion() {
	return SensorCommunication::getMotion(*this);
}


void SensorTag::get_rawData(char* rawData) {
	return SensorCommunication::get_rawData(*this, rawData);
}

void SensorTag::printMotion() {

	Motion_t motion = this->getMotion();

	std::cout << motion.gyro.x << std::endl;
  std::cout << motion.gyro.y << std::endl;
  std::cout << motion.gyro.z << std::endl;
	std::cout << motion.acc.x << std::endl;
  std::cout << motion.acc.y << std::endl;
  std::cout << motion.acc.z << std::endl;
}
