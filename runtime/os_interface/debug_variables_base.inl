/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

/*SIMULATION FLAGS*/
DECLARE_DEBUG_VARIABLE(std::string, TbxServer, std::string("127.0.0.1"), "TCP-IP address of TBX server")
DECLARE_DEBUG_VARIABLE(std::string, ProductFamilyOverride, std::string("unk"), "Specify product for use in AUB/TBX")
DECLARE_DEBUG_VARIABLE(std::string, HardwareInfoOverride, std::string("default"), "Specify hardware info config, i.e 1x4x8, for use in AUB/TBX")
DECLARE_DEBUG_VARIABLE(std::string, ForceCompilerUsePlatform, std::string("unk"), "Specify product for use in compiler interface")
DECLARE_DEBUG_VARIABLE(std::string, AUBDumpCaptureFileName, std::string("unk"), "Name of file to save AUB capture into")
DECLARE_DEBUG_VARIABLE(std::string, AUBDumpFilterKernelName, std::string("unk"), "Name of kernel to AUB capture")
DECLARE_DEBUG_VARIABLE(std::string, AUBDumpToggleFileName, std::string("unk"), "Name of file to save AUB in toggle mode")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpFilterNamedKernelStartIdx, 0, "Start index of named kernel to AUB capture")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpFilterNamedKernelEndIdx, -1, "End index of named kernel to AUB capture")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpSubCaptureMode, 0, "AUB dump subcapture mode (off, toggle, filter)")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpFilterKernelStartIdx, 0, "Start index of kernel to AUB capture")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpFilterKernelEndIdx, -1, "End index of kernel to AUB capture")
DECLARE_DEBUG_VARIABLE(int32_t, AUBDumpToggleCaptureOnOff, 0, "Toggle AUB capture on/off")
DECLARE_DEBUG_VARIABLE(int32_t, AubDumpOverrideMmioRegister, 0, "Override mmio offset from list with new value from AubDumpOverrideMmioRegisterValue")
DECLARE_DEBUG_VARIABLE(int32_t, AubDumpOverrideMmioRegisterValue, 0, "Value to override mmio offset from AubDumpOverrideMmioRegister")
DECLARE_DEBUG_VARIABLE(std::string, AubDumpAddMmioRegistersList, std::string("unk"), "Semicolon separated sequence of additional MMIO registers offset;values pairs i.e. 0x111;0x123;0x222;0x456")
DECLARE_DEBUG_VARIABLE(int32_t, SetCommandStreamReceiver, 0, "Set command stream receiver to: 0 - HW, 1 - AUB, 2 - TBX, 3 - HW & AUB, 4 - TBX & AUB")
DECLARE_DEBUG_VARIABLE(int32_t, TbxPort, 4321, "TCP-IP port of TBX server")
DECLARE_DEBUG_VARIABLE(bool, FlattenBatchBufferForAUBDump, false, "Dump multi-level batch buffers to AUB as single, flat batch buffer")
DECLARE_DEBUG_VARIABLE(bool, AddPatchInfoCommentsForAUBDump, false, "Dump comments containing allocations and patching information")
DECLARE_DEBUG_VARIABLE(bool, UseAubStream, true, "Use aub_stream for aub dumping")
DECLARE_DEBUG_VARIABLE(bool, AUBDumpForceAllToLocalMemory, false, "Force placing every allocation in local memory address space")
DECLARE_DEBUG_VARIABLE(bool, AUBDumpConcurrentCS, false, "Enable concurrent execution on CS (disabled by default)")

/*DEBUG FLAGS*/
DECLARE_DEBUG_VARIABLE(int32_t, SchedulerSimulationReturnInstance, 0, "prints execution model related debug information")
DECLARE_DEBUG_VARIABLE(int32_t, SchedulerGWS, 0, "Forces gws of scheduler kernel, only multiple of 24 allowed or 0 - default selected")
DECLARE_DEBUG_VARIABLE(int32_t, EnableExperimentalCommandBuffer, 0, "Enables injection of experimental command buffer")
DECLARE_DEBUG_VARIABLE(bool, EnableDebugBreak, true, "Enable DEBUG_BREAKs")
DECLARE_DEBUG_VARIABLE(bool, FlushAllCaches, false, "pipe controls between enqueues flush all possible caches")
DECLARE_DEBUG_VARIABLE(bool, MakeEachEnqueueBlocking, false, "equivalent of finish after each enqueue")
DECLARE_DEBUG_VARIABLE(bool, DoCpuCopyOnReadBuffer, false, "triggers CPU copy path for Read Buffer calls, only supported for some basic use cases ( no events, not blocked calls )")
DECLARE_DEBUG_VARIABLE(bool, DoCpuCopyOnWriteBuffer, false, "triggers CPU copy path for Write Buffer calls, only supported for some basic use cases ( no events, not blocked calls )")
DECLARE_DEBUG_VARIABLE(bool, DisableResourceRecycling, false, "when set to true disables resource recycling optimization")
DECLARE_DEBUG_VARIABLE(bool, ForceDispatchScheduler, false, "dispatches scheduler kernel instead of kernel enqueued")
DECLARE_DEBUG_VARIABLE(bool, TrackParentEvents, false, "events track their parents")
DECLARE_DEBUG_VARIABLE(bool, RebuildPrecompiledKernels, false, "forces driver to recompile precompiled kernels from sources")
DECLARE_DEBUG_VARIABLE(bool, LoopAtPlatformInitialize, false, "Adds endless loop in platform initalize, useful for debugging.")
DECLARE_DEBUG_VARIABLE(bool, DoNotRegisterTrimCallback, false, "When set to true driver is not registering trim callback.")
/*LOGGING FLAGS*/
DECLARE_DEBUG_VARIABLE(bool, PrintDebugMessages, false, "when enabled, some debug messages will be propagated to console")
DECLARE_DEBUG_VARIABLE(bool, DumpKernels, false, "Enables dumping kernels' program source code to text files and program from binary to bin file")
DECLARE_DEBUG_VARIABLE(bool, DumpKernelArgs, false, "Enables dumping kernels args to binary files")
DECLARE_DEBUG_VARIABLE(bool, LogApiCalls, false, "Enables logging api function calls, inputs and outputs to file")
DECLARE_DEBUG_VARIABLE(bool, LogPatchTokens, false, "Enables logging patch tokens, inputs and outputs to file")
DECLARE_DEBUG_VARIABLE(bool, LogTaskCounts, false, "Enables logging taskCounts and taskLevels to file")
DECLARE_DEBUG_VARIABLE(bool, LogAlignedAllocations, false, "Logs alignedMalloc and alignedFree allocations")
DECLARE_DEBUG_VARIABLE(bool, LogMemoryObject, false, "Logs memory object ptrs, sizes and operations")
DECLARE_DEBUG_VARIABLE(bool, ResidencyDebugEnable, 0, "enables debug messages and checks for Residency Model")
DECLARE_DEBUG_VARIABLE(bool, EventsDebugEnable, 0, "enables debug messages for events, virtual events, blocked enqueues, events trees etc.")
DECLARE_DEBUG_VARIABLE(bool, EventsTrackerEnable, false, "enables event graphs dumping")
DECLARE_DEBUG_VARIABLE(bool, PrintEMDebugInformation, false, "prints execution model related debug information")
DECLARE_DEBUG_VARIABLE(bool, PrintLWSSizes, false, "prints driver choosen local workgroup sizes")
DECLARE_DEBUG_VARIABLE(bool, PrintDispatchParameters, false, "prints dispatch paramters of kernels passed to clEnqueueNDRangeKernel")
DECLARE_DEBUG_VARIABLE(int32_t, PrintDriverDiagnostics, -1, "prints driver diagnostics messages to standard output, value corresponds to hint level")
/*PERFORMANCE FLAGS*/
DECLARE_DEBUG_VARIABLE(bool, EnableNullHardware, false, "works on Windows only, sets the Null Hardware flag that makes all Command buffers completed while GPU does nothing")
DECLARE_DEBUG_VARIABLE(bool, ForceLinearImages, false, "Force linear images. Default is Y-tiled.")
DECLARE_DEBUG_VARIABLE(bool, ForceSLML3Config, false, "Forces L3Config with SLM for all kernels")
DECLARE_DEBUG_VARIABLE(bool, Force32bitAddressing, false, "Forces 32 bit addresses to be used in 64 bit dll")
DECLARE_DEBUG_VARIABLE(bool, ForceCsrFlushing, false, "Forces flushing of command stream receiver")
DECLARE_DEBUG_VARIABLE(bool, ForceCsrReprogramming, false, "Forces reprogramming of command stream receiver")
DECLARE_DEBUG_VARIABLE(bool, DisableStatelessToStatefulOptimization, false, "Disables stateless to stateful optimization for buffers")
DECLARE_DEBUG_VARIABLE(bool, DisableConcurrentBlockExecution, false, "disables concurrent block kernel execution")
DECLARE_DEBUG_VARIABLE(bool, UseNewHeapAllocator, true, "Custom 4GB heap allocator is used")
DECLARE_DEBUG_VARIABLE(bool, UseNoRingFlushesKmdMode, true, "Windows only, passes flag to KMD that informs KMD to not emit any ring buffer flushes.")
DECLARE_DEBUG_VARIABLE(bool, DisableZeroCopyForUseHostPtr, false, "When active all buffer allocations created with CL_MEM_USE_HOST_PTR flag will not share memory with CPU.")
DECLARE_DEBUG_VARIABLE(bool, DisableZeroCopyForBuffers, false, "When active all buffer allocations will not share memory with CPU.")
DECLARE_DEBUG_VARIABLE(bool, ForceResourceLockOnTransferCalls, false, "Forces resource locking on memory transfer calls")
DECLARE_DEBUG_VARIABLE(bool, EnableMakeResidentOnMapGpuVa, false, "Make allocations resident on call mapGpuVirtualAddress")
DECLARE_DEBUG_VARIABLE(bool, EnableHostPtrTracking, true, "Enable host ptr tracking")

/*FEATURE FLAGS*/
DECLARE_DEBUG_VARIABLE(bool, EnableNV12, true, "Enables NV12 extension")
DECLARE_DEBUG_VARIABLE(bool, EnablePackedYuv, true, "Enables cl_packed_yuv extension")
DECLARE_DEBUG_VARIABLE(bool, EnableIntelVme, true, "Enables cl_intel_motion_estimation extension")
DECLARE_DEBUG_VARIABLE(bool, EnableIntelAdvancedVme, true, "Enables cl_intel_advanced_motion_estimation extension")
DECLARE_DEBUG_VARIABLE(bool, EnableDeferredDeleter, true, "Enables async deleter")
DECLARE_DEBUG_VARIABLE(bool, EnableAsyncDestroyAllocations, true, "Enables async destroying graphics allocations in mem obj destructor")
DECLARE_DEBUG_VARIABLE(bool, EnableAsyncEventsHandler, true, "Enables async events handler")
DECLARE_DEBUG_VARIABLE(bool, EnableForcePin, true, "Enables early pinning for memory object")
DECLARE_DEBUG_VARIABLE(bool, EnableComputeWorkSizeND, true, "Enables diffrent algorithm to compute local work size")
DECLARE_DEBUG_VARIABLE(bool, EnableComputeWorkSizeSquared, false, "Enables algorithm to compute the most squared work group as possible")
DECLARE_DEBUG_VARIABLE(bool, EnableVaLibCalls, true, "Enable cl-va sharing lib calls")
DECLARE_DEBUG_VARIABLE(bool, AddClGlSharing, false, "Add cl-gl extension")
DECLARE_DEBUG_VARIABLE(bool, EnablePassInlineData, false, "Enable passing of inline data")
DECLARE_DEBUG_VARIABLE(int32_t, EnableCacheFlushAfterWalker, 0, "-1: platform behavior, 0: disabled, 1: enabled. Adds dedicated cache flush command after WALKER command when surfaces used by kernel require to flush the cache")
DECLARE_DEBUG_VARIABLE(int32_t, EnableLocalMemory, -1, "-1: default behavior, 0: disabled, 1: enabled, Allows allocating graphics memory in Local Memory")
DECLARE_DEBUG_VARIABLE(int32_t, EnableStatelessToStatefulBufferOffsetOpt, -1, "-1: dont override, 0: disable, 1: enable, Enables buffer-offset improvement of the stateless to stateful optimization")
DECLARE_DEBUG_VARIABLE(int32_t, CreateMultipleDevices, 0, "0: default - disable, 1+: Driver will create multiple (N) devices during initialization.")
DECLARE_DEBUG_VARIABLE(int32_t, LimitAmountOfReturnedDevices, 0, "0: default - disable, 1+: Driver will limit the number of devices returned from clGetDeviceIds to N.")
DECLARE_DEBUG_VARIABLE(int32_t, Enable64kbpages, -1, "-1: default behaviour, 0 Disables, 1 Enables support for 64KB pages for driver allocated fine grain svm buffers")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideEnableKmdNotify, -1, "-1: dont override, 0: disable, 1: enable")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideKmdNotifyDelayMicroseconds, -1, "-1: dont override, 0: infinite timeout, >0: timeout in microseconds")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideEnableQuickKmdSleep, -1, "-1: dont override, 0: disable, 1: enable. It works only when Kmd Notify is enabled.")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideQuickKmdSleepDelayMicroseconds, -1, "-1: dont override, 0: infinite timeout, >0: timeout in microseconds")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideEnableQuickKmdSleepForSporadicWaits, -1, "-1: dont override, 0: disable, 1: enable. It works only when QuickKmdSleep is enabled.")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideDelayQuickKmdSleepForSporadicWaitsMicroseconds, -1, "-1: dont override, >0: timeout in microseconds")
DECLARE_DEBUG_VARIABLE(int32_t, PowerSavingMode, 0, "0: default 1: enable. Whenever driver waits on GPU and its not ready, put waiting thread to sleep and wait for notification.")
DECLARE_DEBUG_VARIABLE(int32_t, CsrDispatchMode, 0, "Chooses DispatchMode for Csr")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideDefaultFP64Settings, -1, "-1: dont override, 0: disable, 1: enable.")
DECLARE_DEBUG_VARIABLE(int32_t, RenderCompressedImagesEnabled, -1, "-1: default, 0: disabled, 1: enabled")
DECLARE_DEBUG_VARIABLE(int32_t, RenderCompressedBuffersEnabled, -1, "-1: default, 0: disabled, 1: enabled")

/*DRIVER TOGGLES*/
DECLARE_DEBUG_VARIABLE(int32_t, ForceOCLVersion, 0, "Force specific OpenCL API version")
DECLARE_DEBUG_VARIABLE(int32_t, ForcePreemptionMode, -1, "Keep this variable in sync with PreemptionMode enum. -1 - devices default mode, 1 - disable, 2 - midBatch, 3 - threadGroup, 4 - midThread")
DECLARE_DEBUG_VARIABLE(int32_t, NodeOrdinal, -1, "-1: default do not override, 0: ENGINE_RCS")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideThreadArbitrationPolicy, -1, "-1 (dont override) or any valid config (0: Age Based, 1: Round Robin)")
DECLARE_DEBUG_VARIABLE(int32_t, OverrideAubDeviceId, -1, "-1 dont override, any other: use this value for AUB generation device id")
DECLARE_DEBUG_VARIABLE(int32_t, EnableTimestampPacket, -1, "-1: default, 0: disable, 1:enable. Write Timestamp Packet for each set of gpu walkers")
DECLARE_DEBUG_VARIABLE(bool, UseMaxSimdSizeToDeduceMaxWorkgroupSize, false, "With this flag on, max workgroup size is deduced using SIMD32 instead of SIMD8, this causes the max wkg size to be 4 times bigger")
DECLARE_DEBUG_VARIABLE(bool, ReturnRawGpuTimestamps, false, "Driver returns raw GPU tiemstamps instead of calculated ones.")
