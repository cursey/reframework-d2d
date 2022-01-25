#include <memory>
#include <functional>
#include <chrono>

#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include "D2DRenderer.hpp"

class D3D12Renderer {
public:
    enum class RTV : int {
        BACKBUFFER_0,
        BACKBUFFER_1,
        BACKBUFFER_2,
        BACKBUFFER_3,
        D2D,
        COUNT,
    };

    enum class SRV : int { D2D };

    D3D12Renderer(IDXGISwapChain* swapchain_, ID3D12Device* device_, ID3D12CommandQueue* cmd_queue_);

    void render(std::function<void(D2DRenderer&)> draw_fn);

    auto& get_d2d() { return m_d2d; }

private:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    using Clock = std::chrono::high_resolution_clock;

    struct Vert {
        float x, y;
        float u, v;
        unsigned int color;
    };

    ComPtr<IDXGISwapChain3> m_swapchain{};
    ComPtr<ID3D12Device> m_device{};
    ComPtr<ID3D12CommandQueue> m_cmd_queue{};

    ComPtr<ID3D11Device> m_d3d11_device{};
    ComPtr<ID3D11DeviceContext> m_d3d11_context{};
    ComPtr<ID3D11On12Device> m_d3d11on12_device{};
    ComPtr<ID3D11Resource> m_wrapped_rt{};

    ComPtr<ID3D12CommandAllocator> m_cmd_allocator{};
    ComPtr<ID3D12GraphicsCommandList> m_cmd_list{};

    ComPtr<ID3D12DescriptorHeap> m_rtv_heap{};
    ComPtr<ID3D12DescriptorHeap> m_srv_heap{};
    ComPtr<ID3D12Resource> m_rts[(int)RTV::COUNT]{};

    ComPtr<ID3D12RootSignature> m_root_signature{};
    ComPtr<ID3D12PipelineState> m_pipeline_state{};
    ComPtr<ID3D12Resource> m_vert_buffer{};

    int m_width{};
    int m_height{};

    std::unique_ptr<D2DRenderer> m_d2d{};
    Clock::time_point m_d2d_next_frame_time{Clock::now()};
    const std::chrono::duration<double> D2D_UPDATE_INTERVAL{1.0 / 30.0};
    const std::chrono::milliseconds D2D_UPDATE_INTERVAL_MS{std::chrono::duration_cast<std::chrono::milliseconds>(D2D_UPDATE_INTERVAL)};

    auto& get_rt(RTV rtv) { return m_rts[(int)rtv]; }

    D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_rtv(RTV rtv) {
        return {m_rtv_heap->GetCPUDescriptorHandleForHeapStart().ptr +
                (int)rtv * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)};
    }

    D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_srv(SRV srv) {
        return {m_srv_heap->GetCPUDescriptorHandleForHeapStart().ptr +
                (int)srv * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)};
    }

    D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_srv(SRV srv) {
        return {m_srv_heap->GetGPUDescriptorHandleForHeapStart().ptr +
                (int)srv * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)};
    }
};