/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * SPDX-License-Identifier: MIT
 */

#ifndef VATOMIC_CONFIG_U16_RLX_H
#define VATOMIC_CONFIG_U16_RLX_H
/* !!!Warning: File generated by tmpl; DO NOT EDIT.!!! */

#include <vsync/atomic/await.h>

#if defined(VATOMIC_ENABLE_ATOMIC_RLX)

    #define VATOMIC16_READ
static inline vuint16_t
vatomic16_read(const vatomic16_t *a)
{
    return vatomic16_read_rlx(a);
}
    #define VATOMIC16_READ_ACQ
static inline vuint16_t
vatomic16_read_acq(const vatomic16_t *a)
{
    return vatomic16_read_rlx(a);
}

    #define VATOMIC16_WRITE
static inline void
vatomic16_write(vatomic16_t *a, vuint16_t v)
{
    vatomic16_write_rlx(a, v);
}
    #define VATOMIC16_WRITE_REL
static inline void
vatomic16_write_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_write_rlx(a, v);
}

    #define VATOMIC16_XCHG
static inline vuint16_t
vatomic16_xchg(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xchg_rlx(a, v);
}
    #define VATOMIC16_XCHG_ACQ
static inline vuint16_t
vatomic16_xchg_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xchg_rlx(a, v);
}
    #define VATOMIC16_XCHG_REL
static inline vuint16_t
vatomic16_xchg_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xchg_rlx(a, v);
}

    #define VATOMIC16_CMPXCHG
static inline vuint16_t
vatomic16_cmpxchg(vatomic16_t *a, vuint16_t e, vuint16_t v)
{
    return vatomic16_cmpxchg_rlx(a, e, v);
}
    #define VATOMIC16_CMPXCHG_ACQ
static inline vuint16_t
vatomic16_cmpxchg_acq(vatomic16_t *a, vuint16_t e, vuint16_t v)
{
    return vatomic16_cmpxchg_rlx(a, e, v);
}
    #define VATOMIC16_CMPXCHG_REL
static inline vuint16_t
vatomic16_cmpxchg_rel(vatomic16_t *a, vuint16_t e, vuint16_t v)
{
    return vatomic16_cmpxchg_rlx(a, e, v);
}

    #define VATOMIC16_GET_MAX
static inline vuint16_t
vatomic16_get_max(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_max_rlx(a, v);
}
    #define VATOMIC16_GET_AND
static inline vuint16_t
vatomic16_get_and(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_and_rlx(a, v);
}
    #define VATOMIC16_GET_OR
static inline vuint16_t
vatomic16_get_or(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_or_rlx(a, v);
}
    #define VATOMIC16_GET_XOR
static inline vuint16_t
vatomic16_get_xor(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_xor_rlx(a, v);
}
    #define VATOMIC16_GET_ADD
static inline vuint16_t
vatomic16_get_add(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_add_rlx(a, v);
}
    #define VATOMIC16_GET_SUB
static inline vuint16_t
vatomic16_get_sub(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_sub_rlx(a, v);
}
    #define VATOMIC16_MAX_GET
static inline vuint16_t
vatomic16_max_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_max_get_rlx(a, v);
}
    #define VATOMIC16_AND_GET
static inline vuint16_t
vatomic16_and_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_and_get_rlx(a, v);
}
    #define VATOMIC16_OR_GET
static inline vuint16_t
vatomic16_or_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_or_get_rlx(a, v);
}
    #define VATOMIC16_XOR_GET
static inline vuint16_t
vatomic16_xor_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xor_get_rlx(a, v);
}
    #define VATOMIC16_ADD_GET
static inline vuint16_t
vatomic16_add_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_add_get_rlx(a, v);
}
    #define VATOMIC16_SUB_GET
static inline vuint16_t
vatomic16_sub_get(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_sub_get_rlx(a, v);
}
    #define VATOMIC16_GET_MAX_ACQ
static inline vuint16_t
vatomic16_get_max_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_max_rlx(a, v);
}
    #define VATOMIC16_GET_AND_ACQ
static inline vuint16_t
vatomic16_get_and_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_and_rlx(a, v);
}
    #define VATOMIC16_GET_OR_ACQ
static inline vuint16_t
vatomic16_get_or_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_or_rlx(a, v);
}
    #define VATOMIC16_GET_XOR_ACQ
static inline vuint16_t
vatomic16_get_xor_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_xor_rlx(a, v);
}
    #define VATOMIC16_GET_ADD_ACQ
static inline vuint16_t
vatomic16_get_add_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_add_rlx(a, v);
}
    #define VATOMIC16_GET_SUB_ACQ
static inline vuint16_t
vatomic16_get_sub_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_sub_rlx(a, v);
}
    #define VATOMIC16_MAX_GET_ACQ
static inline vuint16_t
vatomic16_max_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_max_get_rlx(a, v);
}
    #define VATOMIC16_AND_GET_ACQ
static inline vuint16_t
vatomic16_and_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_and_get_rlx(a, v);
}
    #define VATOMIC16_OR_GET_ACQ
static inline vuint16_t
vatomic16_or_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_or_get_rlx(a, v);
}
    #define VATOMIC16_XOR_GET_ACQ
static inline vuint16_t
vatomic16_xor_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xor_get_rlx(a, v);
}
    #define VATOMIC16_ADD_GET_ACQ
static inline vuint16_t
vatomic16_add_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_add_get_rlx(a, v);
}
    #define VATOMIC16_SUB_GET_ACQ
static inline vuint16_t
vatomic16_sub_get_acq(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_sub_get_rlx(a, v);
}
    #define VATOMIC16_GET_MAX_REL
static inline vuint16_t
vatomic16_get_max_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_max_rlx(a, v);
}
    #define VATOMIC16_GET_AND_REL
static inline vuint16_t
vatomic16_get_and_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_and_rlx(a, v);
}
    #define VATOMIC16_GET_OR_REL
static inline vuint16_t
vatomic16_get_or_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_or_rlx(a, v);
}
    #define VATOMIC16_GET_XOR_REL
static inline vuint16_t
vatomic16_get_xor_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_xor_rlx(a, v);
}
    #define VATOMIC16_GET_ADD_REL
static inline vuint16_t
vatomic16_get_add_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_add_rlx(a, v);
}
    #define VATOMIC16_GET_SUB_REL
static inline vuint16_t
vatomic16_get_sub_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_get_sub_rlx(a, v);
}
    #define VATOMIC16_MAX_GET_REL
static inline vuint16_t
vatomic16_max_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_max_get_rlx(a, v);
}
    #define VATOMIC16_AND_GET_REL
static inline vuint16_t
vatomic16_and_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_and_get_rlx(a, v);
}
    #define VATOMIC16_OR_GET_REL
static inline vuint16_t
vatomic16_or_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_or_get_rlx(a, v);
}
    #define VATOMIC16_XOR_GET_REL
static inline vuint16_t
vatomic16_xor_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_xor_get_rlx(a, v);
}
    #define VATOMIC16_ADD_GET_REL
static inline vuint16_t
vatomic16_add_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_add_get_rlx(a, v);
}
    #define VATOMIC16_SUB_GET_REL
static inline vuint16_t
vatomic16_sub_get_rel(vatomic16_t *a, vuint16_t v)
{
    return vatomic16_sub_get_rlx(a, v);
}

    #define VATOMIC16_GET_INC
static inline vuint16_t
vatomic16_get_inc(vatomic16_t *a)
{
    return vatomic16_get_inc_rlx(a);
}
    #define VATOMIC16_INC_GET
static inline vuint16_t
vatomic16_inc_get(vatomic16_t *a)
{
    return vatomic16_inc_get_rlx(a);
}
    #define VATOMIC16_GET_DEC
static inline vuint16_t
vatomic16_get_dec(vatomic16_t *a)
{
    return vatomic16_get_dec_rlx(a);
}
    #define VATOMIC16_DEC_GET
static inline vuint16_t
vatomic16_dec_get(vatomic16_t *a)
{
    return vatomic16_dec_get_rlx(a);
}
    #define VATOMIC16_GET_INC_ACQ
static inline vuint16_t
vatomic16_get_inc_acq(vatomic16_t *a)
{
    return vatomic16_get_inc_rlx(a);
}
    #define VATOMIC16_INC_GET_ACQ
static inline vuint16_t
vatomic16_inc_get_acq(vatomic16_t *a)
{
    return vatomic16_inc_get_rlx(a);
}
    #define VATOMIC16_GET_DEC_ACQ
static inline vuint16_t
vatomic16_get_dec_acq(vatomic16_t *a)
{
    return vatomic16_get_dec_rlx(a);
}
    #define VATOMIC16_DEC_GET_ACQ
static inline vuint16_t
vatomic16_dec_get_acq(vatomic16_t *a)
{
    return vatomic16_dec_get_rlx(a);
}
    #define VATOMIC16_GET_INC_REL
static inline vuint16_t
vatomic16_get_inc_rel(vatomic16_t *a)
{
    return vatomic16_get_inc_rlx(a);
}
    #define VATOMIC16_INC_GET_REL
static inline vuint16_t
vatomic16_inc_get_rel(vatomic16_t *a)
{
    return vatomic16_inc_get_rlx(a);
}
    #define VATOMIC16_GET_DEC_REL
static inline vuint16_t
vatomic16_get_dec_rel(vatomic16_t *a)
{
    return vatomic16_get_dec_rlx(a);
}
    #define VATOMIC16_DEC_GET_REL
static inline vuint16_t
vatomic16_dec_get_rel(vatomic16_t *a)
{
    return vatomic16_dec_get_rlx(a);
}

    #define VATOMIC16_MAX
static inline void
vatomic16_max(vatomic16_t *a, vuint16_t v)
{
    vatomic16_max_rlx(a, v);
}
    #define VATOMIC16_AND
static inline void
vatomic16_and(vatomic16_t *a, vuint16_t v)
{
    vatomic16_and_rlx(a, v);
}
    #define VATOMIC16_OR
static inline void
vatomic16_or(vatomic16_t *a, vuint16_t v)
{
    vatomic16_or_rlx(a, v);
}
    #define VATOMIC16_XOR
static inline void
vatomic16_xor(vatomic16_t *a, vuint16_t v)
{
    vatomic16_xor_rlx(a, v);
}
    #define VATOMIC16_ADD
static inline void
vatomic16_add(vatomic16_t *a, vuint16_t v)
{
    vatomic16_add_rlx(a, v);
}
    #define VATOMIC16_SUB
static inline void
vatomic16_sub(vatomic16_t *a, vuint16_t v)
{
    vatomic16_sub_rlx(a, v);
}
    #define VATOMIC16_MAX_REL
static inline void
vatomic16_max_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_max_rlx(a, v);
}
    #define VATOMIC16_AND_REL
static inline void
vatomic16_and_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_and_rlx(a, v);
}
    #define VATOMIC16_OR_REL
static inline void
vatomic16_or_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_or_rlx(a, v);
}
    #define VATOMIC16_XOR_REL
static inline void
vatomic16_xor_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_xor_rlx(a, v);
}
    #define VATOMIC16_ADD_REL
static inline void
vatomic16_add_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_add_rlx(a, v);
}
    #define VATOMIC16_SUB_REL
static inline void
vatomic16_sub_rel(vatomic16_t *a, vuint16_t v)
{
    vatomic16_sub_rlx(a, v);
}

    #define VATOMIC16_INC
static inline void
vatomic16_inc(vatomic16_t *a)
{
    vatomic16_inc_rlx(a);
}
    #define VATOMIC16_DEC
static inline void
vatomic16_dec(vatomic16_t *a)
{
    vatomic16_dec_rlx(a);
}
    #define VATOMIC16_INC_REL
static inline void
vatomic16_inc_rel(vatomic16_t *a)
{
    vatomic16_inc_rlx(a);
}
    #define VATOMIC16_DEC_REL
static inline void
vatomic16_dec_rel(vatomic16_t *a)
{
    vatomic16_dec_rlx(a);
}

#endif
#endif
