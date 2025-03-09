/*
 * Copyright (C) 2022-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <compositionengine/UdfpsExtension.h>

uint32_t getUdfpsDimZOrder(uint32_t z) {
    return 0x41000005;
}

uint32_t getUdfpsZOrder(uint32_t z, bool touched) {
    return touched ? 0x41000033 : z;
}

uint64_t getUdfpsUsageBits(uint64_t usageBits, bool /* touched */) {
    return usageBits;
}
