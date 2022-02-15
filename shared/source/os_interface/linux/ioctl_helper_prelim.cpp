/*
 * Copyright (C) 2021-2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/debug_settings/debug_settings_manager.h"
#include "shared/source/helpers/debug_helpers.h"
#include "shared/source/os_interface/linux/cache_info.h"
#include "shared/source/os_interface/linux/ioctl_helper.h"

#include "third_party/uapi/prelim/drm/i915_drm.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/ioctl.h>

namespace NEO {

std::string IoctlHelperPrelim20::getIoctlString(unsigned long request) {
    switch (request) {
    case PRELIM_DRM_IOCTL_I915_GEM_VM_BIND:
        return "PRELIM_DRM_IOCTL_I915_GEM_VM_BIND";
    case PRELIM_DRM_IOCTL_I915_GEM_VM_UNBIND:
        return "PRELIM_DRM_IOCTL_I915_GEM_VM_UNBIND";
    case PRELIM_DRM_IOCTL_I915_GEM_WAIT_USER_FENCE:
        return "PRELIM_DRM_IOCTL_I915_GEM_WAIT_USER_FENCE";
    case PRELIM_DRM_IOCTL_I915_GEM_CREATE_EXT:
        return "PRELIM_DRM_IOCTL_I915_GEM_CREATE_EXT";
    case PRELIM_DRM_IOCTL_I915_GEM_VM_ADVISE:
        return "PRELIM_DRM_IOCTL_I915_GEM_VM_ADVISE";
    case PRELIM_DRM_IOCTL_I915_GEM_VM_PREFETCH:
        return "PRELIM_DRM_IOCTL_I915_GEM_VM_PREFETCH";
    case PRELIM_DRM_IOCTL_I915_UUID_REGISTER:
        return "PRELIM_DRM_IOCTL_I915_UUID_REGISTER";
    case PRELIM_DRM_IOCTL_I915_UUID_UNREGISTER:
        return "PRELIM_DRM_IOCTL_I915_UUID_UNREGISTER";
    case PRELIM_DRM_IOCTL_I915_DEBUGGER_OPEN:
        return "PRELIM_DRM_IOCTL_I915_DEBUGGER_OPEN";
    case PRELIM_DRM_IOCTL_I915_GEM_CLOS_RESERVE:
        return "PRELIM_DRM_IOCTL_I915_GEM_CLOS_RESERVE";
    case PRELIM_DRM_IOCTL_I915_GEM_CLOS_FREE:
        return "PRELIM_DRM_IOCTL_I915_GEM_CLOS_FREE";
    case PRELIM_DRM_IOCTL_I915_GEM_CACHE_RESERVE:
        return "PRELIM_DRM_IOCTL_I915_GEM_CACHE_RESERVE";
    case DRM_IOCTL_I915_GEM_MMAP_GTT:
        return "DRM_IOCTL_I915_GEM_MMAP_GTT";
    case DRM_IOCTL_I915_GEM_MMAP_OFFSET:
        return "DRM_IOCTL_I915_GEM_MMAP_OFFSET";
    case DRM_IOCTL_I915_GEM_VM_CREATE:
        return "DRM_IOCTL_I915_GEM_VM_CREATE";
    case DRM_IOCTL_I915_GEM_VM_DESTROY:
        return "DRM_IOCTL_I915_GEM_VM_DESTROY";
    default:
        return IoctlHelper::getIoctlString(request);
    }
}

std::string IoctlHelperPrelim20::getIoctlParamString(int param) {
    switch (param) {
    case PRELIM_I915_PARAM_HAS_VM_BIND:
        return "PRELIM_I915_PARAM_HAS_VM_BIND";
    default:
        return IoctlHelper::getIoctlParamString(param);
    }
}

IoctlHelper *IoctlHelperPrelim20::clone() {
    return new IoctlHelperPrelim20{};
}

bool IoctlHelperPrelim20::isVmBindAvailable(Drm *drm) {
    int vmBindSupported = 0;
    drm_i915_getparam_t getParam = {};
    getParam.param = PRELIM_I915_PARAM_HAS_VM_BIND;
    getParam.value = &vmBindSupported;
    int retVal = IoctlHelper::ioctl(drm, DRM_IOCTL_I915_GETPARAM, &getParam);
    if (retVal) {
        return false;
    }
    return vmBindSupported;
}

uint32_t IoctlHelperPrelim20::createGemExt(Drm *drm, const MemRegionsVec &memClassInstances, size_t allocSize, uint32_t &handle) {
    uint32_t regionsSize = static_cast<uint32_t>(memClassInstances.size());
    std::vector<prelim_drm_i915_gem_memory_class_instance> regions(regionsSize);
    for (uint32_t i = 0; i < regionsSize; i++) {
        regions[i].memory_class = memClassInstances[i].memoryClass;
        regions[i].memory_instance = memClassInstances[i].memoryInstance;
    }
    prelim_drm_i915_gem_object_param regionParam{};
    regionParam.size = regionsSize;
    regionParam.data = reinterpret_cast<uintptr_t>(regions.data());
    regionParam.param = PRELIM_I915_OBJECT_PARAM | PRELIM_I915_PARAM_MEMORY_REGIONS;

    prelim_drm_i915_gem_create_ext_setparam setparamRegion{};
    setparamRegion.base.name = PRELIM_I915_GEM_CREATE_EXT_SETPARAM;
    setparamRegion.param = regionParam;

    prelim_drm_i915_gem_create_ext createExt{};
    createExt.size = allocSize;
    createExt.extensions = reinterpret_cast<uintptr_t>(&setparamRegion);

    printDebugString(DebugManager.flags.PrintBOCreateDestroyResult.get(), stdout, "Performing GEM_CREATE_EXT with { size: %lu, param: 0x%llX",
                     allocSize, regionParam.param);

    if (DebugManager.flags.PrintBOCreateDestroyResult.get()) {
        for (uint32_t i = 0; i < regionsSize; i++) {
            auto region = regions[i];
            printDebugString(DebugManager.flags.PrintBOCreateDestroyResult.get(), stdout, ", memory class: %d, memory instance: %d",
                             region.memory_class, region.memory_instance);
        }
        printDebugString(DebugManager.flags.PrintBOCreateDestroyResult.get(), stdout, "%s", " }\n");
    }

    auto ret = IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_CREATE_EXT, &createExt);

    printDebugString(DebugManager.flags.PrintBOCreateDestroyResult.get(), stdout, "GEM_CREATE_EXT has returned: %d BO-%u with size: %lu\n", ret, createExt.handle, createExt.size);
    handle = createExt.handle;
    return ret;
}

std::vector<MemoryRegion> IoctlHelperPrelim20::translateToMemoryRegions(const std::vector<uint8_t> &regionInfo) {
    auto *data = reinterpret_cast<const prelim_drm_i915_query_memory_regions *>(regionInfo.data());
    auto memRegions = std::vector<MemoryRegion>(data->num_regions);
    for (uint32_t i = 0; i < data->num_regions; i++) {
        memRegions[i].probedSize = data->regions[i].probed_size;
        memRegions[i].unallocatedSize = data->regions[i].unallocated_size;
        memRegions[i].region.memoryClass = data->regions[i].region.memory_class;
        memRegions[i].region.memoryInstance = data->regions[i].region.memory_instance;
    }
    return memRegions;
}

CacheRegion IoctlHelperPrelim20::closAlloc(Drm *drm) {
    struct prelim_drm_i915_gem_clos_reserve clos = {};

    int ret = IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_CLOS_RESERVE, &clos);
    if (ret != 0) {
        int err = errno;
        printDebugString(DebugManager.flags.PrintDebugMessages.get(), stderr, "ioctl(I915_GEM_CLOS_RESERVE) failed with %d. errno=%d(%s)\n", ret, err, strerror(err));
        DEBUG_BREAK_IF(true);
        return CacheRegion::None;
    }

    return static_cast<CacheRegion>(clos.clos_index);
}

uint16_t IoctlHelperPrelim20::closAllocWays(Drm *drm, CacheRegion closIndex, uint16_t cacheLevel, uint16_t numWays) {
    struct prelim_drm_i915_gem_cache_reserve cache = {};

    cache.clos_index = static_cast<uint16_t>(closIndex);
    cache.cache_level = cacheLevel;
    cache.num_ways = numWays;

    int ret = IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_CACHE_RESERVE, &cache);
    if (ret != 0) {
        int err = errno;
        PRINT_DEBUG_STRING(DebugManager.flags.PrintDebugMessages.get(), stderr, "ioctl(I915_GEM_CACHE_RESERVE) failed with %d. errno=%d(%s)\n", ret, err, strerror(err));
        return 0;
    }

    return cache.num_ways;
}

CacheRegion IoctlHelperPrelim20::closFree(Drm *drm, CacheRegion closIndex) {
    struct prelim_drm_i915_gem_clos_free clos = {};

    clos.clos_index = static_cast<uint16_t>(closIndex);

    int ret = IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_CLOS_FREE, &clos);
    if (ret != 0) {
        int err = errno;
        printDebugString(DebugManager.flags.PrintDebugMessages.get(), stderr, "ioctl(I915_GEM_CLOS_FREE) failed with %d. errno=%d(%s)\n", ret, err, strerror(err));
        DEBUG_BREAK_IF(true);
        return CacheRegion::None;
    }

    return closIndex;
}

int IoctlHelperPrelim20::waitUserFence(Drm *drm, uint32_t ctxId, uint64_t address,
                                       uint64_t value, uint32_t dataWidth, int64_t timeout, uint16_t flags) {
    prelim_drm_i915_gem_wait_user_fence wait = {};

    wait.ctx_id = ctxId;
    wait.flags = flags;

    switch (dataWidth) {
    case 3u:
        wait.mask = PRELIM_I915_UFENCE_WAIT_U64;
        break;
    case 2u:
        wait.mask = PRELIM_I915_UFENCE_WAIT_U32;
        break;
    case 1u:
        wait.mask = PRELIM_I915_UFENCE_WAIT_U16;
        break;
    default:
        wait.mask = PRELIM_I915_UFENCE_WAIT_U8;
        break;
    }

    wait.op = PRELIM_I915_UFENCE_WAIT_GTE;
    wait.addr = address;
    wait.value = value;
    wait.timeout = timeout;

    return IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_WAIT_USER_FENCE, &wait);
}

uint32_t IoctlHelperPrelim20::getHwConfigIoctlVal() {
    return PRELIM_DRM_I915_QUERY_HWCONFIG_TABLE;
}

uint32_t IoctlHelperPrelim20::getAtomicAdvise(bool isNonAtomic) {
    return isNonAtomic ? PRELIM_I915_VM_ADVISE_ATOMIC_NONE : PRELIM_I915_VM_ADVISE_ATOMIC_SYSTEM;
}

uint32_t IoctlHelperPrelim20::getPreferredLocationAdvise() {
    return PRELIM_I915_VM_ADVISE_PREFERRED_LOCATION;
}

bool IoctlHelperPrelim20::setVmBoAdvise(Drm *drm, int32_t handle, uint32_t attribute, void *region) {
    prelim_drm_i915_gem_vm_advise vmAdvise{};

    vmAdvise.handle = handle;
    vmAdvise.attribute = attribute;
    if (region != nullptr) {
        vmAdvise.region = *reinterpret_cast<prelim_drm_i915_gem_memory_class_instance *>(region);
    }

    int ret = IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_VM_ADVISE, &vmAdvise);
    if (ret != 0) {
        int err = errno;
        PRINT_DEBUG_STRING(DebugManager.flags.PrintDebugMessages.get(), stderr, "ioctl(PRELIM_DRM_I915_GEM_VM_ADVISE) failed with %d. errno=%d(%s)\n", ret, err, strerror(err));
        DEBUG_BREAK_IF(true);
        return false;
    }
    return true;
}

uint32_t IoctlHelperPrelim20::getDirectSubmissionFlag() {
    return PRELIM_I915_CONTEXT_CREATE_FLAGS_ULLS;
}

int32_t IoctlHelperPrelim20::getMemRegionsIoctlVal() {
    return PRELIM_DRM_I915_QUERY_MEMORY_REGIONS;
}

int32_t IoctlHelperPrelim20::getEngineInfoIoctlVal() {
    return PRELIM_DRM_I915_QUERY_ENGINE_INFO;
}

uint32_t IoctlHelperPrelim20::getComputeSlicesIoctlVal() {
    return PRELIM_DRM_I915_QUERY_COMPUTE_SLICES;
}

uint16_t IoctlHelperPrelim20::getWaitUserFenceSoftFlag() {
    return PRELIM_I915_UFENCE_WAIT_SOFT;
};

std::unique_ptr<uint8_t[]> IoctlHelperPrelim20::prepareVmBindExt(const StackVec<uint32_t, 2> &bindExtHandles) {
    std::unique_ptr<prelim_drm_i915_vm_bind_ext_uuid[]> extensions;
    extensions = std::make_unique<prelim_drm_i915_vm_bind_ext_uuid[]>(bindExtHandles.size());
    memset(extensions.get(), 0, sizeof(prelim_drm_i915_vm_bind_ext_uuid) * bindExtHandles.size());

    extensions[0].uuid_handle = bindExtHandles[0];
    extensions[0].base.name = PRELIM_I915_VM_BIND_EXT_UUID;

    for (size_t i = 1; i < bindExtHandles.size(); i++) {
        extensions[i - 1].base.next_extension = reinterpret_cast<uint64_t>(&extensions[i]);
        extensions[i].uuid_handle = bindExtHandles[i];
        extensions[i].base.name = PRELIM_I915_VM_BIND_EXT_UUID;
    }
    return std::unique_ptr<uint8_t[]>(reinterpret_cast<uint8_t *>(extensions.release()));
}

uint64_t IoctlHelperPrelim20::getFlagsForVmBind(bool bindCapture, bool bindImmediate, bool bindMakeResident) {
    uint64_t flags = 0u;
    if (bindCapture) {
        flags |= PRELIM_I915_GEM_VM_BIND_CAPTURE;
    }
    if (bindImmediate) {
        flags |= PRELIM_I915_GEM_VM_BIND_IMMEDIATE;
    }
    if (bindMakeResident) {
        flags |= PRELIM_I915_GEM_VM_BIND_MAKE_RESIDENT;
    }
    return flags;
}

std::vector<EngineCapabilities> IoctlHelperPrelim20::translateToEngineCaps(const std::vector<uint8_t> &data) {
    auto engineInfo = reinterpret_cast<const prelim_drm_i915_query_engine_info *>(data.data());
    std::vector<EngineCapabilities> engines;
    engines.reserve(engineInfo->num_engines);
    for (uint32_t i = 0; i < engineInfo->num_engines; i++) {
        EngineCapabilities engine{};
        engine.capabilities = engineInfo->engines[i].capabilities;
        engine.engine.engineClass = engineInfo->engines[i].engine.engine_class;
        engine.engine.engineInstance = engineInfo->engines[i].engine.engine_instance;
        engines.push_back(engine);
    }
    return engines;
}

prelim_drm_i915_query_distance_info translateToi915(const DistanceInfo &distanceInfo) {
    prelim_drm_i915_query_distance_info dist{};
    dist.engine.engine_class = distanceInfo.engine.engineClass;
    dist.engine.engine_instance = distanceInfo.engine.engineInstance;

    dist.region.memory_class = distanceInfo.region.memoryClass;
    dist.region.memory_instance = distanceInfo.region.memoryInstance;
    return dist;
}

uint32_t IoctlHelperPrelim20::queryDistances(Drm *drm, std::vector<drm_i915_query_item> &queryItems, std::vector<DistanceInfo> &distanceInfos) {
    std::vector<prelim_drm_i915_query_distance_info> i915Distances(distanceInfos.size());
    std::transform(distanceInfos.begin(), distanceInfos.end(), i915Distances.begin(), translateToi915);

    for (auto i = 0u; i < i915Distances.size(); i++) {
        queryItems[i].query_id = PRELIM_DRM_I915_QUERY_DISTANCE_INFO;
        queryItems[i].length = sizeof(prelim_drm_i915_query_distance_info);
        queryItems[i].flags = 0u;
        queryItems[i].data_ptr = reinterpret_cast<__u64>(&i915Distances[i]);
    }

    drm_i915_query query{};
    query.items_ptr = reinterpret_cast<__u64>(queryItems.data());
    query.num_items = static_cast<uint32_t>(queryItems.size());
    auto ret = IoctlHelper::ioctl(drm, DRM_IOCTL_I915_QUERY, &query);
    for (auto i = 0u; i < i915Distances.size(); i++) {
        distanceInfos[i].distance = i915Distances[i].distance;
    }
    return ret;
}

int32_t IoctlHelperPrelim20::getComputeEngineClass() {
    return PRELIM_I915_ENGINE_CLASS_COMPUTE;
}

std::optional<int> IoctlHelperPrelim20::getHasPageFaultParamId() {
    return PRELIM_I915_PARAM_HAS_PAGE_FAULT;
};

uint32_t gemCreateContextExt(Drm *drm, drm_i915_gem_context_create_ext &gcc, drm_i915_gem_context_create_ext_setparam &extSetparam) {
    gcc.flags |= I915_CONTEXT_CREATE_FLAGS_USE_EXTENSIONS;
    extSetparam.base.next_extension = gcc.extensions;
    gcc.extensions = reinterpret_cast<uint64_t>(&extSetparam);

    return IoctlHelper::ioctl(drm, DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT, &gcc);
}

uint32_t gemCreateContextAcc(Drm *drm, drm_i915_gem_context_create_ext &gcc, uint16_t trigger, uint8_t granularity) {
    prelim_drm_i915_gem_context_param_acc paramAcc = {};
    paramAcc.trigger = trigger;
    paramAcc.notify = 1;
    paramAcc.granularity = granularity;

    i915_user_extension userExt = {};
    userExt.name = I915_CONTEXT_CREATE_EXT_SETPARAM;

    drm_i915_gem_context_param ctxParam = {};
    ctxParam.param = PRELIM_I915_CONTEXT_PARAM_ACC;
    ctxParam.ctx_id = 0;
    ctxParam.size = sizeof(paramAcc);
    ctxParam.value = reinterpret_cast<uint64_t>(&paramAcc);

    drm_i915_gem_context_create_ext_setparam extSetparam = {};
    extSetparam.base = userExt;
    extSetparam.param = ctxParam;

    return gemCreateContextExt(drm, gcc, extSetparam);
}
uint32_t IoctlHelperPrelim20::createContextWithAccessCounters(Drm *drm, drm_i915_gem_context_create_ext &gcc) {
    uint16_t trigger = 0;
    if (DebugManager.flags.AccessCountersTrigger.get() != -1) {
        trigger = static_cast<uint16_t>(DebugManager.flags.AccessCountersTrigger.get());
    }
    uint8_t granularity = PRELIM_I915_CONTEXT_ACG_2M;
    if (DebugManager.flags.AccessCountersGranularity.get() != -1) {
        granularity = static_cast<uint8_t>(DebugManager.flags.AccessCountersGranularity.get());
    }
    return gemCreateContextAcc(drm, gcc, trigger, granularity);
}

uint32_t IoctlHelperPrelim20::createCooperativeContext(Drm *drm, drm_i915_gem_context_create_ext &gcc) {
    struct drm_i915_gem_context_create_ext_setparam extSetparam = {};
    extSetparam.base.name = I915_CONTEXT_CREATE_EXT_SETPARAM;
    extSetparam.param.param = PRELIM_I915_CONTEXT_PARAM_RUNALONE;
    return gemCreateContextExt(drm, gcc, extSetparam);
}

std::unique_ptr<uint8_t[]> IoctlHelperPrelim20::createVmBindExtSetPat() {
    return std::make_unique<uint8_t[]>(sizeof(prelim_drm_i915_vm_bind_ext_set_pat));
};

void IoctlHelperPrelim20::fillVmBindExtSetPat(const std::unique_ptr<uint8_t[]> &vmBindExtSetPat, uint64_t patIndex, uint64_t nextExtension) {
    auto prelimVmBindExtSetPat = reinterpret_cast<prelim_drm_i915_vm_bind_ext_set_pat *>(vmBindExtSetPat.get());
    UNRECOVERABLE_IF(!prelimVmBindExtSetPat);
    prelimVmBindExtSetPat->base.name = PRELIM_I915_VM_BIND_EXT_SET_PAT;
    prelimVmBindExtSetPat->pat_index = patIndex;
    prelimVmBindExtSetPat->base.next_extension = nextExtension;
}

std::unique_ptr<uint8_t[]> IoctlHelperPrelim20::createVmBindExtSyncFence() {
    return std::make_unique<uint8_t[]>(sizeof(prelim_drm_i915_vm_bind_ext_sync_fence));
}

void IoctlHelperPrelim20::fillVmBindExtSyncFence(const std::unique_ptr<uint8_t[]> &vmBindExtSyncFence, uint64_t fenceAddress, uint64_t fenceValue, uint64_t nextExtension) {
    auto prelimVmBindExtSyncFence = reinterpret_cast<prelim_drm_i915_vm_bind_ext_sync_fence *>(vmBindExtSyncFence.get());
    UNRECOVERABLE_IF(!prelimVmBindExtSyncFence);
    prelimVmBindExtSyncFence->base.name = PRELIM_I915_VM_BIND_EXT_SYNC_FENCE;
    prelimVmBindExtSyncFence->base.next_extension = nextExtension;
    prelimVmBindExtSyncFence->addr = fenceAddress;
    prelimVmBindExtSyncFence->val = fenceValue;
}

std::optional<uint64_t> IoctlHelperPrelim20::getCopyClassSaturatePCIECapability() {
    return PRELIM_I915_COPY_CLASS_CAP_SATURATE_PCIE;
}

std::optional<uint64_t> IoctlHelperPrelim20::getCopyClassSaturateLinkCapability() {
    return PRELIM_I915_COPY_CLASS_CAP_SATURATE_LINK;
}

prelim_drm_i915_gem_vm_bind translateVmBindParamsToPrelimStruct(const VmBindParams &vmBindParams) {
    prelim_drm_i915_gem_vm_bind vmBind{};
    vmBind.vm_id = vmBindParams.vmId;
    vmBind.handle = vmBindParams.handle;
    vmBind.start = vmBindParams.start;
    vmBind.offset = vmBindParams.offset;
    vmBind.length = vmBindParams.length;
    vmBind.flags = vmBindParams.flags;
    vmBind.extensions = vmBindParams.extensions;
    return vmBind;
}

int IoctlHelperPrelim20::vmBind(Drm *drm, const VmBindParams &vmBindParams) {
    auto prelimVmBind = translateVmBindParamsToPrelimStruct(vmBindParams);
    return IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_VM_BIND, &prelimVmBind);
}

int IoctlHelperPrelim20::vmUnbind(Drm *drm, const VmBindParams &vmBindParams) {
    auto prelimVmBind = translateVmBindParamsToPrelimStruct(vmBindParams);
    return IoctlHelper::ioctl(drm, PRELIM_DRM_IOCTL_I915_GEM_VM_UNBIND, &prelimVmBind);
}
} // namespace NEO
