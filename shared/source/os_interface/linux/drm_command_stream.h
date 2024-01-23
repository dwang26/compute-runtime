/*
 * Copyright (C) 2018-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/command_stream/device_command_stream.h"
#include "shared/source/os_interface/linux/drm_gem_close_worker.h"
#include "shared/source/os_interface/linux/ioctl_helper.h"

#include <vector>

namespace NEO {
class BufferObject;
class Drm;
class DrmAllocation;
class DrmMemoryManager;

template <typename GfxFamily>
class DrmCommandStreamReceiver : public DeviceCommandStreamReceiver<GfxFamily> {
  protected:
    using BaseClass = DeviceCommandStreamReceiver<GfxFamily>;

    using BaseClass::getScratchPatchAddress;
    using BaseClass::makeNonResident;
    using BaseClass::makeResident;
    using BaseClass::mediaVfeStateDirty;
    using BaseClass::osContext;
    using BaseClass::requiredScratchSlot0Size;
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiver::getTagAddress;
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiver::getTagAllocation;
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiver::latestSentTaskCount;
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiver::taskCount;
    using CommandStreamReceiverHw<GfxFamily>::CommandStreamReceiver::useNotifyEnableForPostSync;

  public:
    // When drm is null default implementation is used. In this case DrmCommandStreamReceiver is responsible to free drm.
    // When drm is passed, DCSR will not free it at destruction
    DrmCommandStreamReceiver(ExecutionEnvironment &executionEnvironment,
                             uint32_t rootDeviceIndex,
                             const DeviceBitfield deviceBitfield,
                             GemCloseWorkerMode mode = GemCloseWorkerMode::gemCloseWorkerActive);
    ~DrmCommandStreamReceiver() override;

    SubmissionStatus flush(BatchBuffer &batchBuffer, ResidencyContainer &allocationsForResidency) override;
    SubmissionStatus processResidency(const ResidencyContainer &allocationsForResidency, uint32_t handleId) override;
    void makeNonResident(GraphicsAllocation &gfxAllocation) override;
    bool waitForFlushStamp(FlushStamp &flushStampToWait) override;
    bool isKmdWaitModeActive() override;
    bool isKmdWaitOnTaskCountAllowed() const override;

    DrmMemoryManager *getMemoryManager() const;
    GmmPageTableMngr *createPageTableManager() override;

    GemCloseWorkerMode peekGemCloseWorkerOperationMode() const {
        return this->gemCloseWorkerOperationMode;
    }

    void initializeDefaultsForInternalEngine() override {
        gemCloseWorkerOperationMode = GemCloseWorkerMode::gemCloseWorkerInactive;
    }

    SubmissionStatus printBOsForSubmit(ResidencyContainer &allocationsForResidency, GraphicsAllocation &cmdBufferAllocation);

    bool waitUserFence(TaskCountType waitValue, uint64_t hostAddress, int64_t timeout) override;

    using CommandStreamReceiver::pageTableManager;

  protected:
    MOCKABLE_VIRTUAL SubmissionStatus flushInternal(const BatchBuffer &batchBuffer, const ResidencyContainer &allocationsForResidency);
    MOCKABLE_VIRTUAL int exec(const BatchBuffer &batchBuffer, uint32_t vmHandleId, uint32_t drmContextId, uint32_t index);
    MOCKABLE_VIRTUAL void readBackAllocation(void *source);
    bool isUserFenceWaitActive();

    std::vector<BufferObject *> residency;
    std::vector<ExecObject> execObjectsStorage;
    Drm *drm;
    GemCloseWorkerMode gemCloseWorkerOperationMode;

    volatile uint32_t reserved = 0;
    int32_t kmdWaitTimeout = -1;

    bool useUserFenceWait = true;
    bool useContextForUserFenceWait = false;
};
} // namespace NEO
