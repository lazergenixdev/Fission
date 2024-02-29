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
#include <Fission/Base/Math/Vector.hpp>
#include <Fission/Base/Range.hpp>

__FISSION_BEGIN__

//! @TODO: remove this garbage that never gets used
// so many ways to represent a rectangle,
// just pick one you little shit!
typedef enum rect_layout_ {
	rect_layout_LRTB, // {left,right,top,bottom}
	rect_layout_LTRB, // {left,top,right,bottom}
	rect_layout_LTWH, // {left,top,width,height}
} rect_Layout;

//! @brief Structure defining a rectangle containing min and mox for X and Y.
template <typename T>
struct rect
{
	using type = T;
	using vector = math::vector2<T>;

	range<T> x, y;


	constexpr rect(const rect&src) = default;

	//! @brief Create a null rect.
	constexpr rect()noexcept:x(),y(){}

	//! @brief Create rect from 4 values: left, right, top, and bottom.
	constexpr rect(const type&_Low_X,const type&_High_X,const type&_Low_Y,const type&_High_Y):x(_Low_X,_High_X),y(_Low_Y,_High_Y){}
		
	//! @brief Create rect from two ranges, for the X range and Y range.
	constexpr rect(const range<T>&_X_Range,const range<T>&_Y_Range):x(_X_Range),y(_Y_Range){}

	//! @brief Create a valid rect from two vectors.
	constexpr rect(const vector&_A,const vector&_B):x(range<T>::create(_A.x,_B.x)),y(range<T>::create(_A.y,_B.y)){}

	//! @brief Create rect from another rect with a different type.
	template <typename _From> explicit
	constexpr rect(const rect<_From>&_Src):x(static_cast<range<T>>(_Src.x)),y(static_cast<range<T>>(_Src.y)){}


	//! @brief Create a rect from a top-left position and a size.
	static inline constexpr rect from_topleft(const type&_Left,const type&_Top,const type&_Width,const type&_Height)
	{
		return rect(_Left,_Left+_Width,_Top,_Top+_Height);
	}
	static inline constexpr rect from_topleft(const type&_Left,const type&_Top,const vector&_Size_Vector)
	{
		return rect(_Left,_Left+_Size_Vector.x,_Top,_Top+_Size_Vector.y);
	}
	static inline constexpr rect from_topleft(const vector&_TopLeft_Vector,const type&_Width,const type&_Height)
	{
		return rect(_TopLeft_Vector.x,_TopLeft_Vector.x+_Width,_TopLeft_Vector.y,_TopLeft_Vector.y+_Height);
	}
	static inline constexpr rect from_topleft(const vector&_TopLeft_Vector,const vector&_Size_Vector)
	{
		return rect(_TopLeft_Vector.x,_TopLeft_Vector.x+_Size_Vector.x,_TopLeft_Vector.y,_TopLeft_Vector.y+_Size_Vector.y);
	}
	static inline constexpr rect from_topleft(const type&_Width,const type&_Height)
	{
		return rect(static_cast<type>(0),_Width,static_cast<type>(0),_Height);
	}
	static inline constexpr rect from_topleft(const vector&_Size_Vector)
	{
		return rect(static_cast<type>(0),_Size_Vector.x,static_cast<type>(0),_Size_Vector.y);
	}


	//! @brief Create a rect from an center position and a size.
	static inline constexpr rect from_center(const type&_Center_X,const type&_Center_Y,const type&_Width,const type&_Height)
	{
		const auto dx = _Width / static_cast<type>(2), dy = _Height / static_cast<type>(2);
		return rect(_Center_X-dx,_Center_X+dx,_Center_Y-dy,_Center_Y+dy);
	}
	static inline constexpr rect from_center(const type&_Center_X,const type&_Center_Y,const vector&_Size_Vector)
	{
		const auto dx = _Size_Vector.x / static_cast<type>(2), dy = _Size_Vector.y / static_cast<type>(2);
		return rect(_Center_X-dx,_Center_X+dx,_Center_Y-dy,_Center_Y+dy);
	}
	static inline constexpr rect from_center(const vector&_Center_Vector,const type&_Width,const type&_Height)
	{
		const auto dx = _Width / static_cast<type>(2), dy = _Height / static_cast<type>(2);
		return rect(_Center_Vector.x-dx,_Center_Vector.x+dx,_Center_Vector.y-dy,_Center_Vector.y+dy);
	}
	static inline constexpr rect from_center(const vector&_Center_Vector,const vector&_Size_Vector)
	{
		const auto dx = _Size_Vector.x / static_cast<type>(2), dy = _Size_Vector.y / static_cast<type>(2);
		return rect(_Center_Vector.x-dx,_Center_Vector.x+dx,_Center_Vector.y-dy,_Center_Vector.y+dy);
	}
	static inline constexpr rect from_center(const type&_Width,const type&_Height)
	{
		const auto dx = _Width / static_cast<type>(2), dy = _Height / static_cast<type>(2);
		return rect(-dx,dx,-dy,+dy);
	}
	static inline constexpr rect from_center(const vector&_Size_Vector)
	{
		const auto dx = _Size_Vector.x / static_cast<type>(2), dy = _Size_Vector.y / static_cast<type>(2);
		return rect(-dx,dx,-dy,+dy);
	}


	//! @brief Create a rect from a Windows RECT.
	template <typename _Win_RectTpe>
	static inline constexpr rect from_win32(const _Win_RectTpe &_Win_Rect)
	{
		return rect(_Win_Rect.left, _Win_Rect.right, _Win_Rect.top, _Win_Rect.bottom);
	}

	//! @brief Convert a rect to another rect type.
	template <typename _RectTpe, rect_Layout _Layout = rect_layout_LTRB>
	inline constexpr _RectTpe as()
	{
		if constexpr (_Layout == rect_layout_LRTB)
			return _RectTpe{this->x.low,this->x.high,this->y.low,this->y.high};
		if constexpr (_Layout == rect_layout_LTRB)
			return _RectTpe{this->x.low,this->y.low,this->x.high,this->y.high};
		if constexpr (_Layout == rect_layout_LTWH)
			return _RectTpe{this->x.low,this->y.low,this->x.high-this->x.low,this->y.high-this->y.high};
	}

	// Getters

	inline constexpr auto left()  const{return this->x.low ;}
	inline constexpr auto right() const{return this->x.high;}
	inline constexpr auto top()   const{return this->y.low ;}
	inline constexpr auto bottom()const{return this->y.high;}

	inline constexpr auto topLeft() const{return vector(this->x.low, this->y.low );}
	inline constexpr auto topRight()const{return vector(this->x.high,this->y.low );}
	inline constexpr auto botLeft() const{return vector(this->x.low, this->y.high);}
	inline constexpr auto botRight()const{return vector(this->x.high,this->y.high);}

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
	constexpr rect expanded(const type&_Expand)const{return rect(
		this->x.low-_Expand,this->x.high+_Expand,this->y.low-_Expand,this->y.high+_Expand
	);}

	//! @brief Expand this rect in all directions
	constexpr rect expand(const type&_Expand){
		this->x.low-=_Expand,this->y.low-=_Expand;
		this->x.high+=_Expand,this->y.high+=_Expand;
		return *this;
	}

	//! @brief Get a rect that is Scaled from the center
	constexpr rect scaled(const type&_Scale)const{
		return rect(this->y.scaled(_Scale),this->y.scaled(_Scale));
	}

	//! @brief Scale this rect from the center
	constexpr rect scale(const type&_Scale){
		auto _Center=center(),
				_Delta=_Scale*vector(this->x.high-this->x.low,this->y.high-this->y.low)/static_cast<type>(2); 
		this->x.low =_Center.x-_Delta.x,this->y.low =_Center.y-_Delta.y;
		this->x.high=_Center.x+_Delta.x,this->y.high=_Center.y+_Delta.y;
		return *this;
	}

	//! @brief Shift by an offset vector
	inline constexpr auto operator+(const vector& _Offset)const{return rect(this->x.low+_Offset.x,this->x.high+_Offset.x,this->y.low+_Offset.y,this->y.high+_Offset.y); }

	// Rect Functions

	constexpr bool operator()(const vector&_Position)const{return x(_Position.x)&&y(_Position.y);}
	constexpr bool operator[](const vector&_Position)const{return x[_Position.x]&&y[_Position.y];}
	
	constexpr bool operator[](const rect&_Rect)const{return x[_Rect.x.low]&&x[_Rect.x.high]&&y[_Rect.y.low]&&y[_Rect.y.high];}

	//! @brief Check if position is within [xmin,xmax) and [ymin,ymax).
	constexpr bool closed_lower(const vector&_Position)const{return x.closed_lower(_Position.x)&&y.closed_lower(_Position.y);}

	//! @brief Check if position is within (xmin,xmax] and (ymin,ymax].
	constexpr bool closed_upper(const vector&_Position)const{return x.closed_upper(_Position.x)&&y.closed_upper(_Position.y);}

	inline constexpr auto clamp(const vector&_Vector)const{return vector(x.clamp(_Vector.x),y.clamp(_Vector.y));}

	inline constexpr auto operator*(const type&_Right)const{return rect(this->x.low*_Right,this->x.high*_Right,this->y.low*_Right,this->y.high*_Right);}
	inline constexpr auto operator*(const vector&_Right)const{return rect(this->x.low*_Right.x,this->x.high*_Right.x,this->y.low*_Right.y,this->y.high*_Right.y);}

	inline constexpr auto operator/(const type&_Right)const{return rect(this->x.low/_Right,this->x.high/_Right,this->y.low/_Right,this->y.high/_Right);}
	inline constexpr auto operator/(const vector&_Right)const{return rect(this->x.low/_Right.x,this->x.high/_Right.x,this->y.low/_Right.y,this->y.high/_Right.y);}

	inline constexpr bool valid()const{return x.valid()&&y.valid();}

	constexpr bool operator==(rect const&) const = default;

}; // struct Fission::rect

__FISSION_BASE_ALIASES__(rect, r);

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
