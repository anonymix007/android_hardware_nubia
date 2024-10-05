/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thread>

#include "Session.h"

#include "CancellationSignal.h"

namespace aidl {
namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
void onClientDeath(void* cookie) {
    ALOGI("FingerprintService has died");
    Session* session = static_cast<Session*>(cookie);
    if (session && !session->isClosed()) {
        session->close();
    }
}

Session::Session(FingerprintEngine* engine, int userId,
            std::shared_ptr<ISessionCallback> cb, LockoutTracker lockoutTracker)
            : mEngine(engine), mUserId(userId), mCb(cb),
              mLockoutTracker(lockoutTracker) {
    mDeathRecipient = AIBinder_DeathRecipient_new(onClientDeath);
    mEngine->setActiveGroup(mUserId);
}

ndk::ScopedAStatus Session::generateChallenge() {
    mEngine->generateChallengeImpl(mCb.get());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::revokeChallenge(int64_t challenge) {
    mEngine->revokeChallengeImpl(mCb.get(), challenge);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::enroll(const HardwareAuthToken& hat,
                                   std::shared_ptr<ICancellationSignal>* out) {
    mEngine->enrollImpl(mCb.get(), hat);
    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::authenticate(int64_t operationId,
                                         std::shared_ptr<ICancellationSignal>* out) {
    mEngine->authenticateImpl(mCb.get(), operationId);
    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::detectInteraction(std::shared_ptr<ICancellationSignal>* out) {
    mEngine->detectInteractionImpl(mCb.get());
    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::enumerateEnrollments() {
    mEngine->enumerateEnrollmentsImpl(mCb.get());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::removeEnrollments(const std::vector<int32_t>& enrollmentIds) {
    mEngine->removeEnrollmentsImpl(mCb.get(), enrollmentIds);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::getAuthenticatorId() {
    mEngine->getAuthenticatorIdImpl(mCb.get());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::invalidateAuthenticatorId() {
    mEngine->invalidateAuthenticatorIdImpl(mCb.get());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::resetLockout(const HardwareAuthToken& /*hat*/) {
    ALOGI("resetLockout");

    clearLockout(true);
    mIsLockoutTimerAborted = true;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::onPointerDown(int32_t pointerId, int32_t x, int32_t y, float minor, float major) {
    mEngine->onPointerDownImpl(mCb.get(), pointerId, x, y, minor, major);
    checkSensorLockout();

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::onPointerUp(int32_t pointerId) {
    mEngine->onPointerUpImpl(mCb.get(), pointerId);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::onUiReady() {
    mEngine->onUiReadyImpl(mCb.get());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::authenticateWithContext(
        int64_t operationId, const common::OperationContext& /*context*/,
        std::shared_ptr<common::ICancellationSignal>* out) {
    return authenticate(operationId, out);
}

ndk::ScopedAStatus Session::enrollWithContext(const keymaster::HardwareAuthToken& hat,
                                              const common::OperationContext& /*context*/,
                                              std::shared_ptr<common::ICancellationSignal>* out) {
    return enroll(hat, out);
}

ndk::ScopedAStatus Session::detectInteractionWithContext(
        const common::OperationContext& /*context*/,
        std::shared_ptr<common::ICancellationSignal>* out) {
    return detectInteraction(out);
}

ndk::ScopedAStatus Session::onPointerDownWithContext(const PointerContext& context) {
    return onPointerDown(context.pointerId, context.x, context.y, context.minor, context.major);
}

ndk::ScopedAStatus Session::onPointerUpWithContext(const PointerContext& context) {
    return onPointerUp(context.pointerId);
}

ndk::ScopedAStatus Session::onContextChanged(const common::OperationContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::onPointerCancelWithContext(const PointerContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::setIgnoreDisplayTouches(bool /*shouldIgnore*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Session::cancel() {
    return mEngine->cancelImpl(mCb.get());
}

ndk::ScopedAStatus Session::close() {
    ALOGI("close");
    mClosed = true;
    mCb->onSessionClosed();
    AIBinder_DeathRecipient_delete(mDeathRecipient);
    return ndk::ScopedAStatus::ok();
}

binder_status_t Session::linkToDeath(AIBinder* binder) {
    return AIBinder_linkToDeath(binder, mDeathRecipient, this);
}

bool Session::isClosed() {
    return mClosed;
}

bool Session::checkSensorLockout() {
    LockoutMode lockoutMode = mLockoutTracker.getMode();
    if (lockoutMode == LockoutMode::PERMANENT) {
        ALOGE("Fail: lockout permanent");
        mCb->onLockoutPermanent();
        mIsLockoutTimerAborted = true;
        return true;
    } else if (lockoutMode == LockoutMode::TIMED) {
        int64_t timeLeft = mLockoutTracker.getLockoutTimeLeft();
        ALOGE("Fail: lockout timed: %ld", timeLeft);
        mCb->onLockoutTimed(timeLeft);
        if (!mIsLockoutTimerStarted) startLockoutTimer(timeLeft);
        return true;
    }
    return false;
}

void Session::clearLockout(bool clearAttemptCounter) {
    mLockoutTracker.reset(clearAttemptCounter);
    mCb->onLockoutCleared();
}

void Session::startLockoutTimer(int64_t timeout) {
    mIsLockoutTimerAborted = false;
    std::function<void()> action =
            std::bind(&Session::lockoutTimerExpired, this);
    std::thread([timeout, action]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        action();
    }).detach();

    mIsLockoutTimerStarted = true;
}

void Session::lockoutTimerExpired() {
    if (!mIsLockoutTimerAborted)
        clearLockout(false);

    mIsLockoutTimerStarted = false;
    mIsLockoutTimerAborted = false;
}

void Session::notify(const fingerprint_msg_t* msg) {
    if (mEngine->notifyImpl(mCb.get(), msg, mLockoutTracker)) {
        checkSensorLockout();
    }
}

} // namespace fingerprint
} // namespace biometrics
} // namespace hardware
} // namespace android
} // namespace aidl
