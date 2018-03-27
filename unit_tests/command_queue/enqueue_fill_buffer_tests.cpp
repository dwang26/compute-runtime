/*
 * Copyright (c) 2017, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "runtime/built_ins/built_ins.h"
#include "runtime/command_queue/command_queue.h"
#include "runtime/command_stream/command_stream_receiver.h"
#include "runtime/gen_common/reg_configs.h"
#include "runtime/helpers/ptr_math.h"
#include "runtime/helpers/aligned_memory.h"
#include "runtime/helpers/dispatch_info.h"
#include "unit_tests/command_queue/enqueue_fixture.h"
#include "unit_tests/command_queue/enqueue_fill_buffer_fixture.h"
#include "runtime/memory_manager/memory_manager.h"
#include "test.h"

using namespace OCLRT;

typedef Test<EnqueueFillBufferFixture> EnqueueFillBufferCmdTests;

HWTEST_F(EnqueueFillBufferCmdTests, alignsToCSR) {
    //this test case assumes IOQ
    auto &csr = pDevice->getUltCommandStreamReceiver<FamilyType>();
    csr.taskCount = pCmdQ->taskCount + 100;
    csr.taskLevel = pCmdQ->taskLevel + 50;

    enqueueFillBuffer<FamilyType>();
    EXPECT_EQ(csr.peekTaskCount(), pCmdQ->taskCount);
    EXPECT_EQ(csr.peekTaskLevel(), pCmdQ->taskLevel + 1);
}

HWTEST_F(EnqueueFillBufferCmdTests, bumpsTaskLevel) {
    auto taskLevelBefore = pCmdQ->taskLevel;

    enqueueFillBuffer<FamilyType>();
    EXPECT_GT(pCmdQ->taskLevel, taskLevelBefore);
}

HWTEST_F(EnqueueFillBufferCmdTests, setsBufferCompletionStamp) {
    enqueueFillBuffer<FamilyType>();
    auto deviceEngineType = pDevice->getEngineType();
    auto &commandStreamReceiver = pDevice->getCommandStreamReceiver();
    EXPECT_EQ(commandStreamReceiver.peekTaskCount(), buffer->getCompletionStamp().taskCount);
    EXPECT_EQ(0u, buffer->getCompletionStamp().deviceOrdinal);
    EXPECT_EQ(deviceEngineType, buffer->getCompletionStamp().engineType);
}

HWTEST_F(EnqueueFillBufferCmdTests, addsCommands) {
    auto usedCmdBufferBefore = pCS->getUsed();

    enqueueFillBuffer<FamilyType>();
    EXPECT_NE(usedCmdBufferBefore, pCS->getUsed());
}

HWTEST_F(EnqueueFillBufferCmdTests, GPGPUWalker) {
    typedef typename FamilyType::GPGPU_WALKER GPGPU_WALKER;

    enqueueFillBuffer<FamilyType>();

    auto *cmd = (GPGPU_WALKER *)cmdWalker;
    ASSERT_NE(nullptr, cmd);

    // Verify GPGPU_WALKER parameters
    EXPECT_NE(0u, cmd->getThreadGroupIdXDimension());
    EXPECT_NE(0u, cmd->getThreadGroupIdYDimension());
    EXPECT_NE(0u, cmd->getThreadGroupIdZDimension());
    EXPECT_NE(0u, cmd->getRightExecutionMask());
    EXPECT_NE(0u, cmd->getBottomExecutionMask());
    EXPECT_EQ(GPGPU_WALKER::SIMD_SIZE_SIMD32, cmd->getSimdSize());
    EXPECT_NE(0u, cmd->getIndirectDataLength());
    EXPECT_FALSE(cmd->getIndirectParameterEnable());

    // Compute the SIMD lane mask
    size_t simd =
        cmd->getSimdSize() == GPGPU_WALKER::SIMD_SIZE_SIMD32 ? 32 : cmd->getSimdSize() == GPGPU_WALKER::SIMD_SIZE_SIMD16 ? 16 : 8;
    uint64_t simdMask = (1ull << simd) - 1;

    // Mask off lanes based on the execution masks
    auto laneMaskRight = cmd->getRightExecutionMask() & simdMask;
    auto lanesPerThreadX = 0;
    while (laneMaskRight) {
        lanesPerThreadX += laneMaskRight & 1;
        laneMaskRight >>= 1;
    }
}

HWTEST_F(EnqueueFillBufferCmdTests, addsIndirectData) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    auto dshBefore = pDSH->getUsed();
    auto iohBefore = pIOH->getUsed();
    auto ihBefore = pIH->getUsed();
    auto sshBefore = pSSH->getUsed();

    enqueueFillBuffer<FamilyType>();

    MultiDispatchInfo multiDispatchInfo;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {EnqueueFillBufferTraits::offset, 0, 0};
    dc.size = {EnqueueFillBufferTraits::size, 0, 0};
    builder.buildDispatchInfos(multiDispatchInfo, dc);
    EXPECT_NE(0u, multiDispatchInfo.size());

    auto kernel = multiDispatchInfo.begin()->getKernel();

    EXPECT_NE(dshBefore, pDSH->getUsed());
    EXPECT_NE(iohBefore, pIOH->getUsed());
    EXPECT_NE(ihBefore, pIH->getUsed());
    if (kernel->requiresSshForBuffers()) {
        EXPECT_NE(sshBefore, pSSH->getUsed());
    }

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, FillBufferRightLeftover) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    MultiDispatchInfo mdi;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {0, 0, 0};
    dc.size = {EnqueueFillBufferTraits::patternSize, 0, 0};
    builder.buildDispatchInfos(mdi, dc);
    EXPECT_EQ(1u, mdi.size());

    auto kernel = mdi.begin()->getKernel();
    EXPECT_STREQ("FillBufferRightLeftover", kernel->getKernelInfo().name.c_str());

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, FillBufferMiddle) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    MultiDispatchInfo mdi;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {0, 0, 0};
    dc.size = {MemoryConstants::cacheLineSize, 0, 0};
    builder.buildDispatchInfos(mdi, dc);
    EXPECT_EQ(1u, mdi.size());

    auto kernel = mdi.begin()->getKernel();
    EXPECT_STREQ("FillBufferMiddle", kernel->getKernelInfo().name.c_str());

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, FillBufferLeftLeftover) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    MultiDispatchInfo mdi;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {EnqueueFillBufferTraits::patternSize, 0, 0};
    dc.size = {EnqueueFillBufferTraits::patternSize, 0, 0};
    builder.buildDispatchInfos(mdi, dc);
    EXPECT_EQ(1u, mdi.size());

    auto kernel = mdi.begin()->getKernel();
    EXPECT_STREQ("FillBufferLeftLeftover", kernel->getKernelInfo().name.c_str());

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, LoadRegisterImmediateL3CNTLREG) {
    typedef typename FamilyType::PARSE PARSE;
    typedef typename FamilyType::MI_LOAD_REGISTER_IMM MI_LOAD_REGISTER_IMM;
    enqueueFillBuffer<FamilyType>();

    // All state should be programmed before walker
    auto itorCmd = findMmio<FamilyType>(cmdList.begin(), itorWalker, L3CNTLRegisterOffset<FamilyType>::registerOffset);
    ASSERT_NE(itorWalker, itorCmd);

    auto *cmd = genCmdCast<MI_LOAD_REGISTER_IMM *>(*itorCmd);
    ASSERT_NE(nullptr, cmd);

    auto RegisterOffset = L3CNTLRegisterOffset<FamilyType>::registerOffset;
    EXPECT_EQ(RegisterOffset, cmd->getRegisterOffset());
    auto l3Cntlreg = cmd->getDataDword();
    auto numURBWays = (l3Cntlreg >> 1) & 0x7f;
    auto L3ClientPool = (l3Cntlreg >> 25) & 0x7f;
    EXPECT_NE(0u, numURBWays);
    EXPECT_NE(0u, L3ClientPool);
}

HWTEST_F(EnqueueFillBufferCmdTests, StateBaseAddress) {
    typedef typename FamilyType::PARSE PARSE;
    typedef typename FamilyType::STATE_BASE_ADDRESS STATE_BASE_ADDRESS;
    enqueueFillBuffer<FamilyType>();

    auto *cmd = (STATE_BASE_ADDRESS *)cmdStateBaseAddress;
    ASSERT_NE(nullptr, cmd);

    // Verify all addresses are getting programmed
    EXPECT_TRUE(cmd->getDynamicStateBaseAddressModifyEnable());
    EXPECT_TRUE(cmd->getGeneralStateBaseAddressModifyEnable());
    EXPECT_TRUE(cmd->getSurfaceStateBaseAddressModifyEnable());
    EXPECT_TRUE(cmd->getIndirectObjectBaseAddressModifyEnable());
    EXPECT_TRUE(cmd->getInstructionBaseAddressModifyEnable());

    EXPECT_EQ((uintptr_t)pDSH->getCpuBase(), cmd->getDynamicStateBaseAddress());
    // Stateless accesses require GSH.base to be 0.
    EXPECT_EQ(0u, cmd->getGeneralStateBaseAddress());
    EXPECT_EQ((uintptr_t)pSSH->getCpuBase(), cmd->getSurfaceStateBaseAddress());
    EXPECT_EQ((uintptr_t)pIOH->getCpuBase(), cmd->getIndirectObjectBaseAddress());
    EXPECT_EQ((uintptr_t)pIH->getCpuBase(), cmd->getInstructionBaseAddress());

    // Verify all sizes are getting programmed
    EXPECT_TRUE(cmd->getDynamicStateBufferSizeModifyEnable());
    EXPECT_TRUE(cmd->getGeneralStateBufferSizeModifyEnable());
    EXPECT_TRUE(cmd->getIndirectObjectBufferSizeModifyEnable());
    EXPECT_TRUE(cmd->getInstructionBufferSizeModifyEnable());

    EXPECT_EQ(pDSH->getMaxAvailableSpace(), cmd->getDynamicStateBufferSize() * MemoryConstants::pageSize);
    EXPECT_NE(0u, cmd->getGeneralStateBufferSize());
    EXPECT_EQ(pIOH->getMaxAvailableSpace(), cmd->getIndirectObjectBufferSize() * MemoryConstants::pageSize);
    EXPECT_EQ(pIH->getMaxAvailableSpace(), cmd->getInstructionBufferSize() * MemoryConstants::pageSize);

    // Generically validate this command
    FamilyType::PARSE::template validateCommand<STATE_BASE_ADDRESS *>(cmdList.begin(), itorStateBaseAddress);
}

HWTEST_F(EnqueueFillBufferCmdTests, MediaInterfaceDescriptorLoad) {
    typedef typename FamilyType::MEDIA_INTERFACE_DESCRIPTOR_LOAD MEDIA_INTERFACE_DESCRIPTOR_LOAD;
    typedef typename FamilyType::INTERFACE_DESCRIPTOR_DATA INTERFACE_DESCRIPTOR_DATA;

    enqueueFillBuffer<FamilyType>();

    auto *cmd = (MEDIA_INTERFACE_DESCRIPTOR_LOAD *)cmdMediaInterfaceDescriptorLoad;

    // Verify we have a valid length -- multiple of INTERFACE_DESCRIPTOR_DATAs
    EXPECT_EQ(0u, cmd->getInterfaceDescriptorTotalLength() % sizeof(INTERFACE_DESCRIPTOR_DATA));

    // Validate the start address
    size_t alignmentStartAddress = 64 * sizeof(uint8_t);
    EXPECT_EQ(0u, cmd->getInterfaceDescriptorDataStartAddress() % alignmentStartAddress);

    // Validate the length
    EXPECT_NE(0u, cmd->getInterfaceDescriptorTotalLength());
    size_t alignmentTotalLength = 32 * sizeof(uint8_t);
    EXPECT_EQ(0u, cmd->getInterfaceDescriptorTotalLength() % alignmentTotalLength);

    // Generically validate this command
    FamilyType::PARSE::template validateCommand<MEDIA_INTERFACE_DESCRIPTOR_LOAD *>(cmdList.begin(), itorMediaInterfaceDescriptorLoad);
}

HWTEST_F(EnqueueFillBufferCmdTests, InterfaceDescriptorData) {
    typedef typename FamilyType::INTERFACE_DESCRIPTOR_DATA INTERFACE_DESCRIPTOR_DATA;
    typedef typename FamilyType::STATE_BASE_ADDRESS STATE_BASE_ADDRESS;

    enqueueFillBuffer<FamilyType>();

    // Extract the IDD
    auto cmdSBA = (STATE_BASE_ADDRESS *)cmdStateBaseAddress;
    auto &IDD = *(INTERFACE_DESCRIPTOR_DATA *)(cmdInterfaceDescriptorData);

    // Validate the kernel start pointer.  Technically, a kernel can start at address 0 but let's force a value.
    auto kernelStartPointer = ((uint64_t)IDD.getKernelStartPointerHigh() << 32) + IDD.getKernelStartPointer();
    EXPECT_LE(kernelStartPointer, cmdSBA->getInstructionBufferSize() * MemoryConstants::pageSize);

    EXPECT_NE(0u, IDD.getNumberOfThreadsInGpgpuThreadGroup());
    EXPECT_NE(0u, IDD.getCrossThreadConstantDataReadLength());
    EXPECT_NE(0u, IDD.getConstantIndirectUrbEntryReadLength());
}

HWTEST_F(EnqueueFillBufferCmdTests, PipelineSelect) {
    enqueueFillBuffer<FamilyType>();
    int numCommands = getNumberOfPipelineSelectsThatEnablePipelineSelect<FamilyType>();
    EXPECT_EQ(1, numCommands);
}

HWTEST_F(EnqueueFillBufferCmdTests, MediaVFEState) {
    typedef typename FamilyType::MEDIA_VFE_STATE MEDIA_VFE_STATE;

    enqueueFillBuffer<FamilyType>();

    auto *cmd = (MEDIA_VFE_STATE *)cmdMediaVfeState;
    ASSERT_NE(nullptr, cmd);

    // Verify we have a valid length
    EXPECT_EQ(pDevice->getHardwareInfo().pSysInfo->ThreadCount, cmd->getMaximumNumberOfThreads());
    EXPECT_NE(0u, cmd->getNumberOfUrbEntries());
    EXPECT_NE(0u, cmd->getUrbEntryAllocationSize());

    // Generically validate this command
    FamilyType::PARSE::template validateCommand<MEDIA_VFE_STATE *>(cmdList.begin(), itorMediaVfeState);
}

HWTEST_F(EnqueueFillBufferCmdTests, argumentZeroShouldMatchDestAddress) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    // Extract the kernel used
    MultiDispatchInfo multiDispatchInfo;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {EnqueueFillBufferTraits::offset, 0, 0};
    dc.size = {EnqueueFillBufferTraits::size, 0, 0};
    builder.buildDispatchInfos(multiDispatchInfo, dc);
    EXPECT_NE(0u, multiDispatchInfo.size());

    auto kernel = multiDispatchInfo.begin()->getKernel();
    ASSERT_NE(nullptr, kernel);

    // Determine where the argument is
    auto pArgument = (void **)getStatelessArgumentPointer<FamilyType>(*kernel, 0);

    EXPECT_EQ((void *)((uintptr_t)buffer->getGraphicsAllocation()->getGpuAddress()), *pArgument);

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

// This test case should be re-enabled once getStatelessArgumentPointer gets support for SVM pointers.
// This could happen if KernelInfo.kernelArgInfo was accessible given a Kernel.  Just need an offset
// into CrossThreadData.
HWTEST_F(EnqueueFillBufferCmdTests, DISABLED_argumentOneShouldMatchOffset) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    // Extract the kernel used
    MultiDispatchInfo multiDispatchInfo;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {EnqueueFillBufferTraits::offset, 0, 0};
    dc.size = {EnqueueFillBufferTraits::size, 0, 0};
    builder.buildDispatchInfos(multiDispatchInfo, dc);
    EXPECT_NE(0u, multiDispatchInfo.size());

    auto kernel = multiDispatchInfo.begin()->getKernel();
    ASSERT_NE(nullptr, kernel);

    // Determine where the argument is
    auto pArgument = (uint32_t *)getStatelessArgumentPointer<FamilyType>(*kernel, 1);
    ASSERT_NE(nullptr, pArgument);
    EXPECT_EQ(0u, *pArgument);

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, argumentTwoShouldMatchPatternPtr) {
    auto patternAllocation = context.getMemoryManager()->allocateGraphicsMemory(EnqueueFillBufferTraits::patternSize, MemoryConstants::preferredAlignment);

    enqueueFillBuffer<FamilyType>();

    // Extract the kernel used
    MultiDispatchInfo multiDispatchInfo;
    auto &builder = BuiltIns::getInstance().getBuiltinDispatchInfoBuilder(EBuiltInOps::FillBuffer,
                                                                          pCmdQ->getContext(), pCmdQ->getDevice());
    ASSERT_NE(nullptr, &builder);

    BuiltinDispatchInfoBuilder::BuiltinOpParams dc;
    MemObj patternMemObj(&this->context, 0, 0, alignUp(EnqueueFillBufferTraits::patternSize, 4), patternAllocation->getUnderlyingBuffer(),
                         patternAllocation->getUnderlyingBuffer(), patternAllocation, false, false, true);
    dc.srcMemObj = &patternMemObj;
    dc.dstMemObj = buffer;
    dc.dstOffset = {EnqueueFillBufferTraits::offset, 0, 0};
    dc.size = {EnqueueFillBufferTraits::size, 0, 0};
    builder.buildDispatchInfos(multiDispatchInfo, dc);
    EXPECT_NE(0u, multiDispatchInfo.size());

    auto kernel = multiDispatchInfo.begin()->getKernel();
    ASSERT_NE(nullptr, kernel);

    // Determine where the argument is
    auto pArgument = (void **)getStatelessArgumentPointer<FamilyType>(*kernel, 2);
    EXPECT_NE(nullptr, *pArgument);

    context.getMemoryManager()->freeGraphicsMemory(patternAllocation);
}

HWTEST_F(EnqueueFillBufferCmdTests, patternShouldBeCopied) {
    MemoryManager *mmgr = pCmdQ->getDevice().getMemoryManager();
    ASSERT_TRUE(mmgr->allocationsForReuse.peekIsEmpty());
    enqueueFillBuffer<FamilyType>();
    ASSERT_FALSE(mmgr->allocationsForReuse.peekIsEmpty());
    GraphicsAllocation *allocation = mmgr->allocationsForReuse.peekHead();

    while (allocation != nullptr) {
        if ((allocation->getUnderlyingBufferSize() >= sizeof(float)) &&
            (allocation->getUnderlyingBuffer() != nullptr) &&
            (*(static_cast<float *>(allocation->getUnderlyingBuffer())) == EnqueueFillBufferHelper<>::Traits::pattern[0]) &&
            (pCmdQ->taskCount == allocation->taskCount)) {
            break;
        }
        allocation = allocation->next;
    }

    ASSERT_NE(nullptr, allocation);
    EXPECT_NE(&EnqueueFillBufferHelper<>::Traits::pattern[0], allocation->getUnderlyingBuffer());
}

HWTEST_F(EnqueueFillBufferCmdTests, patternShouldBeAligned) {
    MemoryManager *mmgr = pCmdQ->getDevice().getMemoryManager();
    ASSERT_TRUE(mmgr->allocationsForReuse.peekIsEmpty());
    enqueueFillBuffer<FamilyType>();
    ASSERT_FALSE(mmgr->allocationsForReuse.peekIsEmpty());
    GraphicsAllocation *allocation = mmgr->allocationsForReuse.peekHead();

    while (allocation != nullptr) {
        if ((allocation->getUnderlyingBufferSize() >= sizeof(float)) &&
            (allocation->getUnderlyingBuffer() != nullptr) &&
            (*(static_cast<float *>(allocation->getUnderlyingBuffer())) == EnqueueFillBufferHelper<>::Traits::pattern[0]) &&
            (pCmdQ->taskCount == allocation->taskCount)) {
            break;
        }
        allocation = allocation->next;
    }

    ASSERT_NE(nullptr, allocation);
    EXPECT_EQ(alignUp(allocation->getUnderlyingBuffer(), MemoryConstants::cacheLineSize), allocation->getUnderlyingBuffer());
    EXPECT_EQ(alignUp(allocation->getUnderlyingBufferSize(), MemoryConstants::cacheLineSize), allocation->getUnderlyingBufferSize());
}

HWTEST_F(EnqueueFillBufferCmdTests, patternOfSizeOneByteShouldGetPreparedForMiddleKernel) {
    MemoryManager *mmgr = pCmdQ->getDevice().getMemoryManager();
    ASSERT_TRUE(mmgr->allocationsForReuse.peekIsEmpty());

    auto dstBuffer = std::unique_ptr<Buffer>(BufferHelper<>::create());
    const uint8_t pattern[1] = {0x55};
    const size_t patternSize = sizeof(pattern);
    const size_t offset = 0;
    const size_t size = 4 * patternSize;
    const uint8_t output[4] = {0x55, 0x55, 0x55, 0x55};

    auto retVal = clEnqueueFillBuffer(
        pCmdQ,
        dstBuffer.get(),
        pattern,
        patternSize,
        offset,
        size,
        0,
        nullptr,
        nullptr);
    ASSERT_EQ(CL_SUCCESS, retVal);

    ASSERT_FALSE(mmgr->allocationsForReuse.peekIsEmpty());

    GraphicsAllocation *allocation = mmgr->allocationsForReuse.peekHead();
    ASSERT_NE(nullptr, allocation);

    EXPECT_EQ(0, memcmp(allocation->getUnderlyingBuffer(), output, size));
}

HWTEST_F(EnqueueFillBufferCmdTests, patternOfSizeTwoBytesShouldGetPreparedForMiddleKernel) {
    MemoryManager *mmgr = pCmdQ->getDevice().getMemoryManager();
    ASSERT_TRUE(mmgr->allocationsForReuse.peekIsEmpty());

    auto dstBuffer = std::unique_ptr<Buffer>(BufferHelper<>::create());
    const uint8_t pattern[2] = {0x55, 0xAA};
    const size_t patternSize = sizeof(pattern);
    const size_t offset = 0;
    const size_t size = 2 * patternSize;
    const uint8_t output[4] = {0x55, 0xAA, 0x55, 0xAA};

    auto retVal = clEnqueueFillBuffer(
        pCmdQ,
        dstBuffer.get(),
        pattern,
        patternSize,
        offset,
        size,
        0,
        nullptr,
        nullptr);
    ASSERT_EQ(CL_SUCCESS, retVal);

    ASSERT_FALSE(mmgr->allocationsForReuse.peekIsEmpty());

    GraphicsAllocation *allocation = mmgr->allocationsForReuse.peekHead();
    ASSERT_NE(nullptr, allocation);

    EXPECT_EQ(0, memcmp(allocation->getUnderlyingBuffer(), output, size));
}

HWTEST_F(EnqueueFillBufferCmdTests, givenEnqueueFillBufferWhenPatternAllocationIsObtainedThenItsTypeShouldBeSetToFillPattern) {
    MemoryManager *mmgr = pCmdQ->getDevice().getMemoryManager();
    ASSERT_TRUE(mmgr->allocationsForReuse.peekIsEmpty());

    auto dstBuffer = std::unique_ptr<Buffer>(BufferHelper<>::create());
    const uint8_t pattern[1] = {0x55};
    const size_t patternSize = sizeof(pattern);
    const size_t offset = 0;
    const size_t size = patternSize;

    auto retVal = clEnqueueFillBuffer(
        pCmdQ,
        dstBuffer.get(),
        pattern,
        patternSize,
        offset,
        size,
        0,
        nullptr,
        nullptr);
    ASSERT_EQ(CL_SUCCESS, retVal);

    ASSERT_FALSE(mmgr->allocationsForReuse.peekIsEmpty());

    GraphicsAllocation *patternAllocation = mmgr->allocationsForReuse.peekHead();
    ASSERT_NE(nullptr, patternAllocation);

    EXPECT_EQ(GraphicsAllocation::ALLOCATION_TYPE_FILL_PATTERN, patternAllocation->getAllocationType());
}
