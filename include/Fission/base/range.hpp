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
 * @License:      MIT (see end of file)
 */
#pragma once
#include <Fission/base/types.hpp>

FISSION_NAMESPACE_BEGIN


template <typename type>
struct range
{
	type low;  // lower bound of the range
	type high; // upper bound of the range


	//! @brief Create null range: {0,0}.
	constexpr range():low(static_cast<type>(0)),high(static_cast<type>(0)){}

	//! @brief Create range from a single value. [0, n]
	constexpr explicit range(type const& n):low(static_cast<type>(0)),high(n){}

	//! @brief Create range from a low and high value, where lo < hi.
	constexpr range(type const& lo,type const& hi):low(lo),high(hi){}

	//! @brief Converts range from another type.
	template <typename from> explicit
	constexpr range(range<from> const& r):low(static_cast<type>(r.low)),high(static_cast<type>(r.high)){}

	//! @brief Creates valid range from two values A and B.
	static inline constexpr range create(type const& A,type const& B){return(A>B)?range(B,A):range(A,B);}

	//! @brief Creates valid range from center and span.
	static inline constexpr range from_center(type const&center,type const&span){type d=span/static_cast<type>(2);return range(center-d, center+d);}


	// Range Functions

	//!! @brief Clamps a value to this range.
	inline constexpr type clamp(type const&x)const{if(x<this->low)return this->low;if(x>this->high)return this->high;return x;}

	//! @brief Checks whether value is contained within open range: ( low, high ).
	inline constexpr bool operator()(type const&x)const{return(x>this->low)&&(x<this->high);}

	//! @brief Checks whether value is contained within closed range: [ low, high ].
	inline constexpr bool operator[](type const&x)const{return(x>=this->low)&&(x<=this->high);}

	//! @brief Checks whether value is contained within ( low, high ].
	inline constexpr bool closed_upper(type const&x)const{return(x>this->low)&&(x<=this->high);}
	
	//! @brief Checks whether value is contained within [ low, high ).
	inline constexpr bool closed_lower(type const&x)const{return(x>=this->low)&&(x<this->high);}

	//! @brief Get the distance between the two end points of this range.
	inline constexpr type distance()const{return this->high-this->low;}
	inline constexpr type difference()const{return this->high-this->low;}

	//! @brief Get the value of the center.
	inline constexpr type center()const{return(this->high+this->low)/static_cast<type>(2);}
	inline constexpr type average()const{return(this->high+this->low)/static_cast<type>(2);}

	//! @brief Determine whether the high value is greater than or equal to the low value.
	//! @note foreach loops will only function when high >= low (unless a negative step value is used)
	NO_DISCARD inline constexpr bool valid()const{return this->high>=this->low;}


	// Modification Functions
	
	//! @brief Get an Expanded range.
	inline constexpr auto expanded(type const&_dx)const{return range(this->low-_dx,this->high+_dx);}

	//! @brief Expands this range
	inline constexpr auto&expand(type const&_dx){this->low-=_dx,this->high+=_dx;return*this;}

	//! @brief Get a Scaled range from center.
	inline constexpr auto scaled(type const&scale)const{
		auto center=(this->high+this->low)/static_cast<type>(2),
			d=scale*(this->high-this->low)/static_cast<type>(2); 
		return range(center-d,center+d);
	}

	//! @brief Scales this range from center.
	inline constexpr auto&scale(type const&scale){
		auto center=(this->high+this->low)/static_cast<type>(2),
			d=scale*(this->high-this->low)/static_cast<type>(2); 
		this->low=center-d,this->high=center+d;return*this;
	}

	// Extras
private:

	struct range_iterator
	{
		type value;

		constexpr bool operator!=(range_iterator const& r)const{return value<r.value;}
		constexpr type operator*()const{return value;}
		constexpr range_iterator& operator++(){++value;return*this;}
	};

	struct step_range_iterator
	{
		type value, step;

		constexpr bool operator!=(range_iterator const& r)const{return value<r.value;}
		constexpr type operator*()const{return value;}
		constexpr step_range_iterator& operator++(){value+=step;return*this;}
	};

	struct stepped_range
	{
		type low, high, step;

		inline constexpr auto begin()const{return step_range_iterator{low,step};}
		inline constexpr auto end()const{return range_iterator{high};}
	};

public:

	inline constexpr auto step(type const& step)const{return stepped_range{this->low, this->high, step};}

	inline constexpr auto begin()const{return range_iterator{this->low};}
	inline constexpr auto end()const{return range_iterator{this->high};}


	// Operators

	/*! @brief Check if the ranges are equal. */
	constexpr bool operator==(range const&) const = default;

	//! @brief Get a range scaled from zero.
	constexpr auto operator*(type const&scale)const{return range(this->low*scale,this->high*scale);}

	//! @brief Scale this range from zero.
	constexpr auto&operator*=(type const&scale){this->low*=scale,this->high*=scale;return*this;}
	constexpr auto&operator/=(type const&scale){this->low/=scale,this->high/=scale;return*this;}

	// vvv Shift Operators vvv

	constexpr auto operator+(type const&shift)const{return range(this->low+shift,this->high+shift);}
	constexpr auto operator-(type const&shift)const{return range(this->low-shift,this->high-shift);}

	constexpr auto&operator+=(type const&shift){this->low+=shift,this->high+=shift;return*this;}
	constexpr auto&operator-=(type const&shift){this->low-=shift,this->high-=shift;return*this;}

}; // struct Fission::range

_FISSION_BASE_ALIASES(range, range);

//! @return The minimum distance between two ranges.
//! @note: Returns a negative value for ranges that contain a subset of each other.
template <typename T>
inline constexpr auto intersect(range<T> const& a, range<T> const& b)
{
    auto p = b.low - a.high, k = a.low - b.high;
    return (p>k)? p:k;
}


/// TODO: measure performance of this VS keeping an index locally
template <typename iterable>
struct enumerate
{
	template <typename T>
	struct indexed_value {
		size_t index;
		T value;
	};
private:
	template <typename parent>
	struct iterator
	{
		explicit constexpr iterator(parent const& p): it(p){}

		constexpr bool operator!=(iterator r) const { return it != r.it; }
		constexpr auto operator++() { ++i; return ++it; }
		constexpr auto operator*() const { return indexed_value<decltype(*it)>{i, *it}; }

        parent it;
		size_t i = 0;
	};

public:
	enumerate() = delete;
	explicit enumerate(iterable const& object): obj(object) {}

	constexpr auto begin() { return iterator<decltype(obj.begin())>{obj.begin()}; }
	constexpr auto end()   { return iterator<decltype(obj.end())>  {obj.end()}; }

private:
    iterable obj;
};

FISSION_NAMESPACE_END

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2025 lazergenixdev
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
