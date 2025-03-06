/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */

#ifndef VSYNC_COMMON_ASSERT_H
#define VSYNC_COMMON_ASSERT_H

/* We include vtypes here to make sure vfreestanding.h inclusion (if available)
 * precedes the inclusion of this header. Through vfreestanding.h external users
 * can define their own ASSERT which should overwrite ours */
#include <vsync/vtypes.h>

#ifndef ASSERT
    #include <assert.h>
    #define ASSERT(V)                                                          \
        do {                                                                   \
            if (!(V))                                                          \
                reach_error();                                                 \
        } while (0)

static inline void
reach_error(void)
{
    assert(0);
}

#endif

#ifndef BUG_ON
    #define BUG_ON(V) ASSERT(!(V))
#endif

#define V_STATIC_ASSERT(_cond_, _str_) _Static_assert((_cond_), _str_)
#endif
