#include "D3D12CommandContext.hpp"

namespace d3d12 {
CommandContext::CommandContext(ID3D12Device* device, const wchar_t* name) {
    std::scoped_lock _{m_mtx};

    m_cmd_allocator.Reset();
    m_cmd_list.Reset();
    m_fence.Reset();

    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmd_allocator)))) {
        throw std::runtime_error("Failed to create command allocator");
    }

    m_cmd_allocator->SetName(name);

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&m_cmd_list)))) {
        throw std::runtime_error("Failed to create command list");
    }

    m_cmd_list->SetName(name);

    if (FAILED(device->CreateFence(m_fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)))) {
        throw std::runtime_error("Failed to create fence");
    }

    m_fence->SetName(name);
    m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    m_is_setup = true;
}

void CommandContext::reset() {
    std::scoped_lock _{m_mtx};
    wait(2000);

    m_cmd_allocator.Reset();
    m_cmd_list.Reset();
    m_fence.Reset();
    m_fence_value = 0;

    CloseHandle(m_fence_event);

    m_fence_event = nullptr;
    m_waiting_for_fence = false;
}

void CommandContext::wait(uint32_t ms) {
    std::scoped_lock _{m_mtx};

    if (m_fence_event && m_waiting_for_fence) {
        WaitForSingleObject(m_fence_event, ms);
        ResetEvent(m_fence_event);

        m_waiting_for_fence = false;

        if (FAILED(m_cmd_allocator->Reset())) {
            throw std::runtime_error("Failed to reset command allocator");
        }

        if (FAILED(m_cmd_list->Reset(m_cmd_allocator.Get(), nullptr))) {
            throw std::runtime_error("Failed to reset command list");
        }

        m_has_commands = false;
    }
}

void CommandContext::execute(ID3D12CommandQueue* command_queue) {
    std::scoped_lock _{m_mtx};

    if (m_has_commands) {
        if (FAILED(m_cmd_list->Close())) {
            throw std::runtime_error("Failed to close command list");
        }

        ID3D12CommandList* const cmd_lists[] = {m_cmd_list.Get()};

        command_queue->ExecuteCommandLists(1, cmd_lists);
        command_queue->Signal(m_fence.Get(), ++m_fence_value);
        m_fence->SetEventOnCompletion(m_fence_value, m_fence_event);

        m_waiting_for_fence = true;
        m_has_commands = false;
    }
}
} // namespace d3d12