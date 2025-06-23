#include "vm_pager.h"
#include "vm_arena.h"
#include "pager.h"
#include <cstring>
#include <iostream>
#include <string>



/*void assert_machine(pid_t pid){
    for(int i = 0; i < pid_to_page_entry[pid].size(); ++i){
        if(pid_to_page_entry[pid][i].valid == 1){
            //if its read bit is on, assert referenced is too.
            if(pid_to_pagetable[pid][i].read_enable == 1){
            
                if(pid_to_page_entry[pid][i].resident == 0){
                    std::cout<<"VPN "<<i<<" IS READ-ENABLE BUT NOT RESIDENT!!!"<<std::endl;
                    exit(-1);
                }else{//if it is resident

                    if(physical_page_info[pid_to_page_entry[pid][i].ppn].used == false){
                        std::cout<<"VPN = "<<i<<" IN PHYS PAGE BUT NOT ENTRY IS NOT MARKED USED"<<std::endl;
                        exit(-1);
                    }
                    if(physical_page_info[pid_to_page_entry[pid][i].ppn].referenced == 0){
                        std::cout<<"VPN "<<i<<" IS READ_ENABLE BUT NOT REFERENCED!"<<std::endl;
                        exit(-1);
                    }

                    if(pid_to_pagetable[pid][i].write_enable == 0){
                        if(physical_page_info[pid_to_page_entry[pid][i].ppn].dirty == 1){
                            std::cout<<"VPN "<<i<<" IS DIRTY BUT W BIT IS OFF (& R = 1)"<<std::endl;
                            exit(-1);
                        }
                    }
                    
                }
            }else{
                if(pid_to_pagetable[pid][i].write_enable == 1){
                    std::cout<<"VPN "<<i<<" IS WRITE-ENABLE BUT NOT READ-ENABLE "<<std::endl;
                    exit(-1);
                }

                if(pid_to_page_entry[pid][i].resident == 1){
                    if(physical_page_info[pid_to_page_entry[pid][i].ppn].referenced == 1){
                        std::cout<<"VPN "<<i<<" IS REFERENCED BUT R/W IS OFF"<<std::endl;
                        exit(-1);
                    }
                }
            }

        }
    }
}*/




/*EDIT_PTE_BITS:
------------------------------------------------------
-> Helper function used to set page_table_entry_t object
bits according to arg.
------------------------------------------------------*/

void edit_pte_bits(int arg, page_table_entry_t &pte){
    if(arg == 0){
        pte.read_enable = 0;
        pte.write_enable = 0;
    }else{
        pte.read_enable = 1;
        pte.write_enable = 1;
    }

}


/*VADDR_TO_PADDR: 
----------------------------------------------------------------------------------
-> A helper function that, given the virtual address and an index into physical memory
(provided by vaddr_to_paddr), checks the validity of a file string in physical memory
and returns a string to be stored in a page_entry object for a file-backed page.
----------------------------------------------------------------------------------*/
std::string paddr_to_filename(uintptr_t i, const char *filename, bool &err) {
    
    std::string file_nm;
            //get the vpn and then do a fault if it's different?

            int file_vpn = (reinterpret_cast<uintptr_t>(filename)
            - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;

            int curr_vpn = (reinterpret_cast<uintptr_t>(filename)
            - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;

            //while(static_cast<char *>(vm_physmem)[i] != '\0'){
          
            while(static_cast<char *>(vm_physmem)[i] != '\0'){
                //Tbh I'm not positive it should/will ever get here
                //is this okay?
                /*if(i > reinterpret_cast<uintptr_t>(VM_ARENA_SIZE)){
                    return nullptr;
                }*/

                //int check = vm_fault(filename, 0);

                /*if(check == -1){
                   
                    return nullptr;
                }*/

                //INCREMENT POINTER FOR NEXT CHECK
                filename++;

                file_nm += static_cast<char *>(vm_physmem)[i];

                file_vpn = (reinterpret_cast<uintptr_t>(filename)
                - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;

                if(file_vpn != curr_vpn){//new virtual page!
                    int offset = (reinterpret_cast<uintptr_t>(filename) - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) % VM_PAGESIZE;

                    if(pid_to_page_entry[current_pid][file_vpn].resident == 1 && (pid_to_pagetable[current_pid][file_vpn].read_enable == 1)){

                        i = pid_to_page_entry[current_pid][file_vpn].ppn * VM_PAGESIZE + offset;

                    }else{

                        int check_valid = vm_fault(filename, 0);
                        if(check_valid == -1){
                            err = true;
                            return "";
                        }

                        i = pid_to_page_entry[current_pid][file_vpn].ppn * VM_PAGESIZE + offset;
                    }

                    curr_vpn = file_vpn;

                }else{
                    ++i;
                }

            }

         
    return file_nm;

}

/*VADDR_TO_PADDR: 
----------------------------------------------------------------------------------
-> A helper function that, given the virtual address, calculates a physical address
in the form of an index i. 
----------------------------------------------------------------------------------*/
std::string vaddr_to_paddr(const char* filename, bool &err) {
        
        int vpn = (reinterpret_cast<uintptr_t>(filename) - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;
    
            //offset = (virtual address - ARENA_BASEADDR) % VM_PAGESIZE
        int offset = (reinterpret_cast<uintptr_t>(filename) - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) % VM_PAGESIZE;

        uintptr_t i = 0;
        //Need to ensure read bit is on
        //can crash it here?????
        if((pid_to_page_entry[current_pid][vpn].resident == 1) && (pid_to_pagetable[current_pid][vpn].read_enable == 1)){
            i = pid_to_page_entry[current_pid][vpn].ppn * VM_PAGESIZE + offset;
        }else{
            
            int check_valid = vm_fault(filename, 0);
            
            if(check_valid == -1){
                err = true;
                return "";
            }

            i = pid_to_page_entry[current_pid][vpn].ppn * VM_PAGESIZE + offset;
        }
        
        
        return paddr_to_filename(i, filename, err);

}


/*CLOCK_EVICT: 
----------------------------------------------------------------------------------
-> A helper function for evicting physical pages using LRU method.
-> Returns the new empty physical page number.
-> Traverses through every entry in the clock queue checking for the most recent
one with a referenced bit of 0. If the entry at the top has a referenced bit of 1,
it will be set to 0 along with its read and write enable bits and pushed to the back
of the clock queue.
-> If a dirty page is evicted, its data will be written back to either the swap block
or file block.
----------------------------------------------------------------------------------*/

int clock_evict(){
   
    while(true){

        if(clock_queue.empty()){

            break;
        }
        
        int current_ppn = clock_queue.front();    
    
        if (current_ppn == 0) continue;
    
        if(physical_page_info[current_ppn].referenced == false){

                if(physical_page_info[current_ppn].dirty){
            
                    //pid_t pid = physical_page_info[current_ppn].pid_vpn.begin()->first;
                 
                    //int vpn = *physical_page_info[current_ppn].pid_vpn.begin()->second.begin();
                    
                    void * physical_addr = static_cast<char*>(vm_physmem) + current_ppn * VM_PAGESIZE + physical_page_info[current_ppn].offset;
                    //NEED TO FIND ANOTHER WAY TO DO THIS
                    if (physical_page_info[current_ppn].type == 1) {
                        //Write back to a swap block
                        file_write(nullptr, physical_page_info[current_ppn].block, physical_addr);

                    
                    }else if(physical_page_info[current_ppn].type == 2) {
                        //Write back to a file block
                        file_write(physical_page_info[current_ppn].filename.c_str(), physical_page_info[current_ppn].block, static_cast<char*>(vm_physmem) + current_ppn * VM_PAGESIZE + physical_page_info[current_ppn].offset);
                    }
                }
                
        
                if(!physical_page_info[current_ppn].pid_vpn.empty()){ 
                    //for safer erasing
                    std::vector<int> removals;
                    for(auto&[pid, vpn_set] : physical_page_info[current_ppn].pid_vpn){
                      
                        if(physical_page_info[current_ppn].pid_vpn.empty()){
                            
                            break;
                        }

                        if((pid_to_page_entry.find(pid) == pid_to_page_entry.end())||( pid_to_pagetable.find(pid) == pid_to_pagetable.end())){
                            continue;
                        }

                        for(auto& vpn : vpn_set){
                            
                            //If it hasn't been deleted...
                            if(pid_to_page_entry.find(pid) != pid_to_page_entry.end()){

                                pid_to_page_entry[pid][vpn].resident = 0;

                            }
                          
                            edit_pte_bits(0, pid_to_pagetable[pid][vpn]);
                        }

                        //Write back to memory: find the swap or file block it owns
                        //physical_page_info[current_ppn].pid_vpn.erase(pid);
                        removals.push_back(pid);
                    }

                    for(auto pid : removals){
                        physical_page_info[current_ppn].pid_vpn.erase(pid);
                    }

                }

                if(physical_page_info[current_ppn].type == 2){
                    //Don't they all have the same filename and block? (Does this need to be in the loop?)
                    
                    file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].resident = 0;
                    file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].ppn = 0;
                    file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].read_enable = 0;
                    file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].write_enable = 0;
                }
              
                physical_page_info[current_ppn].dirty = false;
                physical_page_info[current_ppn].referenced = false;
                physical_page_info[current_ppn].used = false;
                physical_page_info[current_ppn].type = 0;
                clock_queue.pop();
             
                return current_ppn;

        } else {

            physical_page_info[current_ppn].referenced = false;
            
             if(!physical_page_info[current_ppn].pid_vpn.empty()){
              
                for(auto&[pid, vpn_set] : physical_page_info[current_ppn].pid_vpn){
                    
                    for(auto& vpn : vpn_set){

                        if(pid_to_pagetable.find(pid) != pid_to_pagetable.end()){
                            edit_pte_bits(0, pid_to_pagetable[pid][vpn]);
                        }   
                        
                    }
                }
   
            }
            
            if(physical_page_info[current_ppn].type == 2){
                //Again, does this need to be in a loop?
                file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].read_enable = 0;
                file_backed_info_list[physical_page_info[current_ppn].filename][physical_page_info[current_ppn].block].write_enable = 0;
                //edit_pte_bits(0, pid_to_pagetable[pid][vpn]);
            
            }

          
            clock_queue.push(current_ppn);
            clock_queue.pop();

        }
    }
    return -1;
}


/*VM_INIT:
----------------------------------------------------------------------
->Initializes the pager by resizing a physical page info vector with the
specified memory pages and pushing the specified number of swap blocks 
into the swap block list to be used later.
->Initializes the zero page in physical memory and adjusts the bits for it 
accordingly.
-----------------------------------------------------------------------*/
void vm_init(unsigned int memory_pages, unsigned int swap_blocks) {

    physical_page_info.resize(memory_pages);

    swap_back_block_list.reserve(swap_blocks);
    for (int i = swap_blocks - 1; i >= 0; --i) {
        swap_back_block_list.push_back(i);
    }

    std::memset(vm_physmem, 0, VM_PAGESIZE);
    
    physical_page_info[0].used = true;
    physical_page_info[0].dirty = false;
    physical_page_info[0].referenced = true;

}


/*VM_CREATE: 
-----------------------------------------------------------------
-> Called when the parent process(parent_pid) creates a new process (child_pid).
-> Initializes a new empty arena for the new process and resizes our page. 
-> Assigns a virtual page number to each new virtual page in the arena.
-----------------------------------------------------------------*/
int vm_create(pid_t parent_pid, pid_t child_pid) {

    pid_to_pagetable[child_pid].resize(VM_ARENA_SIZE/VM_PAGESIZE);

    pid_to_page_entry[child_pid].resize(VM_ARENA_SIZE/VM_PAGESIZE);
   
    pid_to_virtual_memory[child_pid] = 0;

    uintptr_t lowest_vaddr = reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR);

    //Fill up new process's page table with (invalid) virtual pages. Page table size is ARENASIZE / VM_PAGESIZE
    for(uintptr_t i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){

        pid_to_page_entry[child_pid][i].valid = 0;
        pid_to_page_entry[child_pid][i].offset = 
        (lowest_vaddr - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) % VM_PAGESIZE;
        
        pid_to_page_entry[child_pid][i].resident = 0;

        lowest_vaddr += VM_PAGESIZE;

        pid_to_pagetable[child_pid][i] = {.ppage = 0, .read_enable = 0, .write_enable = 0};

    }
    

    return 0;

}


/*VM_SWITCH:
--------------------------------------------------
-> Sets current_pid = pid specified by the argument.
Current_pid is used to index into our page_entry data
structures to access information for entries in the process.
-> Sets the PBTR to point to the new process's page table.
---------------------------------------------------*/

void vm_switch(pid_t pid){
    
    current_pid = pid;
    page_table_base_register  = &pid_to_pagetable[pid][0];

}


 /*VM_FAULT
 -------------------------------------------------------
 -> Called when current process has a fault at virtual address addr. 

 -> If the page is resident, we simply set the required read/write bits
 and return 0 for success.

 -> If the page is not resident, we perform a read fault to load the 
 page into memory. If file_read returns -1 (a failure), then we return -1
 in vm_fault as well to let the application know the filename was invalid

 -> If the page is a swap-backed page pointing to the zero page, we perform
 a copy-on-write operation; copying the contents of the zero page into a new
 physical page.

 -> Returns 0 on success, -1 on failure.
 -------------------------------------------------------*/

int vm_fault(const void* addr, bool write_flag){

    //assert_machine(current_pid);

    int vpn = (reinterpret_cast<uintptr_t>(addr)
        - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;
 
    if(pid_to_page_entry[current_pid][vpn].valid == 0){
        return -1;
    }
    

    //IF READ_ENABLE AND WRITE_ENABLE ARE BOTH 0 BUT IN PHYSICAL MEMORY:
    if((pid_to_page_entry[current_pid][vpn].resident == 1) && (write_flag == 0) 
    && (pid_to_page_entry[current_pid][vpn].type == 1) && (pid_to_page_entry[current_pid][vpn].ppn != 0)){
        //IF DIRTY, NEED TO SET BOTH READ AND WRITE = 1
        if(pid_to_pagetable[current_pid][vpn].read_enable == 1){
            
            return 0;

        }else{

            if(physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].dirty == true){
                pid_to_pagetable[current_pid][vpn].write_enable = 1;
            }
            
            //LET THE PAGER KNOW PAGE IS ALREADY IN PHYSMEM
            pid_to_pagetable[current_pid][vpn].read_enable = 1;
            physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].referenced = true;
            
            return 0;

        }
        
        
    //IF RESIDENT AND READ-ENABLE AND WRITE FAULT
    }else if((pid_to_page_entry[current_pid][vpn].resident == 1) && (write_flag == 1) 
    && (pid_to_page_entry[current_pid][vpn].type == 1) && (pid_to_page_entry[current_pid][vpn].ppn != 0)){
        
        //MAKE IT DIRTY (should already be at this point...)
        physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].dirty = true;

        edit_pte_bits(1, pid_to_pagetable[current_pid][vpn]);
        physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].referenced = true;


        return 0;
    }

    if(((pid_to_page_entry[current_pid][vpn].resident == 1) && (write_flag == 1)) && pid_to_page_entry[current_pid][vpn].type == 2){
        
        //lET THE PAGER KNOW PAGE IS ALREADY IN PHYSMEM
        file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].read_enable = 1;
        file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].write_enable = 1;

       
        physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].referenced = true;
        //MAKE IT DIRTY
        physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].dirty = true;

        for(auto& [temp_pid, vpn_set] : file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].pid_vpn){
            for(auto& temp_vpn : vpn_set){

                if(pid_to_pagetable.find(temp_pid) != pid_to_pagetable.end()){
                    edit_pte_bits(1, pid_to_pagetable[temp_pid][temp_vpn]);
                }

            }

        }
     

        return 0;
    }

    if(((pid_to_page_entry[current_pid][vpn].resident == 1) && (write_flag == 0)) && pid_to_page_entry[current_pid][vpn].type == 2){
      
        //IF DIRTY, NEED TO SET BOTH READ AND WRITE = 1
        if(physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].dirty == true){
          
            //pid_to_pagetable[current_pid][vpn].write_enable = 1;
           
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].write_enable = 1;
            
            for(auto& [temp_pid, vpn_set] : file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].pid_vpn){
                for(auto& temp_vpn : vpn_set){
    
                    if(pid_to_pagetable.find(temp_pid) != pid_to_pagetable.end()){
                        pid_to_pagetable[temp_pid][temp_vpn].write_enable = 1;
                    }
    
                }
            }
        }
      
        //lET THE PAGER KNOW PAGE IS ALREADY IN PHYSMEM
        file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].read_enable = 1;

        physical_page_info[pid_to_page_entry[current_pid][vpn].ppn].referenced = true;

     
        for(auto& [temp_pid, vpn_set] : file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].pid_vpn){
            for(auto& temp_vpn : vpn_set){

                if(pid_to_pagetable.find(temp_pid) != pid_to_pagetable.end()){
                    pid_to_pagetable[temp_pid][temp_vpn].read_enable = 1;
                }

            }

        }
        
        return 0;
    }

    //FIND AN AVAILABLE PHYSICAL PAGE AND RETURN ITS NUMBER

    int empty_ppn = physical_memory_accessor();
    //what if I popped here?


    void* physical_addr = static_cast<char*>(vm_physmem) + (empty_ppn * VM_PAGESIZE) + pid_to_page_entry[current_pid][vpn].offset;

    //CHECK ONCE IF THE FILE IS VALID. IF NOT, RETURN -1.
    if((pid_to_page_entry[current_pid][vpn].resident == 0) || (pid_to_page_entry[current_pid][vpn].type == 1 && pid_to_page_entry[current_pid][vpn].ppn != 0 && pid_to_page_entry[current_pid][vpn].resident == 0)){
      
        int file_legal;
        if(pid_to_page_entry[current_pid][vpn].type == 1){

            file_legal = file_read(nullptr, pid_to_page_entry[current_pid][vpn].block, physical_addr);

        }else{
           
            file_legal = file_read(pid_to_page_entry[current_pid][vpn].filename.c_str(), pid_to_page_entry[current_pid][vpn].block, physical_addr);
          
        }
    

        if(file_legal == -1) {
          
            return -1;

        }

    }
    

    clock_queue.push(empty_ppn);
    

    //WRITE FAULT
    if(write_flag == 1){
     
        //COPY-ON-WRITE CASE FOR A SWAP-BACKED PAGE
        if(pid_to_page_entry[current_pid][vpn].type == 1 && pid_to_page_entry[current_pid][vpn].ppn == 0){
         
            physical_page_info[0].pid_vpn[current_pid].erase(vpn);
            
            pid_to_pagetable[current_pid][vpn].ppage = empty_ppn;

        
            physical_page_info[empty_ppn].pid_vpn[current_pid].emplace(vpn);
            physical_page_info[empty_ppn].type = 1;

            //WRITE ALL 0s TO THE NEW PHYSICAL PAGE
            std::memset(physical_addr, 0, VM_PAGESIZE);

            edit_pte_bits(1, pid_to_pagetable[current_pid][vpn]);
            
            pid_to_page_entry[current_pid][vpn].resident = 1;
            pid_to_page_entry[current_pid][vpn].ppn = empty_ppn;


        //SWAP-BACKED, BUT NOT COPY-ON-WRITE, not resident
        }else if(pid_to_page_entry[current_pid][vpn].type == 1 && pid_to_page_entry[current_pid][vpn].ppn != 0){

            pid_to_pagetable[current_pid][vpn].ppage = empty_ppn;
            physical_page_info[empty_ppn].pid_vpn[current_pid].emplace(vpn);
            physical_page_info[empty_ppn].type = 1;

            edit_pte_bits(1, pid_to_pagetable[current_pid][vpn]);

            pid_to_page_entry[current_pid][vpn].resident = 1;
            pid_to_page_entry[current_pid][vpn].ppn = empty_ppn;
        //FILE-BACKED
        }else{
          
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].ppn = empty_ppn;
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].read_enable = 1;
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].write_enable = 1;
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].resident = 1;
            physical_page_info[empty_ppn].type = 2;

            for(auto& [temp_pid, vpn_set] : file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].pid_vpn){
                for(auto& temp_vpn : vpn_set){
                   

                    if(pid_to_pagetable.find(temp_pid) != pid_to_pagetable.end()){
                        pid_to_pagetable[temp_pid][temp_vpn].ppage = empty_ppn;
                        edit_pte_bits(1, pid_to_pagetable[temp_pid][temp_vpn]);
                        physical_page_info[empty_ppn].pid_vpn[temp_pid].emplace(temp_vpn);
                    }

                    if(pid_to_page_entry.find(temp_pid) != pid_to_page_entry.end()){
                        pid_to_page_entry[temp_pid][temp_vpn].resident = 1;
                        pid_to_page_entry[temp_pid][temp_vpn].ppn = empty_ppn;
                    }
                    
                }

            }
        }
    
        physical_page_info[empty_ppn].dirty = true;
           

    }else{//READ FAULT

        //SWAP-BACKED READ FAULT
        if(pid_to_page_entry[current_pid][vpn].type == 1){
            
            physical_page_info[empty_ppn].pid_vpn[current_pid].emplace(vpn);
            physical_page_info[empty_ppn].type = 1;
           
            pid_to_pagetable[current_pid][vpn].read_enable = 1;
            pid_to_pagetable[current_pid][vpn].ppage = empty_ppn;
            
            pid_to_page_entry[current_pid][vpn].resident = 1;
            pid_to_page_entry[current_pid][vpn].ppn = empty_ppn;

        //FILE-BACKED READ FAULT
        }else{
            
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].ppn = empty_ppn;
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].read_enable = 1;
            file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].resident = 1;
            physical_page_info[empty_ppn].type = 2;

            for(auto& [temp_pid, vpn_set] : file_backed_info_list[pid_to_page_entry[current_pid][vpn].filename][pid_to_page_entry[current_pid][vpn].block].pid_vpn){
                for(auto& temp_vpn : vpn_set){
                
                    
                    if(pid_to_pagetable.find(temp_pid) != pid_to_pagetable.end()){
                        pid_to_pagetable[temp_pid][temp_vpn].ppage = empty_ppn;
                        pid_to_pagetable[temp_pid][temp_vpn].read_enable = 1;
                        physical_page_info[empty_ppn].pid_vpn[temp_pid].emplace(temp_vpn);
                    }

                    //shouldn't matter, but just in case
                    if(pid_to_page_entry.find(temp_pid) != pid_to_page_entry.end()){

                        pid_to_page_entry[temp_pid][temp_vpn].resident = 1;
                        pid_to_page_entry[temp_pid][temp_vpn].ppn = empty_ppn;

                    }
                    
                }
            }

            pid_to_pagetable[current_pid][vpn].read_enable = 1;
            pid_to_pagetable[current_pid][vpn].ppage = empty_ppn;
            
            pid_to_page_entry[current_pid][vpn].resident = 1;
            pid_to_page_entry[current_pid][vpn].ppn = empty_ppn;
                
        }
            
    }

    physical_page_info[empty_ppn].used = true;
    physical_page_info[empty_ppn].referenced = true;
    physical_page_info[empty_ppn].block = pid_to_page_entry[current_pid][vpn].block;
    physical_page_info[empty_ppn].filename = pid_to_page_entry[current_pid][vpn].filename;
    physical_page_info[empty_ppn].offset = pid_to_page_entry[current_pid][vpn].offset;

    return 0;
}


/*VM_DESTROY:
-----------------------------------------------------------------
->Called when the current process exits.
->For swap-backed pages: 
    -> Cleans up all pages used in physical memory 
    -> Restores all swap_blocks used by swap-backed pages in this process
->For file-backed pages:
    -> Just remove the pid from the pid_vpn list for both the physical
    page and the file_backed_info data structure.
-----------------------------------------------------------------*/
void vm_destroy(){

    std::queue<int> temp_queue;
    while(!clock_queue.empty()){
      
        int temp_ppn = clock_queue.front();
        clock_queue.pop();

        if(physical_page_info[temp_ppn].type == 1 && (physical_page_info[temp_ppn].pid_vpn.find(current_pid) != physical_page_info[temp_ppn].pid_vpn.end())){ //MAKE SURE ITS THE RIGHT PID NOT JUST EVICT ALL PAGES OF TYPE 1
            //physical_page_info[temp_ppn].used = false;
            continue;
        }

        
        temp_queue.push(temp_ppn);

    }

    clock_queue = temp_queue;
    
    
    for(size_t i = 0; i < pid_to_page_entry[current_pid].size(); ++i){ //i is the VPN
    
        //if this is a swap-backed page
        if(pid_to_page_entry[current_pid][i].type == 1){
           
            //check if resident; if so, clear from vm_physmem
            if(pid_to_page_entry[current_pid][i].resident == 1){
                
                void* physical_addr = static_cast<char*>(vm_physmem) + pid_to_page_entry[current_pid][i].ppn * VM_PAGESIZE + pid_to_page_entry[current_pid][i].offset; 
               
                std::memset(physical_addr, 0, VM_PAGESIZE);
                //reset the physical_page_info variables to their required defaults
                if(pid_to_page_entry[current_pid][i].ppn == 0) continue; // Never evict the zero page
                if(physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.find(current_pid) != physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.end()){
                    physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.erase(current_pid);
                }
                
                physical_page_info[pid_to_page_entry[current_pid][i].ppn].used = false;
                physical_page_info[pid_to_page_entry[current_pid][i].ppn].dirty = false;
                physical_page_info[pid_to_page_entry[current_pid][i].ppn].referenced = false;
                pid_to_page_entry[current_pid][i].resident = 0;
                
            }
            
            //Push all empty blocks to swap back block list
            swap_back_block_list.push_back(pid_to_page_entry[current_pid][i].block);
          
            //Clear all bits of the pte in each page_entry 
            pid_to_pagetable[current_pid][i].ppage = 0;
            edit_pte_bits(0, pid_to_pagetable[current_pid][i]);
           

        }else if(pid_to_page_entry[current_pid][i].type == 2){
            if(pid_to_page_entry[current_pid][i].resident == 1){


                /*if(physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.find(current_pid) != physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.end()){
                    physical_page_info[pid_to_page_entry[current_pid][i].ppn].pid_vpn.erase(current_pid);
                }*/
                
                //DO NOT DO THIS
                /*physical_page_info[pid_to_page_entry[current_pid][i].ppn].used = false;
                physical_page_info[pid_to_page_entry[current_pid][i].ppn].dirty = false;
                physical_page_info[pid_to_page_entry[current_pid][i].ppn].referenced = false;
                pid_to_page_entry[current_pid][i].resident = 0;*/           

            }
            
            //SHOULD DELETE REGARDLESS IF RESIDENT OR NOT
            file_backed_info_list[pid_to_page_entry[current_pid][i].filename][pid_to_page_entry[current_pid][i].block].pid_vpn.erase(current_pid);

        }
        
    }

    //remove this process's page entry map
    pid_to_page_entry.erase(current_pid);

    //remove this process's page table
    pid_to_pagetable.erase(current_pid);
}


/*VM_MAP
--------------------------------------------------------------------
-> Handles a request by the current process for the lowest invalid virtual page in
the process's arena to be declared valid. 

-> On success, vm_map returns the lowest address of the new virtual page.  

-> vm_map returns nullptr if the arena is full.

-> If filename is nullptr, block is ignored, and the new virtual page is
backed by the swap file, is initialized to all zeroes (from the
application's perspective), and private (i.e., not shared with any other
virtual page).

-> vm_map returns nullptr if the swap file is
out of space.

-> If filename is not nullptr, it points to a null-terminated C string that
specifies a file. 

-> The C string pointed to by filename must reside completely 
in the valid portion of the arena. In this case, vm_map returns nullptr if 
the C string pointed to by filename is not completely in the valid part of the arena.

-> To find the C string, we use the helper function vaddr_to_paddr which will
use vm_fault to verify whether the filename is legitimate.
-------------------------------------------------------------------*/
void* vm_map(const char* filename, unsigned int block){
    
    //IS THIS CORRECT?
    if(pid_to_page_entry[current_pid][(VM_ARENA_SIZE / VM_PAGESIZE) - 1].valid){
        return nullptr;
    }
    
    if(swap_back_block_list.empty() && filename == nullptr) {
        return nullptr;
    }
    

    //(reinterpret_cast<uintptr_t>(filename) - reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)) / VM_PAGESIZE;

    if((filename != nullptr) && (reinterpret_cast<uintptr_t>(filename) > (reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR) + VM_ARENA_SIZE))){

        return nullptr;
    }else if((filename != nullptr) && (reinterpret_cast<uintptr_t>(filename) < (reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR)))){
        return nullptr;
        
    }
    
    page_entry new_entry;
    new_entry.valid = 1;
    
    if(filename == nullptr) {

        new_entry.ppn = 0;
        new_entry.resident = 1;
        
        new_entry.type = 1;
        new_entry.block = swap_back_block_list.back();
       
        swap_back_block_list.pop_back();
      
        new_entry.offset = pid_to_page_entry[current_pid][pid_to_virtual_memory[current_pid]].offset;

        pid_to_pagetable[current_pid][pid_to_virtual_memory[current_pid]].read_enable = 1;
      
    }else {//FILENAME IS READ FROM A SWAP-BACKED PAGE IN PHYSICAL MEMORY. IF NOT IN PHYSICAL MEMORY, NEED TO INCUR A READ FAULT
     
        std::string file_nm;
        bool err = false;

        file_nm = vaddr_to_paddr(filename, err); //an an error parameter to see if out of bounds!
        
        if(err == true){
        
            return nullptr;
        }
       
        new_entry.filename = file_nm;
            

        if(file_backed_info_list[file_nm].find(block) == file_backed_info_list[file_nm].end()){
      
            new_entry.resident = 0;

            file_backed_info new_info;
            new_info.pid_vpn[current_pid].emplace(pid_to_virtual_memory[current_pid]);
            new_info.read_enable = 0;
            new_info.write_enable = 0;
            new_info.resident = 0;

            file_backed_info_list[file_nm][block] = new_info;
        }else{
            
            new_entry.resident = file_backed_info_list[file_nm][block].resident;

            file_backed_info_list[file_nm][block].pid_vpn[current_pid].emplace(pid_to_virtual_memory[current_pid]);
          
            if((file_backed_info_list[file_nm][block].resident == 1) && (physical_page_info[file_backed_info_list[file_nm][block].ppn].used)){
               
                new_entry.ppn = file_backed_info_list[file_nm][block].ppn;
                physical_page_info[new_entry.ppn].pid_vpn[current_pid].emplace(pid_to_virtual_memory[current_pid]);

                if(physical_page_info[file_backed_info_list[file_nm][block].ppn].referenced == 1){
                    //SET R|W BITS AND PPN FOR NEW PAGE
                    pid_to_pagetable[current_pid][pid_to_virtual_memory[current_pid]].read_enable = file_backed_info_list[file_nm][block].read_enable;
                    pid_to_pagetable[current_pid][pid_to_virtual_memory[current_pid]].write_enable = file_backed_info_list[file_nm][block].write_enable;
                }

                pid_to_pagetable[current_pid][pid_to_virtual_memory[current_pid]].ppage = file_backed_info_list[file_nm][block].ppn;
                
            }
            
        }
        new_entry.type = 2;
        new_entry.block = block;
   
        new_entry.offset = pid_to_page_entry[current_pid][pid_to_virtual_memory[current_pid]].offset;        
    }
    
    //SET THE NEW PAGE ENTRY
    pid_to_page_entry[current_pid][pid_to_virtual_memory[current_pid]] = new_entry;

    uintptr_t old_vpn = pid_to_virtual_memory[current_pid];

    pid_to_virtual_memory[current_pid]++;
  
    return reinterpret_cast<void*>(old_vpn * VM_PAGESIZE + reinterpret_cast<uintptr_t>(VM_ARENA_BASEADDR));
}

/*PHYSICAL_MEMORY_ACCESSOR
------------------------------------------
-> Helper function used to traverse a physical page
data structure and look for empty pages.
-> If none are found, perform the clock evict
procedure.
------------------------------------------*/
int physical_memory_accessor(){ 
    
    //Looking for an empty physical page first
    for(size_t i = 0; i < physical_page_info.size(); i++) {
        
        if(!physical_page_info[i].used){

            std::queue<int> temp_queue;
            while(!clock_queue.empty()){

                int temp_ppn = clock_queue.front();
                clock_queue.pop();

                if(temp_ppn == i){ //MAKE SURE ITS THE RIGHT PID NOT JUST EVICT ALL PAGES OF TYPE 1
                   
                    continue;
                }

                temp_queue.push(temp_ppn);
            
            }

            clock_queue = temp_queue; 
            return i;

        }

    }

    //If there's no empty physical page, run the clock to find and evict, returning the evicted ppn
    return clock_evict();

}
