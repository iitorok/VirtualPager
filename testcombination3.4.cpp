#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    
    if (fork() != 0) { // parent
        vm_yield();




    }else{

        if (fork() != 0) { // parent
            vm_yield();
    
    
    
    
        }else{
            if (fork() != 0) { // parent
                vm_yield();
        
        
        
        
            }else{
                std::cout<<"INNERMOST CHILD"<<std::endl;
        
        
            }
            
            auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
        
            std::strcpy(page0, "papers.txt"); 
        
            //try mapping a bunch of files to the same page
           
            //physmem: zero, 0, 1, 2
        
            auto *fb0 = static_cast<char *>(vm_map(page0, 0));
            auto *fb1 = static_cast<char *>(vm_map(page0, 0)); 
            auto *fb2 = static_cast<char *>(vm_map(page0, 0)); 
            
            std::cout <<"contents of files = "<<fb0[0]<<fb1[0]<<fb2[0]<<std::endl; 
            //physmem: zero, fb0, fb1, fb2, 0
            auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
            std::strcpy(page1, "data1.bin"); 
            
            std::cout <<"contents of sbs = "<<page0[0]<<page1[0]<<std::endl; //evict
        
            auto *fb3 = static_cast<char *>(vm_map(page1, 0));
            std::strcpy(fb3, "newfilecontents");
            //
            std::cout<<"FIRST END"<<std::endl;
    
        }
        
        auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
    
        std::strcpy(page0, "papers.txt"); 
    
        //try mapping a bunch of files to the same page
       
        //physmem: zero, 0, 1, 2
    
        auto *fb0 = static_cast<char *>(vm_map(page0, 0));
        auto *fb1 = static_cast<char *>(vm_map(page0, 0)); 
        auto *fb2 = static_cast<char *>(vm_map(page0, 0)); 
        
        std::cout <<"contents of files = "<<fb0[0]<<fb1[0]<<fb2[0]<<std::endl; 
        //physmem: zero, fb0, fb1, fb2, 0
        auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
        std::strcpy(page1, "data1.bin"); 
        
        std::cout <<"contents of sbs = "<<page0[0]<<page1[0]<<std::endl; //evict
    
        auto *fb3 = static_cast<char *>(vm_map(page1, 0));
        std::strcpy(fb3, "newfilecontents");
        //
        std::cout<<"SECOND END"<<std::endl;

    }
    
    auto *page0 = static_cast<char *>(vm_map(nullptr, 0));

    std::strcpy(page0, "papers.txt"); 

    //try mapping a bunch of files to the same page
   
    //physmem: zero, 0, 1, 2

    auto *fb0 = static_cast<char *>(vm_map(page0, 0));
    auto *fb1 = static_cast<char *>(vm_map(page0, 0)); 
    auto *fb2 = static_cast<char *>(vm_map(page0, 0)); 
    
    std::cout <<"contents of files = "<<fb0[0]<<fb1[0]<<fb2[0]<<std::endl; 
    //physmem: zero, fb0, fb1, fb2, 0
    auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
    std::strcpy(page1, "data1.bin"); 
    
    std::cout <<"contents of sbs = "<<page0[0]<<page1[0]<<std::endl; //evict

    auto *fb3 = static_cast<char *>(vm_map(page1, 0));
    std::strcpy(fb3, "newfilecontents");
    //

    std::cout<<"THIRD END"<<std::endl;

}