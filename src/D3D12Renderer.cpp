#include <stdexcept>

#include <d3d11on12.h>
#include <d3dcompiler.h>

#include "D3D12Shaders.hpp"

#include "D3D12Renderer.hpp"

D3D12Renderer::D3D12Renderer(IDXGISwapChain* swapchain_, ID3D12Device* device_, ID3D12CommandQueue* cmd_queue_)
    : m_swapchain{(IDXGISwapChain3*)swapchain_}
    , m_device{device_}
    , m_cmd_queue{cmd_queue_} {

    if (FAILED(D3D11On12CreateDevice(m_device.Get(), D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, (IUnknown**)m_cmd_queue.GetAddressOf(),
            1, 0, &m_d3d11_device, &m_d3d11_context, nullptr))) {
        throw std::runtime_error{"Failed to create D3D11On12 device"};
    }

    if (FAILED(m_d3d11_device.As(&m_d3d11on12_device))) {
        throw std::runtime_error{"Failed to query D3D11On12 device"};
    }

    // Create command allocator
    if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmd_allocator)))) {
        throw std::runtime_error{"Failed to create command allocator"};
    }

    // Create command list
    if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&m_cmd_list)))) {
        throw std::runtime_error{"Failed to create command list"};
    }

    if (FAILED(m_cmd_list->Close())) {
        throw std::runtime_error{"Failed to close command list"};
    }

    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtv_desc = {};
    rtv_desc.NumDescriptors = (int)RTV::COUNT;
    rtv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(&m_rtv_heap)))) {
        throw std::runtime_error{"Failed to create RTV descriptor heap"};
    }

    // Create SRV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC srv_desc = {};
    srv_desc.NumDescriptors = 1;
    srv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(m_device->CreateDescriptorHeap(&srv_desc, IID_PPV_ARGS(&m_srv_heap)))) {
        throw std::runtime_error{"Failed to create SRV descriptor heap"};
    }

    // Create back buffer RTVs
    DXGI_SWAP_CHAIN_DESC swapchain_desc{};

    if (FAILED(m_swapchain->GetDesc(&swapchain_desc))) {
        throw std::runtime_error{"Failed to get swapchain description"};
    }

    for (int i = 0; i < swapchain_desc.BufferCount; i++) {
        if (SUCCEEDED(m_swapchain->GetBuffer((UINT)i, IID_PPV_ARGS(&m_rts[i])))) {
            m_device->CreateRenderTargetView(m_rts[i].Get(), nullptr, get_cpu_rtv((RTV)i));
        }
    }

    // Create D2D render target
    auto& backbuffer = get_rt(RTV::BACKBUFFER_0);
    auto backbuffer_desc = backbuffer->GetDesc();

    m_width = backbuffer_desc.Width;
    m_height = backbuffer_desc.Height;

    D3D12_HEAP_PROPERTIES d2d_heap_props = {};
    d2d_heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
    d2d_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d2d_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clear_value{};
    clear_value.Format = backbuffer_desc.Format;

    if (FAILED(m_device->CreateCommittedResource(&d2d_heap_props, D3D12_HEAP_FLAG_NONE, &backbuffer_desc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clear_value, IID_PPV_ARGS(&m_rts[(int)RTV::D2D])))) {
        throw std::runtime_error{"Failed to create D2D render target"};
    }

    m_device->CreateRenderTargetView(m_rts[(int)RTV::D2D].Get(), nullptr, get_cpu_rtv(RTV::D2D));
    m_device->CreateShaderResourceView(m_rts[(int)RTV::D2D].Get(), nullptr, get_cpu_srv(SRV::D2D));

    D3D11_RESOURCE_FLAGS res_flags{D3D11_BIND_RENDER_TARGET};
    if (FAILED(m_d3d11on12_device->CreateWrappedResource(get_rt(RTV::D2D).Get(), &res_flags, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, IID_PPV_ARGS(&m_wrapped_rt)))) {
        throw std::runtime_error{"Failed to create wrapped render target"};
    }

    ComPtr<IDXGISurface> dxgi_surface{};

    if (FAILED(m_wrapped_rt.As(&dxgi_surface))) {
        throw std::runtime_error{"Failed to query DXGI surface"};
    }

    m_d2d = std::make_unique<D2DPainter>(m_d3d11_device.Get(), dxgi_surface.Get());

    // Create root signature.
    D3D12_DESCRIPTOR_RANGE desc_range{};
    desc_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    desc_range.NumDescriptors = 1;
    desc_range.BaseShaderRegister = 0;
    desc_range.RegisterSpace = 0;
    desc_range.OffsetInDescriptorsFromTableStart = 0;

    D3D12_ROOT_PARAMETER root_params[2]{};
    root_params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    root_params[0].Constants.ShaderRegister = 0;
    root_params[0].Constants.RegisterSpace = 0;
    root_params[0].Constants.Num32BitValues = 16;
    root_params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    root_params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root_params[1].DescriptorTable.NumDescriptorRanges = 1;
    root_params[1].DescriptorTable.pDescriptorRanges = &desc_range;
    root_params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc.MipLODBias = 0.0f;
    sampler_desc.MaxAnisotropy = 0;
    sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler_desc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler_desc.MinLOD = 0.0f;
    sampler_desc.MaxLOD = 0.0f;
    sampler_desc.ShaderRegister = 0;
    sampler_desc.RegisterSpace = 0;

    D3D12_ROOT_SIGNATURE_DESC sig_desc{};
    sig_desc.NumParameters = 2;
    sig_desc.pParameters = root_params;
    sig_desc.NumStaticSamplers = 1;
    sig_desc.pStaticSamplers = &sampler_desc;
    sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                     D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    ComPtr<ID3DBlob> blob{};
    if (FAILED(D3D12SerializeRootSignature(&sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr))) {
        throw std::runtime_error{"Failed to serialize root signature"};
    }

    if (FAILED(m_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)))) {
        throw std::runtime_error{"Failed to create root signature"};
    }

    // Create pipeline state object.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
    pso_desc.NodeMask = 1;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.pRootSignature = m_root_signature.Get();
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = backbuffer_desc.Format;
    pso_desc.SampleDesc.Count = 1;
    pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ComPtr<ID3DBlob> vertshader_blob{};
    ComPtr<ID3DBlob> pixshader_blob{};

    if (FAILED(D3DCompile(
            D3D12_VERT_SHADER, strlen(D3D12_VERT_SHADER), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertshader_blob, nullptr))) {
        throw std::runtime_error{"Failed to compile vertex shader"};
    }

    if (FAILED(D3DCompile(
            D3D12_PIX_SHADER, strlen(D3D12_PIX_SHADER), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixshader_blob, nullptr))) {
        throw std::runtime_error{"Failed to compile pixel shader"};
    }

    pso_desc.VS = {vertshader_blob->GetBufferPointer(), vertshader_blob->GetBufferSize()};
    pso_desc.PS = {pixshader_blob->GetBufferPointer(), pixshader_blob->GetBufferSize()};

    static D3D12_INPUT_ELEMENT_DESC input_layout[]{
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    pso_desc.InputLayout = {input_layout, 3};

    auto& blend = pso_desc.BlendState;
    blend.AlphaToCoverageEnable = false;
    blend.RenderTarget[0].BlendEnable = true;
    blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    auto& raster = pso_desc.RasterizerState;
    raster.FillMode = D3D12_FILL_MODE_SOLID;
    raster.CullMode = D3D12_CULL_MODE_NONE;
    raster.FrontCounterClockwise = false;
    raster.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    raster.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    raster.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    raster.DepthClipEnable = true;
    raster.MultisampleEnable = false;
    raster.AntialiasedLineEnable = false;
    raster.ForcedSampleCount = 0;
    raster.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    auto& depth = pso_desc.DepthStencilState;
    depth.DepthEnable = false;
    depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depth.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depth.StencilEnable = false;
    depth.FrontFace.StencilFailOp = depth.FrontFace.StencilDepthFailOp = depth.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depth.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depth.BackFace = depth.FrontFace;

    if (FAILED(m_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pipeline_state)))) {
        throw std::runtime_error{"Failed to create pipeline state"};
    }

    D3D12_HEAP_PROPERTIES vertbuf_heap_props{};
    vertbuf_heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
    vertbuf_heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    vertbuf_heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC vertbuf_desc{};
    vertbuf_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertbuf_desc.Width = sizeof(Vert) * 6;
    vertbuf_desc.Height = 1;
    vertbuf_desc.DepthOrArraySize = 1;
    vertbuf_desc.MipLevels = 1;
    vertbuf_desc.Format = DXGI_FORMAT_UNKNOWN;
    vertbuf_desc.SampleDesc.Count = 1;
    vertbuf_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertbuf_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(m_device->CreateCommittedResource(&vertbuf_heap_props, D3D12_HEAP_FLAG_NONE, &vertbuf_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vert_buffer)))) {
        throw std::runtime_error{"Failed to create vertex buffer"};
    }

    // Upload the verticies.
    D3D12_RANGE range{};
    Vert* verts{};

    if (FAILED(m_vert_buffer->Map(0, &range, (void**)&verts))) {
        throw std::runtime_error{"Failed to map vertex buffer"};
    }

    auto w = (float)m_width;
    auto h = (float)m_height;

    // First triangle (top-left of screen).
    verts[0] = {0.0f, 0.0f, 0.0f, 0.0f, 0xFFFFFFFF};
    verts[1] = {w, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF};
    verts[2] = {0.0f, h, 0.0f, 1.0f, 0xFFFFFFFF};

    // Second triangle (bottom-right of screen).
    verts[3] = {w, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF};
    verts[4] = {w, h, 1.0f, 1.0f, 0xFFFFFFFF};
    verts[5] = {0.0f, h, 0.0f, 1.0f, 0xFFFFFFFF};

    m_vert_buffer->Unmap(0, &range);
}

void D3D12Renderer::render(std::function<void(D2DPainter&)> draw_fn) {
    m_cmd_allocator->Reset();
    m_cmd_list->Reset(m_cmd_allocator.Get(), nullptr);

    auto now = Clock::now();

    if (now >= m_d2d_next_frame_time) {
        m_d3d11on12_device->AcquireWrappedResources(m_wrapped_rt.GetAddressOf(), 1);
        m_d2d->begin();
        draw_fn(*m_d2d);
        m_d2d->end();
        m_d3d11on12_device->ReleaseWrappedResources(m_wrapped_rt.GetAddressOf(), 1);
        m_d3d11_context->Flush();
        m_d2d_next_frame_time = now + D2D_UPDATE_INTERVAL_MS;
    }

    auto L = 0.0f;
    auto R = (float)m_width;
    auto T = 0.0f;
    auto B = (float)m_height;
    float mvp[4][4]{
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };

    D3D12_VIEWPORT vp{};
    vp.Width = m_width;
    vp.Height = m_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;
    m_cmd_list->RSSetViewports(1, &vp);

    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = m_vert_buffer->GetGPUVirtualAddress();
    vbv.SizeInBytes = sizeof(Vert) * 6;
    vbv.StrideInBytes = sizeof(Vert);
    m_cmd_list->IASetVertexBuffers(0, 1, &vbv);
    m_cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_cmd_list->SetPipelineState(m_pipeline_state.Get());
    m_cmd_list->SetGraphicsRootSignature(m_root_signature.Get());
    m_cmd_list->SetGraphicsRoot32BitConstants(0, 16, mvp, 0);

    const float blend_factor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    m_cmd_list->OMSetBlendFactor(blend_factor);

    // Draw to the back buffer.
    auto bb_index = m_swapchain->GetCurrentBackBufferIndex();
    D3D12_RESOURCE_BARRIER barrier{};
    D3D12_CPU_DESCRIPTOR_HANDLE rts[1]{};

    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_rts[bb_index].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_cmd_list->ResourceBarrier(1, &barrier);
    rts[0] = get_cpu_rtv((RTV)bb_index);
    m_cmd_list->OMSetRenderTargets(1, rts, FALSE, NULL);
    m_cmd_list->SetDescriptorHeaps(1, m_srv_heap.GetAddressOf());

    // draw.
    m_cmd_list->SetGraphicsRootDescriptorTable(1, get_gpu_srv(SRV::D2D));
    m_cmd_list->DrawInstanced(6, 1, 0, 0);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_cmd_list->ResourceBarrier(1, &barrier);
    m_cmd_list->Close();

    m_cmd_queue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_cmd_list.GetAddressOf());
}