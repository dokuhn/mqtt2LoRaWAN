#ifndef ProgramOptions_HELPERCLASSES_HPP
#define ProgramOptions_HELPERCLASSES_HPP

#include <cstdint>


typedef union {
    uint8_t     e8[8];      /* lower 64-bit address */
    uint8_t     e32[2]; 
} eui64_t;


/* Define a completely non-sensical class. */
class magic_number {

public:
    magic_number(int n) : n(n) {}
    int n;
};

class appeui {

public:
    appeui(eui64_t application_eui64) : application_eui64{application_eui64} {}
    eui64_t application_eui64;
}; 



#endif
