#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

namespace cleo {
struct PackedScript { const char* name; const uint8_t* data; uint32_t size; };
class PackedScripts {
    const uint8_t* position_;
    const uint8_t* end_;
public:
    PackedScripts(const void* data,size_t size) : position_(static_cast<const uint8_t*>(data)),end_(position_+size) {}
    bool next(PackedScript& file) {
        if (position_==end_) return false;
        const auto* zero=static_cast<const uint8_t*>(std::memchr(position_,0,size_t(end_-position_)));
        if (!zero || size_t(end_-zero-1)<sizeof(uint32_t)) { position_=end_;return false; }
        file.name=reinterpret_cast<const char*>(position_);
        position_=zero+1;
        // File names have arbitrary length; the size field is often unaligned.
        std::memcpy(&file.size,position_,sizeof(file.size));position_+=sizeof(file.size);
        if (!file.size || file.size>size_t(end_-position_)) { position_=end_;return false; }
        file.data=position_;position_+=file.size;
        return true;
    }
};
}
