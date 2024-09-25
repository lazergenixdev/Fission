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
#include <Fission/base/math/vector.hpp>
#include <Fission/base/range.hpp>

__FISSION_BEGIN__

//! @brief Structure defining a rectangle containing min and mox for X and Y.
template <typename T>
struct rect
{
	using type = T;
	using vector = math::vector2<T>;

	range<T> x, y;


	constexpr rect(rect const&) = default;

	//! @brief Create a null rect.
	constexpr rect()noexcept:x(),y(){}

	//! @brief Create rect from 4 values: left, right, top, and bottom.
	constexpr rect(type const&x_low,type const&x_high,type const&y_low,type const&y_high):x(x_low,x_high),y(y_low,y_high){}
		
	//! @brief Create rect from two ranges, for the X range and Y range.
	constexpr rect(const range<T>&X,const range<T>&Y):x(X),y(Y){}

	//! @brief Create a valid rect from two vectors.
	constexpr rect(vector const&a,vector const&b):x(range<T>::create(a.x,b.x)),y(range<T>::create(a.y,b.y)){}

	//! @brief Create rect from another rect with a different type.
	template <typename from> explicit
	constexpr rect(rect<from> const& r):x(static_cast<range<T>>(r.x)),y(static_cast<range<T>>(r.y)){}


	//! @brief Create a rect from a top-left position and a size.
	static inline constexpr rect from_topleft(type const&left,type const&top,type const&width,type const&height)
	{
		return rect(left,left+width,top,top+height);
	}
	static inline constexpr rect from_topleft(type const&left,type const&top,vector const&size)
	{
		return rect(left,left+size.x,top,top+size.y);
	}
	static inline constexpr rect from_topleft(vector const&topleft,type const&width,type const&height)
	{
		return rect(topleft.x,topleft.x+width,topleft.y,topleft.y+height);
	}
	static inline constexpr rect from_topleft(vector const&topleft,vector const&size)
	{
		return rect(topleft.x,topleft.x+size.x,topleft.y,topleft.y+size.y);
	}
	static inline constexpr rect from_topleft(type const&width,type const&height)
	{
		return rect(static_cast<type>(0),width,static_cast<type>(0),height);
	}
	static inline constexpr rect from_topleft(vector const&size)
	{
		return rect(static_cast<type>(0),size.x,static_cast<type>(0),size.y);
	}


	//! @brief Create a rect from an center position and a size.
	static inline constexpr rect from_center(type const&center_x,type const&center_y,type const&width,type const&height)
	{
		const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
		return rect(center_x-dx,center_x+dx,center_y-dy,center_y+dy);
	}
	static inline constexpr rect from_center(type const&center_x,type const&center_y,vector const&size)
	{
		const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
		return rect(center_x-dx,center_x+dx,center_y-dy,center_y+dy);
	}
	static inline constexpr rect from_center(vector const&center,type const&width,type const&height)
	{
		const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
		return rect(center.x-dx,center.x+dx,center.y-dy,center.y+dy);
	}
	static inline constexpr rect from_center(vector const&center,vector const&size)
	{
		const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
		return rect(center.x-dx,center.x+dx,center.y-dy,center.y+dy);
	}
	static inline constexpr rect from_center(type const& width,type const& height)
	{
		const auto dx = width / static_cast<type>(2), dy = height / static_cast<type>(2);
		return rect(-dx,dx,-dy,+dy);
	}
	static inline constexpr rect fromcenter(vector const&size)
	{
		const auto dx = size.x / static_cast<type>(2), dy = size.y / static_cast<type>(2);
		return rect(-dx,dx,-dy,+dy);
	}


	//! @brief Create a rect from a Windows RECT.
	template <typename win32_rect>
	static inline constexpr rect from_win32(win32_rect const& r)
	{
		return {r.left, r.right, r.top, r.bottom};
	}

	// Getters

	inline constexpr auto left()  const{return this->x.low ;}
	inline constexpr auto right() const{return this->x.high;}
	inline constexpr auto top()   const{return this->y.low ;}
	inline constexpr auto bottom()const{return this->y.high;}

	inline constexpr auto top_left() const{return vector(this->x.low, this->y.low );}
	inline constexpr auto top_right()const{return vector(this->x.high,this->y.low );}
	inline constexpr auto bot_left() const{return vector(this->x.low, this->y.high);}
	inline constexpr auto bot_right()const{return vector(this->x.high,this->y.high);}

	inline constexpr auto width()const{return this->x.high-this->x.low;}
	inline constexpr auto height()const{return this->y.high-this->y.low;}
	inline constexpr auto size()const{return vector(this->x.high-this->x.low,this->y.high-this->y.low);}
	inline constexpr auto center()const{auto c=vector(this->x.low+this->x.high,this->y.low+this->y.high);return c/static_cast<type>(2);}


	// Modification Functions

	//! @brief Get a rect where the X and Y values are fliped.
	constexpr rect fliped(){return rect(this->y,this->x);}

	//! @brief flips the X and Y ranges
	constexpr rect & flip() { 
		std::swap(this->x,this->y);
		return *this;
	}

	//! @brief Get a rect that is expanded in all directions
	constexpr rect expanded(type const&expand)const{return rect(
		this->x.low-expand,this->x.high+expand,this->y.low-expand,this->y.high+expand
	);}

	//! @brief Expand this rect in all directions
	constexpr rect expand(type const&expand){
		this->x.low-=expand,this->y.low-=expand;
		this->x.high+=expand,this->y.high+=expand;
		return *this;
	}

	//! @brief Get a rect that is Scaled from the center
	constexpr rect scaled(type const& scale)const{
		return rect(this->y.scaled(scale),this->y.scaled(scale));
	}

	//! @brief Scale this rect from the center
	constexpr rect scale(type const& scale){
		auto c=center(),
             d=scale*vector(this->x.high-this->x.low,this->y.high-this->y.low)/static_cast<type>(2);
		this->x.low =c.x-d.x,this->y.low =c.y-d.y;
		this->x.high=c.x+d.x,this->y.high=c.y+d.y;
		return *this;
	}

	//! @brief Shift by an offset vector
	inline constexpr rect operator+(vector const& offset)const{return {this->x.low+offset.x,this->x.high+offset.x,this->y.low+offset.y,this->y.high+offset.y}; }

	// Rect Functions

	constexpr bool operator()(vector const&p)const{return x(p.x)&&y(p.y);}
	constexpr bool operator[](vector const&p)const{return x[p.x]&&y[p.y];}
	
	constexpr bool operator[](rect const& r)const{return x[r.x.low]&&x[r.x.high]&&y[r.y.low]&&y[r.y.high];}

	//! @brief Check if position is within [xmin,xmax) and [ymin,ymax).
	constexpr bool closed_lower(vector const&p)const{return x.closed_lower(p.x)&&y.closed_lower(p.y);}

	//! @brief Check if position is within (xmin,xmax] and (ymin,ymax].
	constexpr bool closed_upper(vector const&p)const{return x.closed_upper(p.x)&&y.closed_upper(p.y);}

	inline constexpr auto clamp(vector const& p)const{return vector(x.clamp(p.x),y.clamp(p.y));}

	inline constexpr rect operator*(type   const& r)const{return {this->x.low*r,this->x.high*r,this->y.low*r,this->y.high*r};}
	inline constexpr rect operator*(vector const& r)const{return {this->x.low*r.x,this->x.high*r.x,this->y.low*r.y,this->y.high*r.y};}

	inline constexpr rect operator/(type   const& r)const{return {this->x.low/r,this->x.high/r,this->y.low/r,this->y.high/r};}
	inline constexpr rect operator/(vector const& r)const{return {this->x.low/r.x,this->x.high/r.x,this->y.low/r.y,this->y.high/r.y};}

	NO_DISCARD inline constexpr bool valid()const{return x.valid()&&y.valid();}

	constexpr bool operator==(rect const&) const = default;

}; // struct Fission::rect

_FISSION_BASE_ALIASES(rect, r);

__FISSION_END__

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
