#ifndef __SENSORCONFIGURATION_H__
#define __SENSORCONFIGURATION_H__

#include <string>

#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>

class SensorConfiguration {

private:
  std::string addr;

public:
  std::string getAddr() {return this->addr;}
  void setAddr(std::string addr) {this->addr = addr;}

  virtual int initializeSensortag();
  virtual int disconnectSensorTag();
};
#endif /* __SENSORCONFIGURATION_H__ */
