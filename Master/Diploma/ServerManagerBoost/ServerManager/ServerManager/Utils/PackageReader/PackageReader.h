#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>

class PackageReader {
public:
    PackageReader(const char* data, size_t size)
        : buffer(data), bufferSize(size), offset(0) {}

    uint8_t ReadUInt8() {
        EnsureAvailable(sizeof(uint8_t));
        return buffer[offset++];
    }

    uint16_t ReadUInt16() {
        uint16_t val;
        ReadBytes(&val, sizeof(val));
        return val;
    }

    uint32_t ReadUInt32() {
        uint32_t val;
        ReadBytes(&val, sizeof(val));
        return val;
    }

    void ReadBytes(void* out, size_t len) {
        EnsureAvailable(len);
        std::memcpy(out, buffer + offset, len);
        offset += len;
    }

private:
    void EnsureAvailable(size_t len) const
    {
        if (offset + len > bufferSize) {
            throw std::runtime_error("PackerReader: not enough data");
        }
    }

    const char* buffer;
    size_t bufferSize;
    size_t offset;
};