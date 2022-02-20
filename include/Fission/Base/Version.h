/**
*
* @file: Version.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/
#include "../config.h"
#include <stdint.h>

_FISSION_BASE_PUBLIC_BEGIN

struct Version
{
  uint16_t maj, min, pat;
  char ext[18] = {0}; // reserved

  constexpr Version(): maj(0), min(0), pat(0) {}

  constexpr Version(uint16_t major, uint16_t minor, uint16_t patch): maj(major), min(minor), pat(patch) {}

  constexpr bool operator>=( const Version & rhs ) const
  {
    return as_number() >= rhs.as_number();
  }
  constexpr bool operator<( const Version & rhs ) const
  {
    return as_number() < rhs.as_number();
  }

  constexpr uint64_t as_number() const { return (uint64_t)maj << 48 | (uint64_t)min << 32 | (uint64_t)pat << 16; }
};

_FISSION_BASE_PUBLIC_END