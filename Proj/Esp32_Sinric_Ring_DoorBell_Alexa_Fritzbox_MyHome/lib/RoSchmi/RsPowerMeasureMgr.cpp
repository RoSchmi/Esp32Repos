#include "RsPowerMeasureMgr.h"

RsPowerMeasureMgr::RsPowerMeasureMgr(String pPowerSensor_ID, String pFritzDevice_AIN, int pRepeatCounter, int pSendIntervalMs)
 {
    fritzDevice_AIN = pFritzDevice_AIN;
    powerSensor_ID = pPowerSensor_ID;
    activeState = true;
    sendForced = true;
    powerMeasureState = false;
    autoRepeatEnabled = true;
    autoRepeatCounter = pRepeatCounter;
    lastSendTimeMs = millis();
    sendIntervalMs = pSendIntervalMs;
 }

 void RsPowerMeasureMgr::SetAllPowerValues(powerMeasure pPowerMeasure)
 {
     thisPowerMeasure = pPowerMeasure;
     powerMeasureState = false;
 }
 void RsPowerMeasureMgr::SetPower(float power)
 {
     thisPowerMeasure.power = power;
 }

 powerMeasure RsPowerMeasureMgr::GetPowerValues() {
     return thisPowerMeasure; }
 
bool RsPowerMeasureMgr::GetPowerMeasureState() {
     return powerMeasureState; }

bool RsPowerMeasureMgr::isAutoRepeatEnabled() {
     return autoRepeatEnabled; }

void RsPowerMeasureMgr::DecrementAutoRepeatCounter(int dontDecrementThreshold)
{   if (!(autoRepeatCounter >= dontDecrementThreshold))  
    {
        autoRepeatCounter = autoRepeatCounter > 0 ? (autoRepeatCounter - 1) : 0;
    }
}

int RsPowerMeasureMgr::GetAutoRepeatCounter() {
    return autoRepeatCounter;
}

bool RsPowerMeasureMgr::isActive() {
     return activeState; }

bool RsPowerMeasureMgr::isSendForced() {
    return sendForced; }

void RsPowerMeasureMgr::SetPowerMeasureState(bool state) {
    powerMeasureState = state; }

void RsPowerMeasureMgr::SetActiveState(bool state) {
    activeState = state; }

void RsPowerMeasureMgr::SetSendForced(bool state) {
    sendForced = state; }

void RsPowerMeasureMgr::SetAutoRepeatState(bool state) {
    autoRepeatEnabled = state; }

void RsPowerMeasureMgr::SetAutoRepeatCounter(int count) {
    autoRepeatCounter = count;
}

void RsPowerMeasureMgr::SetLastSendTimeMs(uint32_t pLastSendTimeMs) {
    lastSendTimeMs = pLastSendTimeMs; }

void RsPowerMeasureMgr::SetSendIntervalMs(uint32_t pSendIntervalMs) {
    sendIntervalMs = pSendIntervalMs; }

String RsPowerMeasureMgr::GetFritzDevice_AIN()
{
    return fritzDevice_AIN;
}

String RsPowerMeasureMgr::GetPowerSensor_ID()
{
    return powerSensor_ID;
}
String GetPowerSensor_ID;

uint32_t RsPowerMeasureMgr::GetLastSendTimeMs() {
    return lastSendTimeMs; }

uint32_t RsPowerMeasureMgr::GetSendIntervalMs() {
    return sendIntervalMs; }



 