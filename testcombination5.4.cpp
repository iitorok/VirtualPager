#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main() {
    
    if (fork() != 0) { // parent
        vm_yield();
        std::cout<<"FIRST PARENT BACK"<<std::endl;
        if (fork() != 0) {


        }else{


        }

    }else{

        if (fork() != 0) { // parent
            auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
            auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
            auto *page2 = static_cast<char *>(vm_map(nullptr, 0));

            std::strcpy(page0, "papers.txt"); 
            std::strcpy(page1, "data1.bin"); 
            std::strcpy(page2, "data2.bin"); 

            //physmem: zero, 0, 1, 2

            auto *fb0 = static_cast<char *>(vm_map(page0, 0));
            auto *fb1 = static_cast<char *>(vm_map(page1, 0)); 
            auto *fb2 = static_cast<char *>(vm_map(page2, 0)); 
    
            std::cout <<"contents of files = "<<fb0[0]<<fb1[0]<<fb2[0]<<std::endl; //evict 0, evict 1
            //physmem: zero, fb0, fb1, fb2, 2
            std::cout <<"contents of sbs = "<<page0[0]<<page1[0]<<page2[0]<<std::endl; //evict 2, fb0, fb1

            //physmem: zero, fb2, 0, 1, 2
            std::strcpy(fb0, "newfileinfo"); //evict fb2
            std::strcpy(fb1, "newfileinfo"); //evict 0
            std::strcpy(fb2, "newfileinfo"); //evict 1

            //physmem: zero, fb0, fb1, fb2, 2
            auto *fb3 = static_cast<char *>(vm_map(page0, 0)); //evict 2
            //physmem: zero, fb0, fb1, fb2, 0
            std::strcpy(fb3, "newfileinfo"); //evict fb0
            //physmem: zero, fb3, fb1, fb2, 0

            std::strcpy(page0, "data1.bin");  
            std::strcpy(page1, "data2.bin");  //evict fb1
            std::strcpy(page2, "data3.bin");  //evict fb2
            vm_yield();
            std::cout <<"contents of sbs = "<<page0[0]<<page1[0]<<page2[0]<<std::endl; 

            std::strcpy(fb0, "writing...");
            //physmem: zero, 0, 1, 2, fb3
    

            std::cout <<"contents of files = "<<fb3[0]<<fb1[0]<<fb2[0]<<std::endl; //evict 0, 1, 2 (I think. fb3 should get re-referenced.)

    
        }else{

            //map 2 swap-backed pages
            auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
            auto *page1 = static_cast<char *>(vm_map(nullptr, 0));
            auto *page2 = static_cast<char *>(vm_map(nullptr, 0));
            auto *page3 = static_cast<char *>(vm_map(nullptr, 0));


            std::strcpy(page1, "data3.bin"); 
            std::strcpy(page2, "data3.bin"); 
            std::strcpy(page3, "data3.bin"); 
            std::strcpy(page0, "data3.bin"); //eviction! should evict 1 and turn all others off
            vm_yield();

            auto *fb0 = static_cast<char *>(vm_map(page2, 0)); //should turn both r and w on for page2
            vm_yield();

            std::cout<<"fb0 = "<<fb0[0]<<std::endl;

            strcpy(page1, "new data");

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

        std::strcpy(page0, "papers.txt"); 
        std::strcpy(page1, "data1.bin"); 

        std::cout<<"contents of fb0 = "<<fb0[0]<<std::endl;

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

    auto *fb4 = static_cast<char *>(vm_map(page1, 0));
    std::strcpy(fb4, "newfilecontents");


    std::cout<<"THIRD END"<<std::endl;



}