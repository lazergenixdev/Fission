/**
 * @file Assert.hpp
 * @author lazergenixdev@gmail.com
 *
 *	 _______   _   _____   _____   _   _____   __    _
 *	|  _____| | | |  ___| |  ___| | | |  _  | |  \  | |
 *	| |___    | |  \ \     \ \    | | | | | | |   \ | |
 *	|  ___|   | |   \ \     \ \   | | | | | | | |\ \| |
 *	| |       | |  __\ \   __\ \  | | | |_| | | | \   |
 *	|_|       |_| |_____| |_____| |_| |_____| |_|  \__|
 *
 *	MIT License
 *
 *	Copyright (c) 2022 Lazergenix
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
#pragma once
#include <Fission/Base/Exception.hpp>

/**
 * Assertions
 */
#if FISSION_DEBUG
#define FISSION_ASSERT( EXPRESSION, ... ) if(!(EXPRESSION)) \
throw ::Fission::base::generic_error("Assertion Failed", \
::Fission::base::error_message(FISSION_THROW_LOCATION()).append("expression",#EXPRESSION).append(__VA_ARGS__) )
#else
#define FISSION_ASSERT( EXPRESSION, ... ) ((void)0)
#endif // FISSION_DEBUG
