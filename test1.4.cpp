#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"



int main() {
    // map two swap-backed pages
    auto *page0 = static_cast<char *>(vm_map(nullptr, 0));
    auto *page1 = static_cast<char *>(vm_map(nullptr, 0));

    // write the filename into virtual memory
    auto *filename = page0 + VM_PAGESIZE - 4;
    std::strcpy(filename, "papers.txt");

    // map a file-backed page
    auto *page2 = static_cast<char *>(vm_map(filename, 0));
}
