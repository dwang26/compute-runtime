/*
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef _ZEX_METRIC_H
#define _ZEX_METRIC_H
#if defined(__cplusplus)
#pragma once
#endif

#include <level_zero/ze_api.h>
#include <level_zero/zet_api.h>

namespace L0 {
///////////////////////////////////////////////////////////////////////////////
/// @brief Get sets of metric groups which could be collected concurrently.
///
/// @details
///     - Re-arrange the input metric groups to provide sets of concurrent metric groups.
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function must be thread-safe.
///
/// @returns
///     - ::ZE_RESULT_SUCCESS
///     - ::ZE_RESULT_ERROR_INVALID_ARGUMENT
///         + `pConcurrentGroupCount` is not same as was returned by L0 using zexDeviceGetConcurrentMetricGroups
ZE_APIEXPORT ze_result_t ZE_APICALL
zexDeviceGetConcurrentMetricGroups(
    zet_device_handle_t hDevice,               // [in] handle of the device
    uint32_t metricGroupCount,                 // [in] metric group count
    zet_metric_group_handle_t *phMetricGroups, // [in, out] metrics groups to be re-arranged to be sets of concurrent groups
    uint32_t *pConcurrentGroupCount,           // [out] number of concurrent groups.
    uint32_t *pCountPerConcurrentGroup);       // [in,out][optional][*pConcurrentGroupCount] count of metric groups per concurrent group.

} // namespace L0

#endif // _ZEX_METRIC_H
