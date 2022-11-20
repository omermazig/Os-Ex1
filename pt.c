//
// Created by User on 11/18/2022.
//
#include "os.h"

// Virtual page numbers are 45 bits long (57 used bits in translation from the address - 12 offset bits).
static int VIRTUAL_PAGE_NUMBER_BITS = 45;
// A page table entry is 64 bits -> 8 bytes, and the frame size is 4KB -> 4096 bytes. That means
// that we have 4096/8=512 page table entries, meaning that you need 9 bits to represent.
static int BITS_PER_TRIE_LEVEL = 9;
// So we need 45/9=5 level in the TRIE
static int TRIE_LEVELS = 5;
//    12 First bits are not relevant (valid bit + 11 unused bits)
static int PHYS_ADDR_IRRELEVANT_BITS = 12;

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn) {
    uint64_t level_symbol;
    pt <<= PHYS_ADDR_IRRELEVANT_BITS;
    uint64_t *va = phys_to_virt(pt);


    for (int level_i = 1; level_i < TRIE_LEVELS; ++level_i) {
        int offset = VIRTUAL_PAGE_NUMBER_BITS - BITS_PER_TRIE_LEVEL * level_i;
        level_symbol = vpn >> offset;
//        Check valid bit of node, to see if we need to create new node.
        if (!(va[level_symbol] & 1)) {
            if (ppn == NO_MAPPING)
//                That means that it's OK there's no node, since there is no mapping.
                return;
            else {
//                Allocate new memory
                uint64_t new_ppn = alloc_page_frame();
//                Add new node with padding and 1 as the valid bit
                va[level_symbol] = (new_ppn << PHYS_ADDR_IRRELEVANT_BITS) + 1;
            }
        }
//        Update pointer
        va = phys_to_virt(va[level_symbol] - 1);
//        Update "vpn" for next loop
        vpn -= level_symbol << offset;
    }
//    After the final loop, vpn is the final symbol
    level_symbol = vpn;
//    Finally, add the given ppn with padding and 1 as the valid bit - As long as it's not a NO_MAPPING
    va[level_symbol] = (ppn == NO_MAPPING) ? 0 : (ppn << PHYS_ADDR_IRRELEVANT_BITS) + 1;
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn) {
    uint64_t level_symbol;
    pt <<= PHYS_ADDR_IRRELEVANT_BITS;
    uint64_t *va = phys_to_virt(pt);


    for (int level_i = 1; level_i < TRIE_LEVELS; ++level_i) {
        int offset = VIRTUAL_PAGE_NUMBER_BITS - BITS_PER_TRIE_LEVEL * level_i;
        level_symbol = vpn >> offset;
//        Check valid bit of node, to see if there's any point to continue.
        if (!(va[level_symbol] & 1)) {
//          That means that there's no mapping to a ppn
            return NO_MAPPING;
        }
//        Update pointer
        va = phys_to_virt(va[level_symbol] - 1);
//        Update "vpn" for next loop
        vpn -= level_symbol << offset;
    }
    //    After the final loop, vpn is the final symbol
    level_symbol = vpn;

    return !(va[level_symbol] & 1) ? NO_MAPPING : va[level_symbol] >> 12;

}