/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.goodix.hardware.biometrics.fingerprint;

import vendor.goodix.hardware.biometrics.fingerprint.CommandResult;
import vendor.goodix.hardware.biometrics.fingerprint.IGoodixFingerprintDaemonCallback;

@VintfStability
interface IGoodixFingerprintDaemonCallback {
    oneway void onDaemonMessage(long devId, int msgId, int cmdId, in byte[] data);
}