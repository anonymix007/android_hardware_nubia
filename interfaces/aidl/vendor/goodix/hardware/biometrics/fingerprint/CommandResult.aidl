/*
 * Copyright (C) 2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.goodix.hardware.biometrics.fingerprint;

@VintfStability
parcelable CommandResult {
    byte[] data;
    int errCode = 0;
}