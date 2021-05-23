/**
*
* @file: hsv_conversions.inl
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


/*
* 
* I am not the creator of these algorithms;
* They are mostly copied from stackoverflow,
*   then further edited for my needs.
* 
* All values are assumed to be in the range: [0,1]
* 
* These macros are for code written in C,
* but they are compatable with C++
* 
* These defines don't require any dependencies.
* 
* defines: HSV_TO_RGB() and RGB_TO_HSV()
* 
* undefine when done using.
* 
*/


/* HSV_TO_RGB: Convert HSV to RGB */
#define HSV_TO_RGB( _R, _G, _B, _H, _S, _V ) {\
 	float hh, p, q, t, ff; \
 	long i; \
 	if( _S <= 0.0f ) { \
 		_R = _V; \
 		_G = _V; \
 		_B = _V; \
 		return; \
 	} \
 	hh = _H; \
 	if( hh >= 1.0f ) hh = 0.0f; \
 	hh *= 6.0f; \
 	i = (long)hh; \
 	ff = hh - i; \
 	p = _V * ( 1.0f - _S ); \
 	q = _V * ( 1.0f - _S * ff ); \
 	t = _V * ( 1.0f - _S * ( 1.0f - ff ) ); \
 	switch( i ) { \
 	case 0:  _R = _V, _G = t, _B = p; break; \
 	case 1:  _R = q, _G = _V, _B = p; break; \
 	case 2:  _R = p, _G = _V, _B = t; break; \
 	case 3:  _R = p, _G = q, _B = _V; break; \
 	case 4:  _R = t, _G = p, _B = _V; break; \
 	default: _R = _V, _G = p, _B = q; break; \
 	} }


/* RGB_TO_HSV: Convert RGB to HSV */
#define RGB_TO_HSV( _R, _G, _B, _H, _S, _V ) {\
 	float min, max, delta; \
    if( _R > _G ) \
        min = _B < _G ? _B : _G, \
        max = _B > _R ? _B : _R; \
    else \
        min = _B < _R ? _B : _R, \
        max = _B > _G ? _B : _G; \
    _V = max; \
    delta = max - min; \
    if( delta > 0.00001 ) \
        _S = delta / max; \
    else { _S = 0.0f, _H = 0.0f; return; } \
    if( _R == max ) \
        _H = (_G - _B)/delta; \
    else if( _G == max ) \
        _H = 2.0f + (_B - _R)/delta; \
    else \
        _H = 4.0f + (_R - _G)/delta; \
    _H = _H / 6.0f; \
    if( _H < 0.0f ) _H += 1.0f; }
