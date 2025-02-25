/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <compositionengine/UdfpsExtension.h>

#if __has_include(<display/drm/sde_drm.h>)
#include <display/drm/sde_drm.h>
#elif __has_include(<drm/sde_drm.h>)
#include <drm/sde_drm.h>
#endif

uint32_t getUdfpsZOrder(uint32_t z, bool touched) {
    if (touched) {
#ifdef FOD_PRESSED_LAYER_ZORDER
        z |= FOD_PRESSED_LAYER_ZORDER;
#else
        z = 0x41000033;
#endif
    }
    return z;
}

uint64_t getUdfpsUsageBits(uint64_t usageBits, bool /* touched */) {
    return usageBits;
}
