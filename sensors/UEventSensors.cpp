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


#include "UEventSensors.h"
#include "UEvent.h"

#include <hardware/sensors.h>
#include <log/log.h>
#include <utils/SystemClock.h>
#include <cutils/uevent.h>

#include <unordered_map>
#include <string>
#define UEVENT_BUFFER_SIZE 8192

namespace android {
namespace hardware {
namespace sensors {
namespace V2_1 {
namespace subhal {
namespace implementation {

using ::android::hardware::sensors::V1_0::MetaDataEventType;
using ::android::hardware::sensors::V1_0::OperationMode;
using ::android::hardware::sensors::V1_0::Result;
using ::android::hardware::sensors::V1_0::SensorFlagBits;
using ::android::hardware::sensors::V1_0::SensorStatus;
using ::android::hardware::sensors::V2_1::Event;
using ::android::hardware::sensors::V2_1::SensorInfo;
using ::android::hardware::sensors::V2_1::SensorType;

UEventPollingOneShotSensor::UEventPollingOneShotSensor(
    int32_t sensorHandle, ISensorsEventCallback* callback, const UEventInfo& info,
    const std::string& name, const std::string& typeAsString,
    SensorType type)
    : OneShotSensor(sensorHandle, callback), mInfo(info) {
    mSensorInfo.name = name;
    mSensorInfo.type = type;
    mSensorInfo.typeAsString = typeAsString;
    mSensorInfo.maxRange = 2048.0f;
    mSensorInfo.resolution = 1.0f;
    mSensorInfo.power = 0;
    mSensorInfo.flags |= SensorFlagBits::WAKE_UP;

    int rc;

    rc = pipe(mWaitPipeFd);
    if (rc < 0) {
        mWaitPipeFd[0] = -1;
        mWaitPipeFd[1] = -1;
        ALOGE("failed to open wait pipe: %d", rc);
    }

    mPollFd = uevent_open_socket(256 * 1024, false);
    //fcntl(mPollFd, F_SETFL, O_NONBLOCK);

    if (mPollFd < 0) {
        ALOGE("failed to open poll fd: %d", mPollFd);
    }

    if (mWaitPipeFd[0] < 0 || mWaitPipeFd[1] < 0 || mPollFd < 0) {
        mStopThread = true;
        return;
    }

    mPolls[0] = {
        .fd = mWaitPipeFd[0],
        .events = POLLIN,
    };

    mPolls[1] = {
        .fd = mPollFd,
        .events = POLLERR | POLLPRI,
    };
}

UEventPollingOneShotSensor::~UEventPollingOneShotSensor() {
    interruptPoll();
}

void UEventPollingOneShotSensor::activate(bool enable, bool notify, bool lock) {
    std::unique_lock<std::mutex> runLock(mRunMutex, std::defer_lock);

    if (lock) {
        runLock.lock();
    }

    if (mIsEnabled != enable) {

        mIsEnabled = enable;

        if (notify) {
            interruptPoll();
            mWaitCV.notify_all();
        }
    }

    if (lock) {
        runLock.unlock();
    }
}

void UEventPollingOneShotSensor::activate(bool enable) {
    activate(enable, true, true);
}

void UEventPollingOneShotSensor::setOperationMode(OperationMode mode) {
    Sensor::setOperationMode(mode);
    interruptPoll();
}

void UEventPollingOneShotSensor::run() {
    std::unique_lock<std::mutex> runLock(mRunMutex);

    while (!mStopThread) {
        if (!mIsEnabled || mMode == OperationMode::DATA_INJECTION) {
            mWaitCV.wait(runLock, [&] {
                return ((mIsEnabled && mMode == OperationMode::NORMAL) || mStopThread);
            });
        } else {
            // Cannot hold lock while polling.
            runLock.unlock();
            int rc = poll(mPolls, 2, -1);
            runLock.lock();

            if (rc < 0) {
                ALOGE("failed to poll: %d", rc);
                mStopThread = true;
                continue;
            }

            if (mPolls[1].revents == mPolls[1].events && readUEvent()) {
                activate(false, false, false);
                mCallback->postEvents(readEvents(), isWakeUpSensor());
            } else if (mPolls[0].revents == mPolls[0].events) {
                readBool(mWaitPipeFd[0], false /* seek */);
            }
        }
    }
}
bool UEventPollingOneShotSensor::readUEvent() {
	char buf[UEVENT_BUFFER_SIZE + 2];
	int n = uevent_kernel_multicast_recv(mPollFd, buf, UEVENT_BUFFER_SIZE);
    if (n <= 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            ALOGE("Error reading from uevent fd");
        }
        return false;
    }
    if (n >= UEVENT_BUFFER_SIZE) {
        ALOGE("Uevent overflowed buffer, discarding");
        // Return true here even if we discard as we may have more uevents pending and we
        // want to keep processing them.
        return true;
    }

    buf[n] = '\0';
    buf[n + 1] = '\0';

    UEvent event(buf);

    if (!event.contains(mInfo.match)) {
		return false;
	}

    for (auto& key: mInfo.keys) {
		if (event.get(key, "false") == "true") {
			return true;
		}
	}
	return false;
}

void UEventPollingOneShotSensor::interruptPoll() {
    if (mWaitPipeFd[1] < 0) return;

    char c = '1';
    write(mWaitPipeFd[1], &c, sizeof(c));
}

std::vector<Event> UEventPollingOneShotSensor::readEvents() {
    std::vector<Event> events;
    Event event;
    event.sensorHandle = mSensorInfo.sensorHandle;
    event.sensorType = mSensorInfo.type;
    event.timestamp = ::android::elapsedRealtimeNano();
    fillEventData(event);
    events.push_back(event);
    return events;
}

void UEventPollingOneShotSensor::fillEventData(Event& event) {
    event.u.data[0] = 0;
    event.u.data[1] = 0;
}

}  // namespace implementation
}  // namespace subhal
}  // namespace V2_1
}  // namespace sensors
}  // namespace hardware
}  // namespace android
