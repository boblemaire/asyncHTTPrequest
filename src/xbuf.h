#pragma once

#include <Arduino.h>

#define SEGMENT_SIZE 64

struct xseg {
    xseg    *next;
    uint8_t data[SEGMENT_SIZE];
    xseg():next(nullptr){}
    ~xseg(){}
};

class xbuf {
    public:

        xbuf();
        ~xbuf();

        size_t      write(const uint8_t);
        size_t      write(const uint8_t*, size_t);
        uint8_t     read();
        size_t      read(uint8_t*, size_t);
        size_t      available();
        int         indexOf(const char, const size_t begin=0);
        int         indexOf(const char*, const size_t begin=0);
        String      readStringUntil(const char);
        String      readStringUntil(const char*);
        String      readString(int);
        void        flush();

    protected:

        xseg        *_head;
        xseg        *_tail;
        size_t      _offset;
        size_t      _used;
        size_t      _free;

        void        addSeg();
        void        remSeg();

};