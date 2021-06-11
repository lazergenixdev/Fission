/**
*
* @file: Cursor.h
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

#pragma once
#include <Fission/config.h>

// todo: documentation + flesh out API

namespace Fission {

    class Cursor 
    {
    public:

        enum Default_ {
            Default_Hidden,
            Default_Arrow,
            Default_TextInput,
            Default_Hand,
            Default_Move,
            Default_Cross,
            Default_Wait,
            Default_SizeY,
            Default_SizeX,
            Default_SizeBLTR,
            Default_SizeTLBR,
        };

        struct CreateInfo {
            int not_implemented_lolxd;
        };

    public:

        // create a custom cursor
        FISSION_API static std::unique_ptr<Cursor> Create( const CreateInfo & info );

        FISSION_API static Cursor * Get( Default_ default_cursor );

        virtual bool Use() = 0;

        virtual ~Cursor() = default;

    }; // class Fission::Cursor

} // namespace Fission
