#include <Arduino.h>
#ifndef _POWERMEASUREMGR_H_
#define _POWERMEASUREMGR_H_

// struct to store measurement from powersensor
typedef struct {
  float voltage = nanf("");
  float current = nanf("");
  float power = nanf("");
  float apparentPower = nanf("");
  float reactivePower = nanf("");
  float factor = nanf("");
} powerMeasure;

class RsPowerMeasureMgr
{
    public:
    
    RsPowerMeasureMgr(String pPowerSensor_ID, String pFritzDevice_AIN, int pRepeatCounter = 5, int pSendIntervalMs = 10 * 1000);

    void SetAllPowerValues(powerMeasure pPowerMeasure);
    void SetPower(float power);
    powerMeasure GetPowerValues();;
    bool GetPowerMeasureState();
    String GetFritzDevice_AIN();
    String GetPowerSensor_ID();
    
    bool isSendForced();
    bool isActive();
    bool isAutoRepeatEnabled();
    int GetAutoRepeatCounter();
    void DecrementAutoRepeatCounter(int dontDecrementThreshold = 1000);

    void SetPowerMeasureState(bool state);
    void SetActiveState(bool state);
    void SetSendForced(bool state);
    void SetAutoRepeatState(bool state);
    void SetAutoRepeatCounter(int count);

    void SetLastSendTimeMs(uint32_t pLastSendTimeMs);
    void SetSendIntervalMs(uint32_t pSendIntervalMs);

    uint32_t GetLastSendTimeMs();
    uint32_t GetSendIntervalMs();

    private:
    
    powerMeasure thisPowerMeasure;
    bool powerMeasureState;
    bool activeState;
    bool sendForced;
    bool autoRepeatEnabled;
    int autoRepeatCounter;

    uint32_t lastSendTimeMs;
    uint32_t sendIntervalMs;

    String fritzDevice_AIN;
    String powerSensor_ID;    
};

#endif  // _POWERMEASUREMGR_H_