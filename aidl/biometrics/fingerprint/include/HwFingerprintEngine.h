/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/fingerprint.h>
#include <hardware/hardware.h>

#include <FingerprintEngine.h>

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {

struct HwFingerprintModule {
    const char *id_name;
    const char *class_name;
    FingerprintSensorType sensor_type;
};

class HwFingerprintEngine : public FingerprintEngine {
  public:
    HwFingerprintEngine(const std::vector<HwFingerprintModule> &modules, bool setNotifyCallback = true);
    virtual ~HwFingerprintEngine();

    virtual FingerprintSensorType getSensorType() const;
    virtual int32_t getCenterPositionR() const = 0;
    virtual int32_t getCenterPositionX() const = 0;
    virtual int32_t getCenterPositionY() const = 0;

    virtual void setSession(std::shared_ptr<Session> session);
    virtual void setActiveGroup(int userId);

    virtual void generateChallengeImpl();
    virtual void revokeChallengeImpl(int64_t challenge);
    virtual void enrollImpl(const keymaster::HardwareAuthToken& hat);
    virtual void authenticateImpl(int64_t operationId);
    virtual void detectInteractionImpl();
    virtual void enumerateEnrollmentsImpl();
    virtual void removeEnrollmentsImpl(const std::vector<int32_t>& enrollmentIds);
    virtual void getAuthenticatorIdImpl();
    virtual void invalidateAuthenticatorIdImpl();
    virtual void onPointerDownImpl(int32_t pointerId, int32_t x, int32_t y, float minor, float major) = 0;
    virtual void onPointerUpImpl(int32_t pointerId) = 0;
    virtual void onUiReadyImpl() = 0;
    virtual void onAcquired() = 0;
    virtual ndk::ScopedAStatus cancelImpl();

protected:
    static fingerprint_device_t* openHwModule(const char* id_name, const char* class_name);
    fingerprint_device_t *getDevice() { return mDevice; }

    int32_t mUserId;
    uint64_t mAuthId;
    uint64_t mChallengeId;

private:
    static Error VendorErrorFilter(int32_t error, int32_t* vendorCode);
    static AcquiredInfo VendorAcquiredFilter(int32_t info, int32_t* vendorCode);
    static void notify(const fingerprint_msg_t* msg);

    FingerprintSensorType mSensorType;
    fingerprint_device_t *mDevice;
};

} // namespace fingerprint
} // namespace biometrics
} // namespace hardware
} // namespace android
} // namespace aidl
