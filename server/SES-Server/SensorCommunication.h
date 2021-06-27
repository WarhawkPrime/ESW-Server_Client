#ifndef __SENSOR_COMMUNICATION_H__
#define __SENSOR_COMMUNICATION_H__

#include "Motion.h"
#include "SensorConfiguration.h"

#define MAX_BUF 12


/*
	This Class is used for the communication with a SensorTag
*/
class SensorCommunication {
	public:
		/*
			Reads the motion sensor from the SensorTag specified by the given SensorTag
			confoguration, converts the raw data and returns it as Motion_t.
			@param[in] conf Configuration of the SensorTag
			@return Motion values
		*/
		Motion_t getMotion(SensorConfiguration conf);
		/*
			Writes the configuration for the the SensorTag motion sensor
			@param[in] conf Configuration of the SensorTag
			@return 0 for success, -1 for failure
		*/
		int writeMotionConfig(SensorConfiguration conf);

    unsigned int calcHex(char lowOrder, char highOrder);

		void get_rawData(SensorConfiguration conf, char* rawData);

    float sensorMpu9250GyroConvert(int16_t data)
    {
      //-- calculate rotation, unit deg/s, range -250, +250
      return (data * 1.0) / (65536 / 500);
    }

    float sensorMpu9250AccConvert(int16_t data)
    {
      //-- calculate acceleration, unit G, range -2, +2
      return (data * 1.0) / (32768/2);
    }

	private:
		/*
			Reads the Motion from the SensorTag specified by the given configuration.
			@param[in] conf Configuration of the SensorTag
			@param[out] buffer A 12 byte array that will be filled by the function with
			the raw data of the motion sensor.
			@return 0 for success, -1 for failure
		*/
		virtual int readMotion(SensorConfiguration conf, char buffer[MAX_BUF]);

		/*
			Converts the raw data of the motion sensor and return the converted values.
			@param[in] rawData	A 12 byte array that contains the raw data of the motion sensor.
			@return Motion_t filled with the converted motion values
		*/
		virtual Motion_t convertMotion(char rawData[MAX_BUF]);
};

#endif /* __SENSOR_COMMUNICATION_H__ */
