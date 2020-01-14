////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 12 video decoding helper
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pch.hpp"
#include <dxgtaef.h>
#include <DxgTaefDeviceManager.h>
#include <util.h>
#include "TimingDataShaders.h"
#include "PsGouraudShader.h"
#include "VsGouraudShader.h"
#include "simpleRenderer.h"

const UINT c_Width = 2048;
const UINT c_Height = 2048;

void SimpleRenderer::Initialize(ID3D12Device** ppDevice, ID3D12CommandQueue** ppCommandQueue, ID3D12CommandList** ppCommandList)
{
    m_RenderOutput.Initialize(&m_Device, &m_CommandQueue, 1);

    InitializeDefaults();
    CreateDefaultPipelineState();

    m_DescriptorHeap->GetHeapHandles(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0, &m_RenderOutput1.m_RenderTarget);
    VERIFY_SUCCEEDED(D3D12Helper::CreateRenderTarget(m_Device,
        m_CommandQueue,
        NULL,
        &m_RenderOutput1.m_BackBuffer,
        m_RenderOutput1.m_RenderTarget,
        1,
        1,
        &m_RTVBarrier,
        &m_PresentBarrier,
        DXGI_FORMAT_B8G8R8A8_UNORM));

    CD3DX12_ROOT_PARAMETER Parameters[2];
    CD3DX12_DESCRIPTOR_RANGE SRVRange;
    SRVRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    CD3DX12_DESCRIPTOR_RANGE SamplerRange;
    SamplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
    Parameters[0].InitAsDescriptorTable(1, &SRVRange);
    Parameters[1].InitAsDescriptorTable(1, &SamplerRange);

    CreateRootSignature(m_Device, Parameters, ARRAYSIZE(Parameters), &m_RootSignature, &m_Blob);

    const D3D12_INPUT_ELEMENT_DESC InputDescriptor[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = m_DefaultPSODescriptor;
    PsoDesc.SampleMask = UINT_MAX;
    PsoDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
    PsoDesc.SampleDesc.Count = 1;
    PsoDesc.NumRenderTargets = 1;
    PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    PsoDesc.DepthStencilState.DepthEnable = false;
    PsoDesc.pRootSignature = m_RootSignature;
    PsoDesc.InputLayout.pInputElementDescs = InputDescriptor;
    PsoDesc.InputLayout.NumElements = _countof(InputDescriptor);

    ID3D12_SHADER Shaders[] = {
        { D3D12_SHADER_STAGE::D3D12_SHADER_STAGE_VERTEX, g_vs_TimingData, sizeof(g_vs_TimingData) },
        { D3D12_SHADER_STAGE::D3D12_SHADER_STAGE_PIXEL, g_ps_EndOfPipeline, sizeof(g_ps_EndOfPipeline) },
    };
    D3D12Helper::CreatePipelineState(
        m_Device,
        Shaders,
        _countof(Shaders),
        &PsoDesc,
        &m_PipelineState.p);
    // Setup Sampler
    D3D12_SAMPLER_DESC SampDesc;
    ZeroMemory(&SampDesc, sizeof(SampDesc));
    SampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    SampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    SampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    SampDesc.MinLOD = 0;
    SampDesc.MaxLOD = D3D12_FLOAT32_MAX;

    m_SamplerSlot = m_DescriptorHeap->CreateSampler(&SampDesc);

    // Setup SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC SrvDescriptor;
    RtlZeroMemory(&SrvDescriptor, sizeof(SrvDescriptor));
    SrvDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SrvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SrvDescriptor.Texture2D.MipLevels = 1;
    SrvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    D3D12_RESOURCE_DESC SrvResourceDescriptor;
    RtlZeroMemory(&SrvResourceDescriptor, sizeof(SrvResourceDescriptor));
    SrvResourceDescriptor.Width = 1;
    SrvResourceDescriptor.Height = 1;
    SrvResourceDescriptor.DepthOrArraySize = 1;
    SrvResourceDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SrvResourceDescriptor.MipLevels = 1;
    SrvResourceDescriptor.SampleDesc.Count = 1;
    SrvResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    SrvResourceDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    vector<UINT32> sourceData(c_Width * c_Height);
    for (UINT i = 0; i < c_Width * c_Height; ++i)
    {
        sourceData[i] = (rand() % 0xffffffff) | 0x000000ff;
    }

    m_SrvSlot = m_DescriptorHeap->CreateShaderResource(
        &SrvDescriptor,
        &sourceData.front(),
        sizeof(UINT32) * c_Width * c_Height,
        &m_SrvResource,
        &SrvResourceDescriptor);

    // c_Width x c_Height RT pass
    m_DescriptorHeap->GetHeapHandles(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 0, &m_RenderOutput2.m_RenderTarget);
    VERIFY_SUCCEEDED(D3D12Helper::CreateRenderTarget(m_Device,
        m_CommandQueue,
        NULL,
        &m_RenderOutput2.m_BackBuffer,
        m_RenderOutput2.m_RenderTarget,
        c_Width,
        c_Height,
        &m_LRTVBarrier,
        &m_LPresentBarrier,
        DXGI_FORMAT_B8G8R8A8_UNORM));

    // Create query heap & result buffer for tracking gpu duration
    D3D12_QUERY_HEAP_DESC queryHeapDesc = {};
    queryHeapDesc.Count = 4;
    queryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;

    VERIFY_SUCCEEDED(m_Device->CreateQueryHeap(&queryHeapDesc, IID_PPV_ARGS(&m_QueryHeap)));
    VERIFY_SUCCEEDED(m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint64_t) * queryHeapDesc.Count),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_QueryResultsBuffer)
    ));

    *ppDevice = m_Device;
    *ppCommandQueue = m_CommandQueue;
    *ppCommandList = m_CommandList;
}


void SimpleRenderer::Execute(ID3D12TrackedWorkload* pTrackedWorkload, const D3D12_TRACKED_WORKLOAD_DEADLINE& deadline, bool keepTiming)
{
    UNREFERENCED_PARAMETER(pTrackedWorkload);
    UNREFERENCED_PARAMETER(deadline);

    //
    // 1x1 RT pass
    //
    {
        m_DescriptorHeap->SetHeap(m_CommandList);
        if (keepTiming)
        {
            m_CommandList->EndQuery(m_QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
        }

        m_CommandList->SetPipelineState(m_PipelineState);
        UINT VertexSlot = m_DescriptorHeap->UploadVertices((PVOID)quad, sizeof(float) * 6, 4);
        const D3D12_VERTEX_BUFFER_VIEW* VertexHandle = m_DescriptorHeap->GetVBV(VertexSlot);
        m_CommandList->IASetVertexBuffers(0, 1, VertexHandle);
        m_CommandList->SetGraphicsRootSignature(m_RootSignature);
        m_DescriptorHeap->SetHeap(m_CommandList);
        m_DescriptorHeap->SetDescriptorTableBase(m_CommandList, 0, m_SrvSlot);
        m_DescriptorHeap->SetSamplerTableBase(m_CommandList, 1, m_SamplerSlot);
        D3D12Helper::SetRenderTarget(m_CommandList, &m_RenderOutput1.m_RenderTarget);
        D3D12Helper::SetViewportAndScissor(m_CommandList, 1, 1);
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        m_CommandList->DrawInstanced(4, 1, 0, 0);

        if (keepTiming)
        {
            m_CommandList->EndQuery(m_QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);
        }
        m_CommandList->Close();
        m_CommandQueue->ExecuteCommandLists(1, CommandListCast(&m_CommandList.p));
        m_RenderOutput1.Flush(m_CommandList);
        m_CommandList->Reset(m_DCLCommandAllocator, m_PipelineState);
    }

    //
    // c_Width x c_Height RT pass
    //
    {
        if (keepTiming)
        {
            m_CommandList->EndQuery(m_QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 2);
        }

        for (auto i = 0; i < m_repetitions; i++)
        {
            D3D12Helper::SetRenderTarget(m_CommandList, &m_RenderOutput2.m_RenderTarget);
            D3D12Helper::SetViewportAndScissor(m_CommandList, c_Width, c_Height);
            UINT VertexSlot = m_DescriptorHeap->UploadVertices((PVOID)quad, sizeof(float) * 6, 4);
            const D3D12_VERTEX_BUFFER_VIEW* VertexHandle = m_DescriptorHeap->GetVBV(VertexSlot);
            m_CommandList->IASetVertexBuffers(0, 1, VertexHandle);
            m_CommandList->SetGraphicsRootSignature(m_RootSignature);
            m_DescriptorHeap->SetHeap(m_CommandList);
            m_DescriptorHeap->SetDescriptorTableBase(m_CommandList, 0, m_SrvSlot);
            m_DescriptorHeap->SetSamplerTableBase(m_CommandList, 1, m_SamplerSlot);
            m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            m_CommandList->DrawInstanced(4, 1, 0, 0);
        }
        if (keepTiming)
        {
            m_CommandList->EndQuery(m_QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 3);
            m_CommandList->ResolveQueryData(m_QueryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 4, m_QueryResultsBuffer, 0);
        }
        m_CommandList->Close();
        m_CommandQueue->ExecuteCommandLists(1, CommandListCast(&m_CommandList.p));
        m_RenderOutput2.Flush(m_CommandList);
    }
}

void SimpleRenderer::CreateWorkloadWithSpecifiedDuration(_In_ float requestedDurationInMs, _Out_opt_ float* pActualDurationInMs)
{
    D3D12_TRACKED_WORKLOAD_DEADLINE deadline = {};
    float actualDurationInMs;

    m_repetitions = 0;
    do
    {
        ++m_repetitions;
        float totalDuration = 0;
        const UINT iterations = 10;
        for (auto i = 0; i < iterations; i++)
        {
            Execute(nullptr, deadline, true);
            WaitForIdle(m_CommandQueue);

            UINT64* pGpuTimestamps = nullptr;
            VERIFY_SUCCEEDED(m_QueryResultsBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pGpuTimestamps)));

            UINT64 gpuExecutionFrequency = 0;
            VERIFY_SUCCEEDED(m_CommandQueue->GetTimestampFrequency(&gpuExecutionFrequency));

            VERIFY_IS_TRUE(pGpuTimestamps[1] > pGpuTimestamps[0]);
            VERIFY_IS_TRUE(pGpuTimestamps[2] > pGpuTimestamps[1]);
            VERIFY_IS_TRUE(pGpuTimestamps[3] > pGpuTimestamps[2]);
            const UINT64 gpuTimestampDelta1 = pGpuTimestamps[1] - pGpuTimestamps[0];
            const UINT64 gpuTimestampDelta2 = pGpuTimestamps[3] - pGpuTimestamps[2];
            const float gpuExecutionDuration1 = static_cast<float>(gpuTimestampDelta1) / static_cast<float>(gpuExecutionFrequency);
            const float gpuExecutionDuration2 = static_cast<float>(gpuTimestampDelta2) / static_cast<float>(gpuExecutionFrequency);

            UINT64 calibrationCpu = 0;
            UINT64 calibrationGpu = 0;
            VERIFY_SUCCEEDED(m_CommandQueue->GetClockCalibration(&calibrationGpu, &calibrationCpu));
            VERIFY_IS_TRUE(calibrationCpu != 0 && calibrationGpu != 0);

            const float gpuExecutionStartTimeToCalibrateTime =
                static_cast<float>(calibrationGpu - pGpuTimestamps[0]) / static_cast<float>(gpuExecutionFrequency);

            m_QueryResultsBuffer->Unmap(0, nullptr);

            // Sanity checks.
            {
                VERIFY_IS_TRUE(gpuExecutionStartTimeToCalibrateTime > 0.0f);

                // Not always verifying timeFromCpuExecuteToGpuStartInMs > 0, as when there is
                // no work to preempt it is effectively a race between the CPU and GPU.
            }

            LOG_COMMENT(L"*************************** took %dms %dms", (DWORD)(gpuExecutionDuration1 * 1000.0f), (DWORD)(gpuExecutionDuration2 * 1000.0f));
            totalDuration += gpuExecutionDuration1 + gpuExecutionDuration2;
        }
        actualDurationInMs = (totalDuration * 1000.0f) / iterations;
    } while (actualDurationInMs < requestedDurationInMs);

    *pActualDurationInMs = actualDurationInMs;
}
