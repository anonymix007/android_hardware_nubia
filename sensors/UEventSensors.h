/*
 * Copyright (C) 2024 Paranoid Android
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <vector>

#include "Sensor.h"
#include "V2_1/SubHal.h"

#define TP_EVENT_PATH "MODALIAS=platform:zte_touch"
#define SINGLE_TAP_GESTURE "single_tap"
#define DOUBLE_TAP_GESTURE "double_tap"
#define AOD_AREAMEET_DOWN "aod_areameet_down"
#define AREAMEET_DOWN "areameet_down"
#define AREAMEET_UP "areameet_up"

namespace android {
namespace hardware {
namespace sensors {
namespace V2_1 {
namespace subhal {
namespace implementation {

struct UEventInfo {
	std::string match;
	std::vector<std::string> keys;
};

class UEventPollingOneShotSensor : public OneShotSensor {
  public:
    UEventPollingOneShotSensor(int32_t sensorHandle, ISensorsEventCallback* callback,
                              const UEventInfo& info, const std::string& name,
                              const std::string& typeAsString, SensorType type);
    virtual ~UEventPollingOneShotSensor() override;

    virtual void activate(bool enable) override;
    virtual void activate(bool enable, bool notify, bool lock);
    virtual void setOperationMode(OperationMode mode) override;
    virtual std::vector<Event> readEvents() override;
    virtual void fillEventData(Event& event);

  protected:
    virtual void run() override;

  private:
    void interruptPoll();
    bool readUEvent();

    struct pollfd mPolls[2];
    int mWaitPipeFd[2];
    int mPollFd;
    UEventInfo mInfo;
};

#ifdef USES_UDFPS_SENSOR
static const UEventInfo udfpsInfo = {TP_EVENT_PATH, {AOD_AREAMEET_DOWN, AREAMEET_DOWN}};

class UdfpsSensor : public UEventPollingOneShotSensor {
  public:
    UdfpsSensor(int32_t sensorHandle, ISensorsEventCallback* callback)
        : UEventPollingOneShotSensor(
              sensorHandle, callback, udfpsInfo,
              "UDFPS Sensor", "co.aospa.sensor.udfps",
              static_cast<SensorType>(static_cast<int32_t>(SensorType::DEVICE_PRIVATE_BASE) + 2)) {}
};
#endif

#ifdef USES_DOUBLE_TAP_SENSOR
static const UEventInfo doubleTapInfo = {TP_EVENT_PATH, {DOUBLE_TAP_GESTURE}};

class DoubleTapSensor : public UEventPollingOneShotSensor {
  public:
    DoubleTapSensor(int32_t sensorHandle, ISensorsEventCallback* callback)
        : UEventPollingOneShotSensor(
              sensorHandle, callback, doubleTapInfo,
              "Double Tap Sensor", "co.aospa.sensor.double_tap",
              static_cast<SensorType>(static_cast<int32_t>(SensorType::DEVICE_PRIVATE_BASE) + 1)) {}
};
#endif

#ifdef USES_SINGLE_TAP_SENSOR
static const UEventInfo singleTapInfo = {TP_EVENT_PATH, {SINGLE_TAP_GESTURE}};

class SingleTapSensor : public UEventPollingOneShotSensor {
  public:
    SingleTapSensor(int32_t sensorHandle, ISensorsEventCallback* callback)
        : UEventPollingOneShotSensor(
              sensorHandle, callback, singleTapInfo,
              "Single Tap Sensor", "co.aospa.sensor.single_tap",
              static_cast<SensorType>(static_cast<int32_t>(SensorType::DEVICE_PRIVATE_BASE) + 1)) {}
};
#endif

}  // namespace implementation
}  // namespace subhal
}  // namespace V2_1
}  // namespace sensors
}  // namespace hardware
}  // namespace android
