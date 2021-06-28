#ifndef __MOTION_H__
#define __MOTION_H__

struct Accelerometer {
	float x;
	float y;
	float z;
};
typedef struct Accelerometer Accelerometer_t;

struct Gyroscope {
	float x;
	float y;
	float z;
};
typedef struct Gyroscope Gyroscope_t;

struct Motion {
	struct Gyroscope gyro;
	struct Accelerometer acc;
};
typedef struct Motion Motion_t;

#endif /* __MOTION__*/
