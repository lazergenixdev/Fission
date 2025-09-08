/*
* UTF-8 Decode
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
*   _utfdecode_IsAtOrPastEndOfString(loc)   returns true if pointer 'loc' >= (End Of String)
* optional:
*   _utfdecode_ReplacementChr
*/

#ifndef _utfdecode_ReplacementChr
#define _utfdecode_ReplacementChr U'\uFFFD'
#define __utfdecodeUndefReplacementChr 1
#endif

char32_t x = *_utfdecode_CurrentLocation;

if( 0b1000'0000 & x )
{
	if( 0b0100'0000 & x )
	{
		if( 0b0010'0000 & x )
		{
			if( 0b0001'0000 & x )
			{
				if( 0b0000'1000 & x ) {_utfdecode_ThrowException( invalid_codepoint );}
				else // then 4 bytes in codepoint
				{
					if( x >= 0xF5 )
					{
						_utfdecode_ThrowException( invalid_codepoint );
						_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
					}

					if( _utfdecode_IsAtOrPastEndOfString(_utfdecode_CurrentLocation + 3) )
					{
						_utfdecode_ThrowException( unexpected_end_of_string );
						_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
					}

					x = ( 0b0000'0111 & x ) << 18;

					char32_t _dx1 = *(++_utfdecode_CurrentLocation);
					char32_t _dx2 = *(++_utfdecode_CurrentLocation);
					char32_t _dx3 = *(++_utfdecode_CurrentLocation);

					if( ( _dx1 & 0b1100'0000 ) != 0b1000'0000 ||
						( _dx2 & 0b1100'0000 ) != 0b1000'0000 ||
						( _dx3 & 0b1100'0000 ) != 0b1000'0000 )
					{
						_utfdecode_ThrowException( unexpected_non_continuation_byte );
						_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
					}

					x |= ( _dx1 & 0b0011'1111 ) << 12;
					x |= ( _dx2 & 0b0011'1111 ) << 6;
					x |=   _dx3 & 0b0011'1111;

					++_utfdecode_CurrentLocation;

					_utfdecode_ReturnCodepoint( x );
				}
			}
			else // then 3 bytes in codepoint
			{
				if( _utfdecode_IsAtOrPastEndOfString(_utfdecode_CurrentLocation + 2) )
				{
					_utfdecode_ThrowException( unexpected_end_of_string );
					_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
				}

				x = ( 0b0000'1111 & x ) << 12;

				char32_t _dx1 = *(++_utfdecode_CurrentLocation);
				char32_t _dx2 = *(++_utfdecode_CurrentLocation);

				if( ( _dx1 & 0b1100'0000 ) != 0b1000'0000 ||
					( _dx2 & 0b1100'0000 ) != 0b1000'0000 )
				{
					_utfdecode_ThrowException( unexpected_non_continuation_byte );
					_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
				}

				x |= ( _dx1 & 0b0011'1111 ) << 6;
				x |=   _dx2 & 0b0011'1111;

				++_utfdecode_CurrentLocation;

				_utfdecode_ReturnCodepoint( x );
			}
		}
		else // then 2 bytes in codepoint
		{
			if( x == 0xC0 || x == 0xC1 )
			{
				_utfdecode_ThrowException( overlong_encoding );
				_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
			}
			
			if( _utfdecode_IsAtOrPastEndOfString(_utfdecode_CurrentLocation + 1) )
			{
				_utfdecode_ThrowException( unexpected_end_of_string );
				_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
			}

			x = ( 0b0001'1111 & x ) << 6;

			char32_t _dx1 = *(++_utfdecode_CurrentLocation);

			if( ( _dx1 & 0b1100'0000 ) != 0b1000'0000 )
			{
				_utfdecode_ThrowException( unexpected_non_continuation_byte );
				_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );
			}

			x |= _dx1 & 0b0011'1111;

			++_utfdecode_CurrentLocation;

			_utfdecode_ReturnCodepoint( x );
		}
	}
	else { _utfdecode_ThrowException( unexpected_continuation_byte ); }
}
else // No continuation byte
{
	++_utfdecode_CurrentLocation;
	_utfdecode_ReturnCodepoint(x);
}

++_utfdecode_CurrentLocation;

_utfdecode_ReturnCodepoint( _utfdecode_ReplacementChr );


#ifdef __utfdecodeUndefReplacementChr
#undef _utfdecodeReplacementChr
#endif
