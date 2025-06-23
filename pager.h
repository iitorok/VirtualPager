#include "vm_arena.h"
#include "vm_pager.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <queue>
#include <set>
#include <string>



std::unordered_map<pid_t, int> pid_to_virtual_memory;


/*FORMERLY IN VM_PAGER.H*/
struct page_entry{
    int ppn;

    //TYPE == 1 SWAP BACKED, TYPE == 2 FILE BACKED
    int type;
    
    //use to index into swap-backed or file-backed block depending on value of type
    int block;
    std::string filename;
    int resident; // 0 if not in physmem, 1 if it is
    int offset;
    int valid; // 0 if invalid, 1 if valid
};

struct page_info{
    int type;
    bool used;
    bool dirty;
    bool referenced;
    int block;
    int offset;
    std::string filename;
    std::unordered_map<pid_t, std::set<int>> pid_vpn;
};

//
std::vector<page_info> physical_page_info;

//Index is (VADDR - BASE)/PAGE_SIZE 
//std::vector<std::vector<page_entry>> page_tables;

//Each process gets its own page table
//first Index is pid, second is (VADDR - BASE)/PAGE_SIZE 
std::unordered_map<pid_t, std::vector<page_entry>> pid_to_page_entry;

//Indexed by vpn
std::unordered_map<pid_t, std::vector<page_table_entry_t>> pid_to_pagetable;


//std::unordered_map<int, page_table_entry_t> page_table;


//SWAP BACK (index of vector: block, corresponding value: vpn)
std::vector<int> swap_back_block_list;

struct file_backed_info{
    int ppn;
    std::unordered_map<pid_t, std::set<int>> pid_vpn;
    unsigned int read_enable;
    unsigned int write_enable;
    unsigned int resident;
};

//FILE BACK (Uses vm_map to know who is referencing what filenames/blocks even when not loaded into physmem)
std::unordered_map<std::string, std::unordered_map<int, file_backed_info>> file_backed_info_list;

//CLOCK QUEUE: .first is PPN, while .second is referenced bit
std::queue<int> clock_queue;

//CURRENT PROCESS: PID for current process
pid_t current_pid;

//END VM_PAGER.H





//EVICT FUNCTION (to prevent likely repeated code)
int clock_evict();

//A HELPER FUNCTION TO LOOK THROUGH PHYSICAL MEMORY CHECKING FOR EMPTY PAGES. IF NOT EXIST THEN EVICT
int physical_memory_accessor();