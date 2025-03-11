/*
 * Copyright (C) 2022-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <compositionengine/UdfpsExtension.h>

#define ZTE_UDFPS_FOD_MASK 0x20000000
#define ZTE_UDFPS_HBM_MASK 0x40000000
#define ZTE_UDFPS_AOD_MASK 0x80000000

uint32_t getUdfpsDimZOrder(uint32_t z) {
    return z | ZTE_UDFPS_HBM_MASK;
}

uint32_t getUdfpsZOrder(uint32_t z, bool touched) {
    return touched ? (z | ZTE_UDFPS_FOD_MASK) : z;
}

uint64_t getUdfpsUsageBits(uint64_t usageBits, bool /* touched */) {
    return usageBits;
}
