/**
*
* @file: SmartPointer.h
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
#include "../config.h"
#include <memory> /* std::shared_ptr + std::unique_ptr */

_FISSION_BASE_PUBLIC_BEGIN

template <typename T>
using ref = std::shared_ptr<T>;

template <typename T>
using scoped = std::unique_ptr<T>;

template <typename T, typename... Args>
static constexpr ref<T> make_ref( Args &&...args )
{
    return std::make_shared<T>( std::forward<Args>( args )... );
}

template <typename T, typename... Args>
static constexpr scoped<T> make_scoped( Args &&...args )
{
    return std::make_unique<T>( std::forward<Args>( args )... );
}

//! @brief Managed pointer type that destroys the pointer when moves out of scope.
template <typename T>
class FPointer
{
public:
    FPointer() = default;
    FPointer( FPointer & ) = delete; /*!< no copi fo u */
    FPointer( T * _Src ) : ptr( _Src ) {};

    FPointer( FPointer && src )
    {
        ptr = src.ptr;
        src.ptr = nullptr;
    }

    ~FPointer()
    {
        if( ptr )
        {
            ptr->Destroy();
            ptr = nullptr;
        }
    }

    //! @brief Release and Get the address of.
    inline T ** operator&()
    {
        this->~FPointer();
        return &ptr;
    }

    //! @brief Get the address of underlying raw pointer.
    inline T ** address_of()
    {
        return &ptr;
    }

    inline FPointer & operator=( T * _Right )
    {
        this->~FPointer();
        ptr = _Right;
        return *this;
    }

    inline T * operator->()      { return ptr; }
    inline       T * get()       { return ptr; }
    inline const T * get() const { return ptr; }

private:
    T * ptr = nullptr;
};

template <typename T>
using fsn_ptr = FPointer<T>;

_FISSION_BASE_PUBLIC_END