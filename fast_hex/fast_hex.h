//
// Created by baojian on 25-8-12.
//

#ifndef FAST_HEX_H
#define FAST_HEX_H
#include <stdint.h>
#include <stddef.h>

// Scalar look-up table version. len is number of dest bytes (1/2 the size of src).
void decodeHexLUT(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t dest_len);

// Optimized scalar look-up table version (avoids a shift). len is number of dest bytes (1/2 the size of src).
void decodeHexLUT4(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t dest_len);

// Scalar version. len is number of src bytes. dest must be twice the size of src.
void encodeHex(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t src_len);
void encodeHexLower(uint8_t* __restrict__ dest, const uint8_t* __restrict__ src, size_t src_len);

#endif //FAST_HEX_H
