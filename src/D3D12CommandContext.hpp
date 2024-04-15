#pragma once

#include <d3d12.h>
#include <mutex>
#include <wrl.h>

class D3D12CommandContext {
public:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    D3D12CommandContext() = delete;
    D3D12CommandContext(ID3D12Device* device, const wchar_t* name = L"REFD2D D3D12CommandContext object");
    virtual ~D3D12CommandContext() { reset(); }

    void reset();
    void wait(uint32_t ms);
    ComPtr<ID3D12GraphicsCommandList>& begin();
    void end(ID3D12CommandQueue* queue);

    [[nodiscard]] bool is_setup() const { return m_is_setup; }

private:
    ComPtr<ID3D12CommandAllocator> m_cmd_allocator{};
    ComPtr<ID3D12GraphicsCommandList> m_cmd_list{};
    ComPtr<ID3D12Fence> m_fence{};
    UINT64 m_fence_value{};
    HANDLE m_fence_event{};

    std::recursive_mutex m_mtx{};

    bool m_waiting_for_fence{};
    bool m_is_setup{};
};