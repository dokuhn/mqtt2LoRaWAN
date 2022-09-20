#ifndef ProgramOptions_HELPERCLASSES_HPP
#define ProgramOptions_HELPERCLASSES_HPP

#include <string>
#include <cstdint>


typedef union {
    uint8_t     e8[8];      /* lower 64-bit address */
    uint8_t     e32[2];
} eui64_t;


typedef uint8_t devkey_t[16];


/* Define a completely non-sensical class. */
class magic_number {

public:
    magic_number(int n) : n(n) {}
    int n;
};


class appeui {

public:
    appeui(std::string appeui_hexstring);
    eui64_t application_eui64;
};



class deveui {

public:
    deveui(std::string deveui_hexstring);
    eui64_t device_eui64;
};


class devkey {

public:
    devkey(std::string devkey_hexstring);
    devkey_t device_key;
};



#endif
