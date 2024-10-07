/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <memory>

#include <aidl/android/hardware/biometrics/fingerprint/FingerprintSensorType.h>
#include <aidl/android/hardware/biometrics/fingerprint/ISessionCallback.h>

#include <LockoutTracker.h>

using ::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType;
using namespace ::aidl::android::hardware::biometrics::common;

#define WEAK_SESSION_CALL_METHOD_OR_LOG_ERROR(session, method, ...) \
    do {                                                                                 \
        if (auto sps = (session).lock()) {                                               \
            sps->(method)(__VA_ARGS__);                                                  \
        } else {                                                                         \
            ALOGE("%s: %s does not exist, cannot call %s", __func__, #session, #method); \
        }                                                                                \
    } while(0)

#define WEAK_SESSION_CALLBACK_OR_LOG_ERROR(session, method, ...) \
    do {                                                                                            \
        if (auto sps = (session).lock()) {                                                          \
            sps->mCb->method(__VA_ARGS__);                                                          \
        } else {                                                                                    \
            ALOGE("%s: %s does not exist, cannot invoke callback %s", __func__, #session, #method); \
        }                                                                                           \
    } while(0)


namespace aidl::android::hardware::biometrics::fingerprint {

class Session;

// Device-specific fingerprint engine

class FingerprintEngine {
  public:
    virtual ~FingerprintEngine();

    virtual void setSession(std::shared_ptr<Session> session) = 0;
    virtual void setActiveGroup(int userId) = 0;

    virtual FingerprintSensorType getSensorType() const = 0;
    virtual int32_t getCenterPositionR() const = 0;
    virtual int32_t getCenterPositionX() const = 0;
    virtual int32_t getCenterPositionY() const = 0;

    virtual void generateChallengeImpl() = 0;
    virtual void revokeChallengeImpl(int64_t challenge) = 0;
    virtual void enrollImpl(const keymaster::HardwareAuthToken& hat) = 0;
    virtual void authenticateImpl(int64_t operationId) = 0;
    virtual void detectInteractionImpl() = 0;
    virtual void enumerateEnrollmentsImpl() = 0;
    virtual void removeEnrollmentsImpl(const std::vector<int32_t>& enrollmentIds) = 0;
    virtual void getAuthenticatorIdImpl() = 0;
    virtual void invalidateAuthenticatorIdImpl() = 0;
    virtual void onPointerDownImpl(int32_t pointerId, int32_t x, int32_t y, float minor, float major) = 0;
    virtual void onPointerUpImpl(int32_t pointerId) = 0;
    virtual void onUiReadyImpl() = 0;
    virtual ndk::ScopedAStatus cancelImpl() = 0;

protected:
    std::weak_ptr<Session> mSession;
};

extern std::shared_ptr<FingerprintEngine> makeFingerprintEngine();
}
