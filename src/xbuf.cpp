#include <xbuf.h>

xbuf::xbuf(const uint16_t segSize)
    : _head(nullptr)
    , _tail(nullptr)
    , _used(0)
    , _free(0)
    , _offset(0) {
    _segSize = (segSize + 3) & -4;//((segSize + 3) >> 2) << 2;
}

//*******************************************************************************************************************
xbuf::~xbuf(){
    flush();
}

//*******************************************************************************************************************
size_t      xbuf::write(const uint8_t byte){
    return write((uint8_t*) &byte, 1);
}

//*******************************************************************************************************************
size_t      xbuf::write(const char* buf){
    return write((uint8_t*)buf, strlen(buf));
}

//*******************************************************************************************************************
size_t      xbuf::write(String string){
    return write((uint8_t*)string.c_str(), string.length());
}

//*******************************************************************************************************************
size_t      xbuf::write(const uint8_t* buf, const size_t len){
    size_t supply = len;
    while(supply){
        if(!_free){
            addSeg();
        }
        size_t demand = _free < supply ? _free : supply;
        memcpy(_tail->data + ((_offset + _used) % _segSize), buf + (len - supply), demand);
        _free -= demand;
        _used += demand;
        supply -= demand;
    }
    return len;
}

//*******************************************************************************************************************
size_t      xbuf::write(xbuf* buf, const size_t len){
    size_t supply = len;
    if(supply > buf->available()){
        supply = buf->available();
    }
    size_t read = 0;
    while(supply){
        if(!_free){
            addSeg();
        }
        size_t demand = _free < supply ? _free : supply;
        read += buf->read(_tail->data + ((_offset + _used) % _segSize), demand);
        _free -= demand;
        _used += demand;
        supply -= demand;
    }
    return read;
}

//*******************************************************************************************************************
uint8_t     xbuf::read(){
    uint8_t byte = 0;
    read((uint8_t*) &byte, 1);
    return byte;
}

//*******************************************************************************************************************
uint8_t     xbuf::peek(){
    uint8_t byte = 0;
    peek((uint8_t*) &byte, 1);
    return byte;
}

//*******************************************************************************************************************
size_t      xbuf::read(uint8_t* buf, const size_t len){
    size_t read = 0;
    while(read < len && _used){
        size_t supply = (_offset + _used) > _segSize ? _segSize - _offset : _used;
        size_t demand = len - read;
        size_t chunk = supply < demand ? supply : demand;
        memcpy(buf + read, _head->data + _offset, chunk);
        _offset += chunk;
        _used -= chunk;
        read += chunk;
        if(_offset == _segSize){
            remSeg();
            _offset = 0;        
        }
    }
    if( ! _used){
        flush();
    }
    return read;

}

//*******************************************************************************************************************
size_t      xbuf::peek(uint8_t* buf, const size_t len){
    size_t read = 0;
    xseg* seg = _head;
    size_t offset = _offset;
    size_t used = _used;
    while(read < len && used){
        size_t supply = (offset + used) > _segSize ? _segSize - offset : used;
        size_t demand = len - read;
        size_t chunk = supply < demand ? supply : demand;
        memcpy(buf + read, seg->data + offset, chunk);
        offset += chunk;
        used -= chunk;
        read += chunk;
        if(offset == _segSize){
            seg = seg->next;
            offset = 0;        
        }
    }
    return read;
}

//*******************************************************************************************************************
size_t      xbuf::available(){
    return _used;
}

//*******************************************************************************************************************
int      xbuf::indexOf(const char target, const size_t begin){
    char targetstr[2] = " ";
    targetstr[0] = target;
    return indexOf(targetstr, begin);
}

//*******************************************************************************************************************
int      xbuf::indexOf(const char* target, const size_t begin){
    size_t targetLen = strlen(target);
    if(targetLen > _segSize || targetLen > _used) return -1;
    size_t searchPos = _offset + begin;
    size_t searchEnd = _offset + _used - targetLen;
    if(searchPos > searchEnd) return -1;
    size_t searchSeg = searchPos / _segSize;
    xseg* seg = _head;
    while(searchSeg){
        seg = seg->next;
        searchSeg --;
    }
    size_t segPos = searchPos % _segSize;
    while(searchPos <= searchEnd){
        size_t compLen = targetLen;
        if(compLen <= (_segSize - segPos)){
            if(memcmp(target,seg->data+segPos,compLen) == 0){
                return searchPos - _offset;
            }
        }
        else {
            size_t compLen = _segSize - segPos;
            if(memcmp(target,seg->data+segPos,compLen) == 0){
                compLen = targetLen - compLen;
                if(memcmp(target+targetLen-compLen, seg->next->data, compLen) == 0){
                    return searchPos - _offset;
                }
            }  
        }
        searchPos++;
        segPos++;
        if(segPos == _segSize){
            seg = seg->next;
            segPos = 0;
        } 
    }
    return -1;
}

//*******************************************************************************************************************
String      xbuf::readStringUntil(const char target){
    return readString(indexOf(target)+1);
}

//*******************************************************************************************************************
String      xbuf::readStringUntil(const char* target){
    int index = indexOf(target);
    if(index < 0) return String();
    return readString(index + strlen(target));
}

//*******************************************************************************************************************
String      xbuf::readString(int endPos){
    String result;
    if( ! result.reserve(endPos+1)){
        return result;
    }
    if(endPos > _used){
        endPos = _used;
    }
    if(endPos > 0 && result.reserve(endPos+1)){
        while(endPos--){
            result += (char)_head->data[_offset++];
            _used--;
            if(_offset >= _segSize){
                remSeg();
            }
        }
    }   
    return result;
}

//*******************************************************************************************************************
String      xbuf::peekString(int endPos){
    String result;
    xseg* seg = _head;
    size_t offset = _offset;
    if(endPos > _used){
        endPos = _used;
    }
    if(endPos > 0 && result.reserve(endPos+1)){
        while(endPos--){
            result += (char)seg->data[offset++];
            if( offset >= _segSize){
                seg = seg->next;
                offset = 0;
            }
        }
    }   
    return result;
}

//*******************************************************************************************************************
void        xbuf::flush(){
    while(_head) remSeg();
    _tail = nullptr;
    _offset = 0;
    _used = 0;
    _free = 0;
}

//*******************************************************************************************************************
void        xbuf::addSeg(){
    if(_tail){
        _tail->next = (xseg*) new uint32_t[_segSize / 4 + 1];
        _tail = _tail->next;
    }
    else {
        _tail = _head = (xseg*) new uint32_t[_segSize / 4 + 1];
    }
    _tail->next = nullptr;
    _free += _segSize;
}

//*******************************************************************************************************************
void        xbuf::remSeg(){
    if(_head){
        xseg *next = _head->next;
        delete[] (uint32_t*) _head;
        _head = next;
        if( ! _head){
            _tail = nullptr;
        }
    }   
    _offset = 0;
}

