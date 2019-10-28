#ifndef __MOTOR_SPEED_HAL__
#define __MOTOR_SPEED_HAL__


extern uint16_t MaxVehicleCalibrate;//单位km/h
extern uint16_t HalCalibrate100KmH;
/*
extern uint16_t WheelDiameter;//单位cm
extern uint16_t MotorPoles;//电机对极数
extern uint32_t ReductionRatio;//齿比*100extern uint16_t HalMaxSpeedCalibrate;
*/



void MotorSpeedHalHwInit(void);
void MotorSpeedHalIrqHandler(void);
void HalSpeedTimerIntHandler(void);
int16_t GetDistSpeedHal(void);
void InitThrottleCurve(void);
void CalHalMaxSpeedCalibrate(void);


#endif


