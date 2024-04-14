#include <chrono>
#include <functional>
#include <memory>

#include <d3d11.h>
#include <d3d11on12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

#include "d3d12/CommandContext.hpp"

#include "D2DPainter.hpp"

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
    virtual ~D3D12Renderer() {
        // Give the command contexts priority cleanup before everything else
        // so any command lists can finish executing before destroying everything
        for (auto& cmd_context : m_cmd_contexts) {
            cmd_context->reset();
        }

        m_cmd_contexts.clear();
    }

    void render(std::function<void(D2DPainter&)> draw_fn, bool update_d2d);

    auto& get_d2d() { return m_d2d; }

private:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

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

    ComPtr<ID3D12DescriptorHeap> m_rtv_heap{};
    ComPtr<ID3D12DescriptorHeap> m_srv_heap{};
    ComPtr<ID3D12Resource> m_rts[(int)RTV::COUNT]{};

    ComPtr<ID3D12RootSignature> m_root_signature{};
    ComPtr<ID3D12PipelineState> m_pipeline_state{};

    struct RenderResources {
        ComPtr<ID3D12Resource> vert_buffer{};
    };

    uint32_t m_frames_in_flight{1};
    std::vector<std::unique_ptr<d3d12::CommandContext>> m_cmd_contexts{};
    std::vector<std::unique_ptr<RenderResources>> m_render_resources{};

    int m_width{};
    int m_height{};

    std::unique_ptr<D2DPainter> m_d2d{};

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