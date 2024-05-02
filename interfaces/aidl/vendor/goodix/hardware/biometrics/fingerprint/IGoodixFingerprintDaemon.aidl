/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.goodix.hardware.biometrics.fingerprint;

import vendor.goodix.hardware.biometrics.fingerprint.CommandResult;
import vendor.goodix.hardware.biometrics.fingerprint.IGoodixFingerprintDaemonCallback;

@VintfStability
interface IGoodixFingerprintDaemon {
    oneway void setNotify(IGoodixFingerprintDaemonCallback callback);
    CommandResult sendCommand(int cmdId, in byte[] param);
}