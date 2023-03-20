/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 */
#pragma once
#include <Fission/config.hpp>
#include <memory>

__FISSION_BEGIN__

template <class T>
using ref = std::shared_ptr<T>;

template <class T>
using ptr = std::unique_ptr<T>;

template <typename T, typename... Args>
static constexpr ref<T> make_ref( Args &&...args ) {
    return std::make_shared<T>( std::forward<Args>( args )... );
}

template <typename T, typename... Args>
static constexpr ptr<T> make_ptr( Args &&...args ) {
    return std::make_unique<T>( std::forward<Args>( args )... );
}

//! @brief Managed pointer type that destroys the pointer when moves out of scope.
template <typename _Ty>
class fsn_ptr
{
public:
	using raw_pointer = std::add_pointer_t<_Ty>;

public:
	fsn_ptr(fsn_ptr const&) = delete;
	
    constexpr fsn_ptr() = default;
    
    constexpr fsn_ptr(raw_pointer const _Src) : ptr( _Src ) {};

    constexpr fsn_ptr(fsn_ptr && src) {
        ptr = src.ptr;
        src.ptr = nullptr;
    }

    ~fsn_ptr() {
        if( ptr ) {
            ptr->Destroy();
            ptr = nullptr;
        }
    }

    //! @brief Release and Get the address of.
    inline raw_pointer * operator&() {
        this->~fsn_ptr();
        return &ptr;
    }

    //! @brief Get the address of underlying raw pointer.
    inline raw_pointer * address_of() {
        return &ptr;
    }

    inline constexpr fsn_ptr & operator=(raw_pointer const _Right) {
        this->~fsn_ptr();
        ptr = _Right;
        return *this;
    }

    inline constexpr void swap(fsn_ptr& _Right) {
        std::swap( this->ptr, _Right.ptr );
    }

    inline constexpr raw_pointer operator->()      { return ptr; }
    inline constexpr       raw_pointer get()       { return ptr; }
    inline constexpr const raw_pointer get() const { return ptr; }

private:
    raw_pointer ptr = nullptr;

}; // class Fission::fsn_ptr<>

__FISSION_END__

/**
*	MIT License
*
*	Copyright (c) 2021-2023 lazergenixdev
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
