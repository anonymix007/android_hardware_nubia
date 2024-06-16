/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/biometrics/common/SensorStrength.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>
#include <aidl/android/hardware/biometrics/fingerprint/SensorLocation.h>

#include <hardware/fingerprint.h>
#include <hardware/hardware.h>

#include "LockoutTracker.h"

using namespace ::aidl::android::hardware::biometrics::common;

struct fingerprint_device_gf95xx;

namespace aidl::android::hardware::biometrics::fingerprint {

// Device-specific fingerprint engine

class FingerprintEngine {
  public:
    FingerprintEngine(fingerprint_device_t* device);
    ~FingerprintEngine();

    void setActiveGroup(int userId);

    void generateChallengeImpl(ISessionCallback* cb);
    void revokeChallengeImpl(ISessionCallback* cb, int64_t challenge);
    void enrollImpl(ISessionCallback* cb, const keymaster::HardwareAuthToken& hat);
    void authenticateImpl(ISessionCallback* cb, int64_t operationId);
    void detectInteractionImpl(ISessionCallback* cb);
    void enumerateEnrollmentsImpl(ISessionCallback* cb);
    void removeEnrollmentsImpl(ISessionCallback* cb, const std::vector<int32_t>& enrollmentIds);
    void getAuthenticatorIdImpl(ISessionCallback* cb);
    void invalidateAuthenticatorIdImpl(ISessionCallback* cb);
    void onPointerDownImpl(int32_t pointerId, int32_t x, int32_t y, float minor, float major);
    void onPointerUpImpl(int32_t pointerId);
    void onUiReadyImpl();

    ndk::ScopedAStatus cancelImpl(ISessionCallback* cb);

    bool notifyImpl(ISessionCallback* cb, const fingerprint_msg_t* msg, LockoutTracker& lockoutTracker);

private:
    fingerprint_device_gf95xx* mDevice;
    int32_t mUserId;

    uint64_t mAuthId;
    uint64_t mChallengeId;

    Error VendorErrorFilter(int32_t error, int32_t* vendorCode);
    AcquiredInfo VendorAcquiredFilter(int32_t info, int32_t* vendorCode);
};

}