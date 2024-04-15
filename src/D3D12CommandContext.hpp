#pragma once

#include <d3d12.h>
#include <mutex>
#include <wrl.h>

namespace d3d12 {
struct TextureContext;

class CommandContext {
public:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    CommandContext() = delete;
    CommandContext(ID3D12Device* device, const wchar_t* name = L"REFD2D CommandContext object");
    virtual ~CommandContext() { reset(); }

    void reset();
    void wait(uint32_t ms);
    void execute(ID3D12CommandQueue* queue);

    void begin() { m_has_commands = true; }

    [[nodiscard]] bool is_setup() const { return m_is_setup; }
    [[nodiscard]] ComPtr<ID3D12GraphicsCommandList>& cmd_list() { return m_cmd_list; }

private:
    ComPtr<ID3D12CommandAllocator> m_cmd_allocator{};
    ComPtr<ID3D12GraphicsCommandList> m_cmd_list{};
    ComPtr<ID3D12Fence> m_fence{};
    UINT64 m_fence_value{};
    HANDLE m_fence_event{};

    std::recursive_mutex m_mtx{};

    bool m_waiting_for_fence{};
    bool m_has_commands{};
    bool m_is_setup{};
};
} // namespace d3d12