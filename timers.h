/*******************************************************************************
*   File Name: 
*  
* Description: 
*******************************************************************************/
#ifndef __TIMERS_H__
#define __TIMERS_H__
  
// global data
extern volatile uint16_t ms_count;
extern volatile uint16_t ms_MotorCount;
extern volatile uint16_t ms_SleepCount;  
extern volatile uint16_t ms_ADCcount;
extern volatile uint16_t ms_coldCount; 
extern volatile uint16_t ms_hotCount; 
extern volatile uint16_t ms_sampleCount; 
extern volatile uint16_t ms_switchCount;
extern volatile uint16_t ms_switchReleasedCount;
extern volatile uint16_t ms_twiCount;
extern volatile uint16_t ms_PressureCount;


// global functions
void ms_sleep(uint16_t ms);
void init_timers(void);


#endif  // end __TIMERS_H__
