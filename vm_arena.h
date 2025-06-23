/*
 * vm_arena.h
 *
 * Constants describing the arena provided by the pager.
 */

#pragma once

#include <stdint.h>
#include <unordered_map>
#include <memory>

/*struct virtual_info {
    int lowest_vpn;
};

std::unordered_map<pid_t, virtual_info> pid_to_virtual_memory;*/
//comment

/* page size for the machine */
static constexpr unsigned int VM_PAGESIZE = 0x10000;

/* virtual address at which the application's arena starts */
static void* const VM_ARENA_BASEADDR = reinterpret_cast<void *>(0x600000000);

/* size (in bytes) of arena */
static constexpr uintptr_t VM_ARENA_SIZE = 0x01000000;









