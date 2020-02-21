/*
 * Copyright (C) 2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "core/command_stream/command_stream_receiver.h"
#include "core/device/device.h"
#include "core/helpers/hw_helper.h"

namespace NEO {

EngineControl &Device::getInternalEngine() {
    if (this->engines[0].commandStreamReceiver->getType() != CommandStreamReceiverType::CSR_HW) {
        return this->getDefaultEngine();
    }
    return this->getDeviceById(0)->engines[HwHelper::internalUsageEngineIndex];
}
} // namespace NEO
