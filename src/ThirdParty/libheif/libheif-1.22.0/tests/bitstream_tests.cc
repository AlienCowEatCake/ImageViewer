/*
  libheif bitstream unit tests

  MIT License

  Copyright (c) 2024 Brad Hards <bradh@frogmouth.net>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "catch_amalgamated.hpp"
#include "error.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <bitstream.h>


TEST_CASE("read bits") {
  std::vector<uint8_t> byteArray{0x7f, 0xf1, 0b01000001, 0b10000111, 0b10001111};
  BitReader uut(byteArray.data(), (int)byteArray.size());
  uint32_t byte0 = uut.get_bits(8);
  REQUIRE(byte0 == 0x7f);
  uint32_t byte1_high = uut.get_bits(4);
  REQUIRE(byte1_high == 0x0f);
  uint32_t byte1_low = uut.get_bits(4);
  REQUIRE(byte1_low == 0x01);
  uint32_t byte2_partial1 = uut.get_bits(3);
  REQUIRE(byte2_partial1 == 0x02);
  uint32_t byte2_partial2 = uut.get_bits(3);
  REQUIRE(byte2_partial2 == 0x00);
  uint32_t byte2_3_overlap = uut.get_bits(11);
  REQUIRE(byte2_3_overlap == 0b0000001100001111);
}

TEST_CASE("read uint8") {
  std::vector<uint8_t> byteArray{0x7f, 0xf1, 0b01000001, 0b10000111, 0b10001111};
  BitReader uut(byteArray.data(), (int)byteArray.size());
  uint8_t byte0 = uut.get_bits8(8);
  REQUIRE(byte0 == 0x7f);
  uint8_t byte1_high = uut.get_bits8(4);
  REQUIRE(byte1_high == 0x0f);
  uint8_t byte1_low = uut.get_bits8(4);
  REQUIRE(byte1_low == 0x01);
  uint8_t byte2_partial1 = uut.get_bits8(3);
  REQUIRE(byte2_partial1 == 0x02);
  uint8_t byte2_partial2 = uut.get_bits8(3);
  REQUIRE(byte2_partial2 == 0x00);
  uint8_t byte2_3_overlap = uut.get_bits8(8);
  REQUIRE((int)byte2_3_overlap == 0b1100001);
}

TEST_CASE("read uint32") {
  std::vector<uint8_t> byteArray{0x7f, 0b11110001, 0b01000001, 0b10000111, 0b10001111};
  BitReader uut(byteArray.data(), (int)byteArray.size());
  uint32_t byte0 = uut.get_bits32(8);
  REQUIRE(byte0 == 0x7f);
  uint32_t byte1_high = uut.get_bits(1);
  REQUIRE(byte1_high == 0x01);
  uint32_t overlap = uut.get_bits32(30);
  REQUIRE(overlap == 0b111000101000001100001111000111);
}

// --- BitWriter tests ---

TEST_CASE("bitwriter single flags") {
  BitWriter writer;
  writer.write_flag(true);
  writer.write_flag(false);
  writer.write_flag(true);
  writer.write_flag(true);
  writer.write_flag(false);
  writer.write_flag(false);
  writer.write_flag(true);
  writer.write_flag(false);
  auto data = writer.get_data();
  REQUIRE(data.size() == 1);
  REQUIRE(data[0] == 0b10110010);
}

TEST_CASE("bitwriter multi-bit values") {
  BitWriter writer;
  writer.write_bits(0b01, 2);   // 01
  writer.write_bits(0b110, 3);  // 110
  writer.write_bits(0b101, 3);  // 101
  auto data = writer.get_data();
  REQUIRE(data.size() == 1);
  REQUIRE(data[0] == 0b01110101);
}

TEST_CASE("bitwriter cross-byte boundary") {
  BitWriter writer;
  writer.write_bits(0b11111, 5);  // 5 bits
  writer.write_bits(0b000111, 6); // 6 bits - crosses byte boundary
  auto data = writer.get_data();
  REQUIRE(data.size() == 2);
  REQUIRE(data[0] == 0b11111000);
  REQUIRE(data[1] == 0b11100000); // remaining 3 bits + 5 zero-padded
}

TEST_CASE("bitwriter 16-bit value") {
  BitWriter writer;
  writer.write_bits16(0x1234, 16);
  auto data = writer.get_data();
  REQUIRE(data.size() == 2);
  REQUIRE(data[0] == 0x12);
  REQUIRE(data[1] == 0x34);
}

TEST_CASE("bitwriter 32-bit value") {
  BitWriter writer;
  writer.write_bits32(0xDEADBEEF, 32);
  auto data = writer.get_data();
  REQUIRE(data.size() == 4);
  REQUIRE(data[0] == 0xDE);
  REQUIRE(data[1] == 0xAD);
  REQUIRE(data[2] == 0xBE);
  REQUIRE(data[3] == 0xEF);
}

TEST_CASE("bitwriter skip_to_byte_boundary") {
  BitWriter writer;
  writer.write_bits(0b101, 3);
  REQUIRE(writer.get_bits_written() == 3);
  writer.skip_to_byte_boundary();
  REQUIRE(writer.get_bits_written() == 8);
  writer.write_bits8(0xFF, 8);
  auto data = writer.get_data();
  REQUIRE(data.size() == 2);
  REQUIRE(data[0] == 0b10100000);
  REQUIRE(data[1] == 0xFF);
}

TEST_CASE("bitwriter skip_to_byte_boundary already aligned") {
  BitWriter writer;
  writer.write_bits8(0xAB, 8);
  writer.skip_to_byte_boundary(); // should be no-op
  writer.write_bits8(0xCD, 8);
  auto data = writer.get_data();
  REQUIRE(data.size() == 2);
  REQUIRE(data[0] == 0xAB);
  REQUIRE(data[1] == 0xCD);
}

TEST_CASE("bitwriter write_bytes") {
  BitWriter writer;
  writer.write_bits8(0xAA, 8);
  std::vector<uint8_t> bytes = {0x01, 0x02, 0x03};
  writer.write_bytes(bytes);
  auto data = writer.get_data();
  REQUIRE(data.size() == 4);
  REQUIRE(data[0] == 0xAA);
  REQUIRE(data[1] == 0x01);
  REQUIRE(data[2] == 0x02);
  REQUIRE(data[3] == 0x03);
}

TEST_CASE("bitwriter round-trip with BitReader") {
  // Write a sequence of mixed-width values
  BitWriter writer;
  writer.write_bits(2, 2);        // version = 2
  writer.write_flag(false);       // flag1
  writer.write_flag(true);        // flag2
  writer.write_flag(true);        // flag3
  writer.write_flag(false);       // flag4
  writer.write_flag(true);        // flag5
  writer.write_flag(false);       // flag6
  writer.write_flag(true);        // flag7
  writer.write_flag(false);       // flag8
  writer.write_flag(true);        // flag9
  writer.write_bits(1, 2);        // chroma = 1
  writer.write_bits(4, 3);        // orientation = 4
  writer.write_flag(true);        // large_dim
  writer.write_bits(255, 15);     // width-1
  writer.write_bits(127, 15);     // height-1
  writer.write_bits8(0xAB, 8);    // some byte

  auto data = writer.get_data();

  // Read back with BitReader
  BitReader reader(data.data(), (int)data.size());
  REQUIRE(reader.get_bits(2) == 2);      // version
  REQUIRE(reader.get_flag() == false);    // flag1
  REQUIRE(reader.get_flag() == true);     // flag2
  REQUIRE(reader.get_flag() == true);     // flag3
  REQUIRE(reader.get_flag() == false);    // flag4
  REQUIRE(reader.get_flag() == true);     // flag5
  REQUIRE(reader.get_flag() == false);    // flag6
  REQUIRE(reader.get_flag() == true);     // flag7
  REQUIRE(reader.get_flag() == false);    // flag8
  REQUIRE(reader.get_flag() == true);     // flag9
  REQUIRE(reader.get_bits(2) == 1);       // chroma
  REQUIRE(reader.get_bits(3) == 4);       // orientation
  REQUIRE(reader.get_flag() == true);     // large_dim
  REQUIRE(reader.get_bits(15) == 255);    // width-1
  REQUIRE(reader.get_bits(15) == 127);    // height-1
  REQUIRE(reader.get_bits8(8) == 0xAB);   // byte
}

TEST_CASE("bitwriter get_current_byte_index") {
  BitWriter writer;
  REQUIRE(writer.get_current_byte_index() == 0);
  writer.write_bits8(0xFF, 8);
  REQUIRE(writer.get_current_byte_index() == 1);
  writer.write_bits(0, 3);  // partial byte not yet flushed
  REQUIRE(writer.get_current_byte_index() == 1);
  writer.skip_to_byte_boundary();
  REQUIRE(writer.get_current_byte_index() == 2);
}

TEST_CASE("bitwriter zero bits") {
  BitWriter writer;
  writer.write_bits(0, 0);  // writing 0 bits should be a no-op
  REQUIRE(writer.get_bits_written() == 0);
  auto data = writer.get_data();
  REQUIRE(data.empty());
}

TEST_CASE("read float") {
  std::vector<uint8_t> byteArray{0x40, 0x00, 0x00, 0x00};
  std::shared_ptr<StreamReader_memory> stream = std::make_shared<StreamReader_memory>(byteArray.data(), (int)byteArray.size(), false);
  BitstreamRange uut(stream, byteArray.size(), nullptr);
  float f = uut.read_float32();
  REQUIRE(f == 2.0);
}
