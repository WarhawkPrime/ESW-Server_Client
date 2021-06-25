#ifndef __SENSOR_TAG_H__
#define __SENSOR_TAG_H__

#include "SensorConfiguration.h"
#include "SensorCommunication.h"

class SensorTag: public SensorConfiguration, private SensorCommunication {
    public:
      int initRead();
	    int writeMovementConfig();
	    int disconnect();
      /*
      Reads motion sensor from SensorTag
      @return Return Motion_t with convertet values
      */
      Motion_t getMotion();

      void get_rawData(char *rawData);

      /*
      Writes Config for SensorTag motion sensor
      @return 0 for success, -1 for failure
      */

	     int writeMotionConfig();

       void printMotion();
};

#endif /* __SENSOR_TAG_H__ */
