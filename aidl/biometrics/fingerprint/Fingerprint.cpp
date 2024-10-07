/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <cutils/properties.h>

#include <cstdlib>

#include "Fingerprint.h"

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {

namespace {
constexpr int SENSOR_ID = 0;
constexpr common::SensorStrength SENSOR_STRENGTH = common::SensorStrength::STRONG;
constexpr int MAX_ENROLLMENTS_PER_USER = 7;
constexpr bool SUPPORTS_NAVIGATION_GESTURES = false;
constexpr char HW_COMPONENT_ID[] = "fingerprintSensor";
constexpr char HW_VERSION[] = "vendor/model/revision";
constexpr char FW_VERSION[] = "1.01";
constexpr char SERIAL_NUMBER[] = "00000001";
constexpr char SW_COMPONENT_ID[] = "matchingAlgorithm";
constexpr char SW_VERSION[] = "vendor/version/revision";

}  // namespace

Fingerprint::Fingerprint()
    : mEngine(makeFingerprintEngine()),
      mMaxEnrollmentsPerUser(MAX_ENROLLMENTS_PER_USER),
      mSupportsGestures(SUPPORTS_NAVIGATION_GESTURES),
      mSensorType(FingerprintSensorType::UNKNOWN) {}

Fingerprint::~Fingerprint() {
    ALOGV("~Fingerprint()");
}

ndk::ScopedAStatus Fingerprint::getSensorProps(std::vector<SensorProps>* out) {
    std::vector<common::ComponentInfo> componentInfo = {
            {HW_COMPONENT_ID, HW_VERSION, FW_VERSION, SERIAL_NUMBER, "" /* softwareVersion */},
            {SW_COMPONENT_ID, "" /* hardwareVersion */, "" /* firmwareVersion */,
            "" /* serialNumber */, SW_VERSION}};
    common::CommonProps commonProps = {SENSOR_ID, SENSOR_STRENGTH,
                                       mMaxEnrollmentsPerUser, componentInfo};

    SensorLocation sensorLocation;

    sensorLocation.sensorLocationX = mEngine->getCenterPositionX();
    sensorLocation.sensorLocationY = mEngine->getCenterPositionY();
    sensorLocation.sensorRadius = mEngine->getCenterPositionR();


    ALOGI("Sensor type: %s, location: %s", ::android::internal::ToString(mSensorType).c_str(), sensorLocation.toString().c_str());

    *out = {{commonProps,
             mSensorType,
             {sensorLocation},
             mSupportsGestures,
             false,
             false,
             false,
             std::nullopt}};

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Fingerprint::createSession(int32_t /*sensorId*/, int32_t userId,
                                              const std::shared_ptr<ISessionCallback>& cb,
                                              std::shared_ptr<ISession>* out) {
    CHECK(mSession == nullptr || mSession->isClosed()) << "Open session already exists!";

    mSession = SharedRefBase::make<Session>(mEngine, userId, cb, mLockoutTracker);
    mEngine->setSession(mSession);
    *out = mSession;

    mSession->linkToDeath(cb->asBinder().get());

    return ndk::ScopedAStatus::ok();
}

} // namespace fingerprint
} // namespace biometrics
} // namespace hardware
} // namespace android
} // namespace aidl
