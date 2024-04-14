#pragma once

#include <mutex>
#include <d3d12.h>

#include "ComPtr.hpp"

namespace d3d12 {
struct TextureContext;

struct CommandContext {
    CommandContext() = delete;
    CommandContext(ID3D12Device* device, const wchar_t* name = L"REFD2D CommandContext object");
    virtual ~CommandContext() { this->reset(); }

    void reset();
    void wait(uint32_t ms);
    void execute(ID3D12CommandQueue* queue);

    ComPtr<ID3D12CommandAllocator> cmd_allocator{};
    ComPtr<ID3D12GraphicsCommandList> cmd_list{};
    ComPtr<ID3D12Fence> fence{};
    UINT64 fence_value{};
    HANDLE fence_event{};

    std::recursive_mutex mtx{};

    bool waiting_for_fence{false};
    bool has_commands{false};
    bool is_setup{false};

    std::wstring internal_name{L"REFD2D CommandContext object"};
};
}