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
#include "config.h"
#include <cstdint>

_FISSION_BASE_PUBLIC_BEGIN

// Use this when printing versions
#define FISSION_VERSION_FORMAT "%i.%i.%i"

/*!
* @brief Version Number
* @see https://semver.org/
*/
struct Version
{
	// Version: Major.Minor.Patch
	// e.g. 2.2.0
	uint16_t    Major,Minor,Patch;

	// For simplicity, this will NOT be considered in precedence
	char _ext[18] = { 0 };


	constexpr Version() : Major( 0 ), Minor( 0 ), Patch( 0 ) {}

	constexpr Version( uint16_t major, uint16_t minor, uint16_t patch ) : Major( major ), Minor( minor ), Patch( patch ) {}

	constexpr Version( uint16_t major, uint16_t minor, uint16_t patch, const char * ext ) : Major( major ), Minor( minor ), Patch( patch )
	{
		for( int i = 0; i < 18 && *ext; ++i )
			_ext[i] = *ext++;
	}


	constexpr bool operator>=( const Version &rhs ) const
	{
		return as_number() >= rhs.as_number();
	}
	constexpr bool operator<( const Version &rhs ) const
	{
		return as_number() < rhs.as_number();
	}


	constexpr uint64_t as_number() const { return (uint64_t)Major << 32 | (uint64_t)Minor << 16 | (uint64_t)Patch; }

};

_FISSION_BASE_PUBLIC_END