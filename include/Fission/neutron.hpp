/*
* WIP
* 
* REQUIREMENTS:
*	C++11 or later
* 
* This code is released to Public Domain. Patches and comments are welcome.
* 
* Code written by Lazergenix (2021)
* 
* contributers:
*	(none)
* 
*/


/*
*  ***********************************  API  ***********************************
* 
* Dependencies:
* 
*	`KeyType`: Key type used to represent a keycode
* 
*	`g_KeyLeftMouse`: Global variable used to represent the keycode for a Left Mouse Button
* 
*	`g_KeyRightMouse`: Global variable used to represent the keycode for a Right Mouse Button
* 
*	`CharType`: Character type used when inputing text
* 
*	(define `_neutron_no_cursor` to disable)
*	`CursorType`: Type that represent a handle to a mouse cursor
* 
*	`rect`: Struct which represents an aligned rectangle
* 
*	`point`: Struct which represents a point in 2d space
* 
*	`vector`: dynamic array
* 
* 
* 
* Core API:
* 
*	interface IEventHandler
* 
*	interface Window
*		
*		
* 
*/

#pragma once

#include <algorithm>
#include <stdexcept>

/* begin/end header macros for 3rd-party integration */
#ifndef _neutron_begin_header_
#define _neutron_begin_header_ namespace neutron {
#endif
#ifndef _neutron_end_header_
#define _neutron_end_header_ }
#endif

_neutron_begin_header_

/* 
*  config wraped in macro if you want to define the dependencies on your own, 
*  DEFINE _neutron_config_ to bypass the default implementation
*/
#ifndef _neutron_config_
#define _neutron_config_

	/* Key type that is used in Key callbacks, Default: 'int' */
	#ifndef _neutron_key_type
	#define _neutron_key_type int
	#endif
	using KeyType = _neutron_key_type;

	/* Char type that is used in Text input callbacks, Default: 'char' */
	#ifndef _neutron_char_type
	#define _neutron_char_type char
	#endif
	using CharType = _neutron_char_type;

	/* Key value that represents the primary mouse button, DEFINE this before including neutron.hpp */
	#ifndef _neutron_key_primary_mouse
	#define _neutron_key_primary_mouse 0
	#endif
	static KeyType g_KeyPrimaryMouse = _neutron_key_primary_mouse;

	/* Key value that represents the secondary mouse button, DEFINE this before including neutron.hpp */
	#ifndef _neutron_key_secondary_mouse
	#define _neutron_key_secondary_mouse 1
	#endif
	static KeyType g_KeySecondaryMouse = _neutron_key_secondary_mouse;

	/* DEFINE `_neutron_no_cursor` to disable cursors */
	#ifndef _neutron_no_cursor
	/* Cursor type that is used in SetCursor, DEFINE this before including neutron.hpp if using cursors */
	#ifndef _neutron_cursor_type
	#define NEUTRON_HAS_CURSOR 1
	#define _neutron_cursor_type void *
	#endif
	using CursorType = _neutron_cursor_type;
	#endif

	#ifndef _neutron_point_type
	#define _neutron_point_type ::neutron::_point
	struct _point {
		int x, y; 
		constexpr _point operator-( const _point & rhs ) const { return { this->x - rhs.x, this->y - rhs.y }; }
	};
	#endif
	using point = _neutron_point_type;

	#ifndef _neutron_rect_type
	#define _neutron_rect_type ::neutron::_rect
	struct _rect { 
		struct _range { 
			int low, high; 
			auto average() const { return (low+high)/2; }
		};
		_range x, y;
		constexpr bool operator[]( const point & p ) const { return ( p.x >= x.low ) && ( p.x <= x.high ) && ( p.y >= y.low ) && ( p.y <= y.high ); }
		constexpr static _rect from_topleft( const point & position, const point & size ) { return { position.x, position.x + size.x, position.y, position.y + size.y }; }
		point topLeft() const { return { x.low, y.low }; }
		point size() const { return { x.high - x.low, y.high - y.low }; }
		point clamp( const point & p ) const { point out = p; if( p.x > x.high ) out.x = x.high; else if( p.x < x.low ) out.x = x.low; if( p.y > y.high ) out.y = y.high; else if( p.y < y.low ) out.y = y.low; return out; }
		_rect & expand( int dx ) { x.low -= dx, x.high += dx; y.low -= dx, y.high += dx; return *this; }
		_rect expanded( int dx ) const { return { {x.low-dx,x.high+dx},{y.low-dx,y.high+dx} }; }

		auto top() const { return y.low; }
		auto bottom() const { return y.high; }
		auto left() const { return x.low; }
		auto right() const { return x.high; }
	};
	#endif
	using rect = _neutron_rect_type;

	#ifndef _neutron_vector_type
	#define _neutron_vector_type ::neutron::_vector
	template <typename T>
	struct _vector 
	{
		using value_type = T;
		using iterator = T*;

		iterator begin();
		iterator end();

		iterator rbegin();
		iterator rend();

		bool empty() const;
		void erase( iterator where );
		template <typename...Params>
		void emplace_back( Params&&...params ) {}
		void push_back( T & value ) {}

		value_type & back();
	};
	#endif
	template <typename T>
	using vector = _neutron_vector_type<T>;

	enum rect_pos_
	{
		rect_pos_inside,
		rect_pos_outside,
		rect_pos_left,
		rect_pos_right,
		rect_pos_top,
		rect_pos_bottom,
		rect_pos_top_left,
		rect_pos_top_right,
		rect_pos_bottom_left,
		rect_pos_bottom_right,
	};


	//
	// +--------------------+
	// | TL  |  TOP   |  TR |
	// |   +------------+   | } - Corner Tolerance
	// |---|            |---|
	// | L |            | R |
	// |---|            |---|
	// |   +------------+   |
	// | BL  | BOTTOM |  BR |  \ - Expand 
	// +--------------------+  /
	//
	static constexpr rect_pos_ GetRectPos( rect rect, const point & pos, const int & expand, const int & inflate, const int & corner_tolerance )
	{
		const auto & [x, y] = pos;

		if( rect.expanded( -inflate )[pos] )
			return rect_pos_inside;

		rect.expand( expand );

		if( !rect[pos] )
			return rect_pos_outside;

		if( x > rect.x.average() )
		{
			if( y > rect.y.average() )
			{
				if( x < ( rect.right() - expand - corner_tolerance ) )
					return rect_pos_bottom;
				if( y < ( rect.bottom() - expand - corner_tolerance ) )
					return rect_pos_right;
				return rect_pos_bottom_right;
			}
			if( x < ( rect.right() - expand - corner_tolerance ) )
				return rect_pos_top;
			if( y > ( rect.top() + expand + corner_tolerance ) )
				return rect_pos_right;
			return rect_pos_top_right;
		}

		if( y > rect.y.average() )
		{
			if( x > ( rect.left() + expand + corner_tolerance ) )
				return rect_pos_bottom;
			if( y < ( rect.bottom() - expand - corner_tolerance ) )
				return rect_pos_left;
			return rect_pos_bottom_left;
		}
		if( x > ( rect.left() + expand + corner_tolerance ) )
			return rect_pos_top;
		if( y > ( rect.top() + expand + corner_tolerance ) )
			return rect_pos_left;
		return rect_pos_top_left;
	}

	static point g_MousePosition = {0,0};
	static point GetMousePosition() noexcept { return g_MousePosition; }

	/*
	* Message Types:
	* - MouseMoveEventArgs
	* - MouseLeaveEventArgs
	* - KeyDownEventArgs
	* - KeyUpEventArgs
	* - TextInputEventArgs
	*/
	typedef struct MouseMoveEventArgs_ {
		point pos;
	} MouseMoveEventArgs;

	struct KeyDownEventArgs { KeyType key; };

	struct KeyUpEventArgs { KeyType key; };

	struct TextInputEventArgs { CharType ch; };

	#ifndef _neutron_no_cursor
	struct SetCursorEventArgs { CursorType cursor; };
	#endif

	enum Result {
		Handled = 0,
		Pass	= 1,
	};

	/* Default ui callback */
	#define _neutron_Pass_Event_ { return Pass; }

	/* Default update callback */
	#define _neutron_Empty_Expression_ {}

	/*
	* Type used for window uids
	* - Operators overloaded:
	*		==  !=  ++
	*/
	typedef struct UInt128_ {
		unsigned long long low, high;
	} window_uid;

	static constexpr bool operator==( const window_uid & lhs, const window_uid & rhs ) {
		return ( lhs.low == rhs.low ) && ( lhs.high == rhs.high );
	}
	static constexpr bool operator!=( const window_uid & lhs, const window_uid & rhs ) {
		return ( lhs.low != rhs.low ) || ( lhs.high != rhs.high );
	}
	static window_uid & operator++( window_uid & uid ) {
		if( ++uid.low == 0llu ) ++uid.high; return uid;
	}

	static constexpr window_uid null_window_uid = {0,0}; // null
	static window_uid g_NextWindow_uid = { 0x45, 0x1a4 }; // perfect

#endif /* _neutron_config_ */

/* <helpers> */
#define _neutron_bit(X) (1<<X)
/* </helpers> */


/* ============================================================================================================== */
/* ------------------------------------------------- Public API ------------------------------------------------- */
/* ============================================================================================================== */

/* <Forward Declarations> */
struct IEventHandler;	// Interface for message callbacks
struct Parent;			// Any Object which imbeds child windows
class Window;			// Represents a ui window
class DynamicWindow;	// Represents a ui window that can contain children and freely move
class WindowManager;    // Structure that handles the managing of, and communication between ui windows
class Context;          // context help text

class Widget;
class Slider;
class Button;
class TextEdit;
/* </Forward Declarations> */


struct IEventHandler {

#ifndef _neutron_no_cursor
	inline virtual Result OnSetCursor( SetCursorEventArgs & )
		_neutron_Pass_Event_
#endif

	inline virtual Result OnMouseMove( MouseMoveEventArgs & )
		_neutron_Pass_Event_

	inline virtual Result OnMouseLeave()
		_neutron_Pass_Event_

	inline virtual Result OnKeyDown( KeyDownEventArgs & )
		_neutron_Pass_Event_

	inline virtual Result OnKeyUp( KeyUpEventArgs & )
		_neutron_Pass_Event_

	inline virtual Result OnTextInput( TextInputEventArgs & )
		_neutron_Pass_Event_

	inline virtual void OnFocusGain()
		_neutron_Empty_Expression_

	inline virtual void OnParentResize( rect rc )
		_neutron_Empty_Expression_

	inline virtual void OnFocusLost()
		_neutron_Empty_Expression_

	inline virtual void OnUpdate( float dt )
		_neutron_Empty_Expression_

};

/*
* Parents MUST be able to contain child windows, 
* that is who the helper functions are for.. 
*/
struct Parent 
{
	rect Rect;

	virtual void SetCapture( Window * window ) = 0;
	virtual void SetFocus( Window * window ) = 0;
	virtual void SetHover( Window * window ) = 0;

	virtual Window * GetCapture() = 0;
	virtual Window * GetFocus() = 0;
	virtual Window * GetHover() = 0;
	
	/* release ownership of window */
	virtual void Release( Window * window ) = 0;

	inline virtual ~Parent() = default;
};

class Window : public IEventHandler
{
public:
	Parent * parent = nullptr;
public:
	Window();
	Window( const Window & src );
	void Release();
	inline virtual ~Window() = default;
	virtual bool isInside( point pos );
	const window_uid getuid() const;
private:
	window_uid uid;
};

class Widget : public Window {};

class DynamicWindow : public Window, public Parent
{
public:
	enum Flags_ {
		Flags_None = 0,
		Flags_Movable = _neutron_bit(0),
		Flags_Sizable = _neutron_bit(1),
	//	Flags_Visible = _neutron_bit(2),

		Flags_Default = Flags_Movable | Flags_Sizable,
	};
	enum State_ {
		State_Default = 0,
		State_Moving = _neutron_bit(0),

		State_SizingL = _neutron_bit(1),
		State_SizingR = _neutron_bit(2),
		State_SizingT = _neutron_bit(3),
		State_SizingB = _neutron_bit(4),
		State_Sizing = State_SizingL | State_SizingR | State_SizingT | State_SizingB,
	};

	vector<Widget *> widgets;
	Flags_ flags = Flags_Default;
	State_ state = State_Default;
	point last;
	Window * focus = nullptr;
	Window * hover = nullptr;
	Window * capture = nullptr;
public:
	DynamicWindow( rect rc, Flags_ Flags = Flags_Default );
	inline virtual ~DynamicWindow() = default;
	DynamicWindow * addWidget( Widget * widget );
	virtual void OnUpdate( float dt ) override;
	virtual bool isInside( point pos ) override;
	virtual void OnMove( point & new_pos );
	virtual void OnResize();

#ifndef _neutron_no_cursor
	Result OnSetCursor( SetCursorEventArgs & args ) override;
#endif
	virtual Result OnMouseLeave() override;
	virtual Result OnMouseMove( MouseMoveEventArgs & args ) override;
	virtual Result OnKeyDown( KeyDownEventArgs & args ) override;
	virtual Result OnKeyUp( KeyUpEventArgs & args ) override; 
	virtual Result OnTextInput( TextInputEventArgs & args ) override;

	virtual void OnFocusLost() override;

	virtual void SetCapture( Window * window ) override;
	virtual void SetFocus( Window * window ) override;
	virtual void SetHover( Window * window ) override;

	virtual Window * GetCapture() override;
	virtual Window * GetFocus() override;
	virtual Window * GetHover() override;

	void Release( Window * window ) override;
};

class Button : public Widget 
{
public:
	bool active = false;

public:
	inline virtual void OnPressed()
		_neutron_Empty_Expression_

	virtual Result OnKeyDown( KeyDownEventArgs & args ) override;
	virtual Result OnKeyUp( KeyUpEventArgs & args ) override;
};

class Slider : public Widget 
{
public:
	bool active = false;

public:
	inline virtual void OnSlide()
		_neutron_Empty_Expression_

	inline virtual void UpdateSlidePosition( point mouse )
		_neutron_Empty_Expression_

	Result OnKeyDown( KeyDownEventArgs & args ) override;
	Result OnKeyUp( KeyUpEventArgs & args ) override;
	Result OnMouseMove( MouseMoveEventArgs & args ) override;
};

class TextEdit : public Widget 
{
public:
	bool active = false;
public:
	inline virtual void OnFinishEdit()
		_neutron_Empty_Expression_

	inline virtual void OnEdit()
		_neutron_Empty_Expression_
};

class WindowManager : public IEventHandler, public Parent
{
public:
	Window * hover = nullptr;
	Window * focus = nullptr;
	Window * capture = nullptr;

	vector<Window *> windowStack;
public:
	WindowManager( int width, int height );
	WindowManager(){}
	~WindowManager();

	void Initialize( int width, int height );

	virtual void SetCapture( Window * window ) override;
	virtual void SetFocus( Window * window ) override;
	virtual void SetHover( Window * window ) override;

	virtual Window * GetCapture() override;
	virtual Window * GetFocus() override;
	virtual Window * GetHover() override;

	virtual Result OnKeyDown( KeyDownEventArgs & args ) override;
	virtual Result OnKeyUp( KeyUpEventArgs & args ) override;
	virtual Result OnTextInput( TextInputEventArgs & args ) override;
	virtual Result OnMouseMove( MouseMoveEventArgs & args ) override;
#ifndef _neutron_no_cursor
	virtual Result OnSetCursor( SetCursorEventArgs & args ) override;
#endif
	virtual Result OnMouseLeave() override;

	virtual void OnUpdate( float dt ) override;

	void bringToFront( Window * window );
	void setFocus( Window * window );
	void addWindow( Window * window );
	void Release( Window * window ) override;

	Window * findWindow( const window_uid & uid )
	{
		auto it = std::find_if( windowStack.begin(), windowStack.end(), [uid] ( const Window * w ) { return w->getuid() == uid; } );
		if( it == windowStack.end() ) return nullptr;
		return *it;
	}
};

class Context
{
public:
	Context(){}
	~Context();

public:
	WindowManager wm;
};



/* ============================================================================================================== */
/* ----------------------------------------------- Implementation ----------------------------------------------- */
/* ============================================================================================================== */

inline Window::Window() : uid( ++g_NextWindow_uid ) {}
inline Window::Window( const Window & src ) : uid( ++g_NextWindow_uid ) {}
inline void Window::Release() { if( parent ) parent->Release( this ); }
inline bool Window::isInside( point pos ) { return false; }
inline const window_uid Window::getuid() const { return uid; }


inline WindowManager::WindowManager( int width, int height ) { Rect = rect::from_topleft( {}, point{ width, height } ); }
inline WindowManager::~WindowManager() { for( auto && w : windowStack ) delete w; }

inline void WindowManager::SetCapture( Window * window ) { capture = window; }
inline void WindowManager::SetFocus( Window * window ) { if( focus ) focus->OnFocusLost(); focus = window; if( focus ) focus->OnFocusGain(); }
inline void WindowManager::SetHover( Window * window ) { if( hover ) hover->OnMouseLeave(); hover = window; }

inline Window * WindowManager::GetCapture() { return capture; };
inline Window * WindowManager::GetFocus() { return focus; };
inline Window * WindowManager::GetHover() { return hover; };

inline Result WindowManager::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyPrimaryMouse || args.key == g_KeySecondaryMouse )
	{
		if( hover ) {
			bringToFront( hover );
			SetFocus( hover );
			return hover->OnKeyDown( args );
		}
		capture = nullptr;
	}
	else if( focus ) { return focus->OnKeyDown( args ); }
	SetFocus( nullptr );
	return Pass;
}

inline Result WindowManager::OnKeyUp( KeyUpEventArgs & args ) {
	if( capture ) { return capture->OnKeyUp( args ); }
	if( focus ) { return focus->OnKeyUp( args ); }
	return Pass;
}

inline Result WindowManager::OnTextInput( TextInputEventArgs & args ) {
	if( capture ) { return capture->OnTextInput( args ); }
	if( focus ) { return focus->OnTextInput( args ); }
	return Pass;
}

inline Result WindowManager::OnMouseMove( MouseMoveEventArgs & args ) {
	g_MousePosition = args.pos;
	if( capture ) { return capture->OnMouseMove( args ); }
	// search for window to be hovered window
	for( auto ri = windowStack.rbegin(); ri != windowStack.rend(); ri++ )
	{
		auto & w = *ri;
		if( w->isInside( args.pos ) ) {
			if( hover != w ) SetHover( w );
			return w->OnMouseMove( args );
		}
	}
	hover = nullptr;
	return Pass;
}

inline Result WindowManager::OnSetCursor( SetCursorEventArgs & args ) {
	if( hover ) { return hover->OnSetCursor( args ); }
	return Pass;
}

inline Result WindowManager::OnMouseLeave() {
	if( hover ) {
		hover->OnMouseLeave();
		hover = nullptr;
		return Handled;
	}
	return Pass;
}

inline void WindowManager::OnUpdate( float dt ) {
	for( auto && w : windowStack )
		w->OnUpdate( dt );
}

inline void WindowManager::Release( Window * window ) {
	auto it = std::find( windowStack.begin(), windowStack.end(), window );
	if( it == windowStack.end() ) throw std::logic_error("this don't make no fucking sense");
	windowStack.erase( it );
	if( hover == window ) hover = nullptr;
	if( focus == window ) focus = nullptr;
	if( capture == window ) capture = nullptr;
}

inline void WindowManager::bringToFront( Window * window ) {
	auto it = std::find( windowStack.begin(), windowStack.end(), window );
	if( it == windowStack.end() ) throw std::logic_error( "this don't make no fucking sense" );
	windowStack.erase( it );
	windowStack.emplace_back( window );
}

inline void WindowManager::setFocus( Window * window ) {
	if( window != windowStack.back() )
		bringToFront( window );
	if( focus ) focus->OnFocusLost();
	focus = window;
	focus->OnFocusGain();
}

inline void WindowManager::addWindow( Window * window ) {
	window->parent = this;
	windowStack.push_back( window );
}


inline DynamicWindow::DynamicWindow( rect rc, Flags_ Flags ) : flags( Flags ) { Rect = rc; }
inline DynamicWindow * DynamicWindow::addWidget( Widget * widget ) { widget->parent = this; widgets.emplace_back( widget ); return this; }
inline void DynamicWindow::OnUpdate( float dt ) { for( auto && w : widgets ) w->OnUpdate( dt ); }
inline bool DynamicWindow::isInside( point pos ) { return Rect.expanded( 8 )[pos]; }
inline void DynamicWindow::OnMove( point & new_pos ) {}

inline void DynamicWindow::OnResize()
{
	static constexpr int min_w = 110;
	static constexpr int min_h = 80;
	switch( state & State_Sizing )
	{
	case State_SizingL: Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ); break;
	case State_SizingR: Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ); break;
	case State_SizingT: Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h ); break;
	case State_SizingB: Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h ); break;
	case ( State_SizingL | State_SizingT ):
		Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ), Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h );
	break;
	case ( State_SizingR | State_SizingT ):
		Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ), Rect.y.low = std::min( Rect.y.low, Rect.y.high - min_h );
	break;
	case ( State_SizingL | State_SizingB ):
		Rect.x.low = std::min( Rect.x.low, Rect.x.high - min_w ), Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h );
	break;
	case ( State_SizingR | State_SizingB ):
		Rect.x.high = std::max( Rect.x.high, Rect.x.low + min_w ), Rect.y.high = std::max( Rect.y.high, Rect.y.low + min_h );
	break;
	default:break;
	}
}

inline Result DynamicWindow::OnSetCursor( SetCursorEventArgs & args ) {
	if( hover ) { return hover->OnSetCursor( args ); }
	return Pass;
}

inline Result DynamicWindow::OnMouseLeave() {
	if( hover ) {
		hover->OnMouseLeave();
		hover = nullptr;
	}
	return Handled;
}

inline void DynamicWindow::OnFocusLost() { if( focus ) { focus->OnFocusLost(); focus = nullptr; } }

inline void DynamicWindow::SetCapture( Window * window ) { capture = window; parent->SetCapture( capture ? this : nullptr ); }
inline void DynamicWindow::SetFocus( Window * window ) { focus = window; }
inline void DynamicWindow::SetHover( Window * window ) { hover = window; }

inline Window * DynamicWindow::GetCapture() { return capture; }
inline Window * DynamicWindow::GetFocus() { return focus; }
inline Window * DynamicWindow::GetHover() { return hover; }

inline void DynamicWindow::Release( Window * window ) {
	auto it = std::find( widgets.begin(), widgets.end(), window );
	if( it == widgets.end() ) throw std::logic_error("this don't make no fucking sense");
	widgets.erase( it );
	if( hover == window ) hover = nullptr;
	if( focus == window ) focus = nullptr;
	if( capture == window ) capture = nullptr;
}

inline Result DynamicWindow::OnMouseMove( MouseMoveEventArgs & args ) {
	if( state & State_Moving ) {
		auto size = Rect.size();
		point p = parent->Rect.clamp( args.pos );
		OnMove( p );
		Rect = rect::from_topleft( p - last, size );
	}
	else if( state & State_Sizing ) {
		point p = parent->Rect.clamp( args.pos );
		switch( state & State_Sizing )
		{
		case State_SizingL: Rect.x.low = p.x; break;
		case State_SizingR: Rect.x.high = p.x; break;
		case State_SizingT: Rect.y.low = p.y; break;
		case State_SizingB: Rect.y.high = p.y; break;
		case (State_SizingL|State_SizingT): Rect.x.low = p.x, Rect.y.low = p.y; break;
		case (State_SizingR|State_SizingT): Rect.x.high = p.x, Rect.y.low = p.y; break;
		case (State_SizingL|State_SizingB): Rect.x.low = p.x, Rect.y.high = p.y; break;
		case (State_SizingR|State_SizingB): Rect.x.high = p.x, Rect.y.high = p.y; break;
		default:
			break;
		}
		OnResize();
		for( auto && w : widgets )
			w->OnParentResize( Rect );
	//	auto size = Rect.size();
	//	Rect = rect::from_tl( p - last, size );
	}
	else if( capture ) {
		return capture->OnMouseMove( args );
	}
	else {
		for( auto && w : widgets )
			if( w->isInside( args.pos - Rect.topLeft() ) )
			{
				if( hover ) hover->OnMouseLeave();
				hover = w;
				w->OnMouseMove( args );
				return Handled;
			}
		if( hover ) hover->OnMouseLeave();
		hover = nullptr;
	}
	return Handled;
}

inline Result DynamicWindow::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyPrimaryMouse || args.key == g_KeySecondaryMouse ) {
		if( hover ) {
			if( focus ) focus->OnFocusLost();
			focus = hover;
			focus->OnFocusGain();
			hover->OnKeyDown( args );
			return Handled;
		}
	}
	else if( focus ) {
		focus->OnKeyDown( args );
		return Handled;
	}
	if( args.key == g_KeyPrimaryMouse && flags & Flags_Movable ) {
		auto P = GetRectPos( Rect, g_MousePosition, 8, 0, 8 );
		switch( P )
		{
		case rect_pos_inside:
		{
			(int &)state |= State_Moving;
			break;
		}
		case rect_pos_left:			(int &)state |= State_SizingL; break;
		case rect_pos_right:		(int &)state |= State_SizingR; break;
		case rect_pos_top:			(int &)state |= State_SizingT; break;
		case rect_pos_bottom:		(int &)state |= State_SizingB; break;
		case rect_pos_top_left:		(int &)state |= State_SizingL|State_SizingT; break;
		case rect_pos_top_right:	(int &)state |= State_SizingR|State_SizingT; break;
		case rect_pos_bottom_left:	(int &)state |= State_SizingL|State_SizingB; break;
		case rect_pos_bottom_right:	(int &)state |= State_SizingR|State_SizingB; break;
		default:break;
		}
		last = GetMousePosition() - Rect.topLeft();
		parent->SetCapture( this );
	}
	if( focus ) {
		focus->OnFocusLost();
		focus = nullptr;
	}
	return Handled;
}

inline Result DynamicWindow::OnKeyUp( KeyUpEventArgs & args ) {
	if( focus )
		return focus->OnKeyUp( args );
	else if( args.key == g_KeyPrimaryMouse ) {
		(int &)state &=~ (State_Sizing|State_Moving);
		parent->SetCapture( nullptr );
	}
	return Handled;
}

inline Result DynamicWindow::OnTextInput( TextInputEventArgs & args ) {
	if( focus ) return focus->OnTextInput( args );
	return Pass;
}

inline Result Button::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyPrimaryMouse ) {
		active = true;
		return Handled;
	}
	return Pass;
}

inline Result Button::OnKeyUp( KeyUpEventArgs & args ) {
	if( g_KeyPrimaryMouse == args.key ) {
		if( parent->GetHover() == this ) OnPressed();
		active = false;
		return Handled;
	}
	return Pass;
}

inline Result Slider::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyPrimaryMouse ) {
		parent->SetCapture( this );
		active = true;
		UpdateSlidePosition( GetMousePosition() );
		return Handled;
	}
	return Pass;
}

inline Result Slider::OnKeyUp( KeyUpEventArgs & args ) {
	if( g_KeyPrimaryMouse == args.key ) {
		parent->SetCapture( nullptr );
		active = false;
		return Handled;
	}
	return Pass;
}

inline Result Slider::OnMouseMove( MouseMoveEventArgs & args ) {
	if( active ) {
		UpdateSlidePosition( args.pos );
		return Handled;
	}
	return Pass;
}

_neutron_end_header_