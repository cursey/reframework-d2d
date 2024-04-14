#include "CommandContext.hpp"

namespace d3d12 {
CommandContext::CommandContext(ID3D12Device* device, const wchar_t* name) {
    std::scoped_lock _{this->mtx};

    this->internal_name = name;

    this->cmd_allocator.Reset();
    this->cmd_list.Reset();
    this->fence.Reset();

    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->cmd_allocator)))) {
        throw std::runtime_error("Failed to create command allocator");
    }

    this->cmd_allocator->SetName(name);

    if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&this->cmd_list)))) {
        throw std::runtime_error("Failed to create command list");
    }
    
    this->cmd_list->SetName(name);

    if (FAILED(device->CreateFence(this->fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence)))) {
        throw std::runtime_error("Failed to create fence");
    }

    this->fence->SetName(name);
    this->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    this->is_setup = true;
}

void CommandContext::reset() {
    std::scoped_lock _{this->mtx};
    this->wait(2000);

    this->cmd_allocator.Reset();
    this->cmd_list.Reset();
    this->fence.Reset();
    this->fence_value = 0;
    CloseHandle(this->fence_event);
    this->fence_event = 0;
    this->waiting_for_fence = false;
}

void CommandContext::wait(uint32_t ms) {
    std::scoped_lock _{this->mtx};

	if (this->fence_event && this->waiting_for_fence) {
        WaitForSingleObject(this->fence_event, ms);
        ResetEvent(this->fence_event);
        this->waiting_for_fence = false;
        if (FAILED(this->cmd_allocator->Reset())) {
            throw std::runtime_error("Failed to reset command allocator");
        }

        if (FAILED(this->cmd_list->Reset(this->cmd_allocator.Get(), nullptr))) {
            throw std::runtime_error("Failed to reset command list");
        }

        this->has_commands = false;
    }
}

void CommandContext::execute(ID3D12CommandQueue* command_queue) {
    std::scoped_lock _{this->mtx};
    
    if (this->has_commands) {
        if (FAILED(this->cmd_list->Close())) {
            throw std::runtime_error("Failed to close command list");
        }
        
        ID3D12CommandList* const cmd_lists[] = {this->cmd_list.Get()};
        command_queue->ExecuteCommandLists(1, cmd_lists);
        command_queue->Signal(this->fence.Get(), ++this->fence_value);
        this->fence->SetEventOnCompletion(this->fence_value, this->fence_event);
        this->waiting_for_fence = true;
        this->has_commands = false;
    }
}
}