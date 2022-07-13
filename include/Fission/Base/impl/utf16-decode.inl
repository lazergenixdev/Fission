/*
* UTF-16 Decode
*
* Copyright (c) 2022 Lazergenix Software
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
* USAGE:
*
* defines:
*   _utfdecode_CurrentLocation              Pointer to start of character
*   _utfdecode_ThrowException(name)         Throws Exception of type 'name'
*   _utfdecode_ReturnCodepoint(codepoint)   Saves 'codepoint' as the codepoint at current location
* optional:
*   _utfdecode_ReplacementChr
*/

#ifndef _utfdecode_ReplacementChr
#define _utfdecode_ReplacementChr U'\uFFFD'
#define __utfdecodeUndefReplacementChr 1
#endif

_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );

#ifdef __utfdecodeUndefReplacementChr
#undef _utfdecodeReplacementChr
#endif
