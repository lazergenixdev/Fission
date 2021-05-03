/*
* I have searched the internet for a UI api that is not attached to any specific
* rendering API, and I just turned up empty handed. So this software aims to do that, 
* the purpose is to create the structural foundation for any application which 
* wishes to implement it's own UI; there are no dependences needed and no build systems,
* only a single header file, simply include this to build into your project.
* 
* This is a WIP, and I'm considering rewriting this API in C.
* 
* REQUIREMENTS:
*	C++17 or later
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
*	(define `_lazer_ui_no_cursor` to disable)
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

/* begin/end header macros for 3rd-party integration */
#ifndef _lazer_ui_begin_header_
#define _lazer_ui_begin_header_ namespace lazer { namespace ui {
#endif
#ifndef _lazer_ui_end_header_
#define _lazer_ui_end_header_ } }
#endif

_lazer_ui_begin_header_

/* 
*  config wraped in macro if you want to define the dependencies on your own, 
*  DEFINE _lazer_ui_config_ to bypass the default implementation
*/
#ifndef _lazer_ui_config_
#define _lazer_ui_config_

	/* Key type that is used in Key callbacks, Default: 'int' */
	#ifndef _lazer_key_type
	#define _lazer_key_type int
	#endif
	using KeyType = _lazer_key_type;

	/* Char type that is used in Text input callbacks, Default: 'char' */
	#ifndef _lazer_char_type
	#define _lazer_char_type char
	#endif
	using CharType = _lazer_char_type;

	/* Key value that represents the left mouse button, DEFINE this before including lazerui.h */
	#ifndef _lazer_key_left_mouse_
	#define _lazer_key_left_mouse_ 0
	#endif
	static KeyType g_KeyLeftMouse = _lazer_key_left_mouse_;

	/* Key value that represents the right mouse button, DEFINE this before including lazerui.h */
	#ifndef _lazer_key_right_mouse_
	#define _lazer_key_right_mouse_ 1
	#endif
	static KeyType g_KeyRightMouse = _lazer_key_right_mouse_;

	/* DEFINE `_lazer_ui_no_cursor` to disable cursors */
	#ifndef _lazer_ui_no_cursor
	/* Cursor type that is used in SetCursor, DEFINE this before including lazerui.h if using cursors */
	#ifndef _lazer_cursor_type
	#define _lazer_cursor_type void *
	#endif
	using CursorType = _lazer_cursor_type;
	#endif

#ifndef _lazer_has_alloc_
	//! @note WIP
	//! @brief Memory manager that ensures all memory allocated for ui
	//!			components is packed close together for optimal performance.
	//! 
	//! @note context assumes that less than 2 GB will be allocated for UI components.
	//! @warning NOT THREAD SAFE, use mutex to ensure no corruption.
	class context
	{
	public:
		using byte = unsigned char;

		context( unsigned int nByteCount = 6'400'000u, unsigned int nBlockSize = 64u ) :
			m_pBeginBlock( operator new( nByteCount + (nByteCount / nBlockSize)*sizeof( block_entry ), std::align_val_t(64) ) )
		{
			m_pBeginIndex = (block_entry *)( (byte *)m_pBeginBlock + nByteCount );
			m_pEndIndex = (block_entry *)( (byte *)m_pBeginIndex + ( nByteCount / nBlockSize ) * sizeof( block_entry ) );

			m_pFreeBegin = m_pBeginBlock;
			m_pFreeEnd = (byte *)m_pBeginBlock + nByteCount;
		}

		template <typename T, typename...P>
		T * alloc( P&&...params ) { 

			/* Initialize Object and Adjust next free */
			T * out = new ( m_pFreeBegin ) T( params... );
			(byte*)m_pFreeBegin += sizeof( T );
			return out; 
		}

		void free( void * p ) {}

	private:
		struct block_entry { unsigned int nBytes, flags = 0; };

		/* Block Data */
		void * m_pBeginBlock;

		/* Memory Index */
		block_entry * m_pBeginIndex;
		block_entry * m_pEndIndex;

		/* Allocation Optimizations */
		void * m_pFreeBegin, * m_pFreeEnd; // Smallest range that contains all free blocks
	};
#endif

#ifndef _lazer_has_point
	struct int2 { 
		int x, y; 
		int2 operator-( const int2 & rhs ) const { return { x - rhs.x, y - rhs.y }; }
	};
	using point = int2;
#endif

#ifndef _lazer_has_rect
	struct range2 { 
		int xlow, xhigh, ylow, yhigh; 
		bool operator[]( const point & p ) const { return ( p.x >= xlow ) && ( p.x <= xhigh ) && ( p.y >= ylow ) && ( p.y <= yhigh ); }
		static range2 from_tl( const point & position, const point & size ) { return { position.x, position.x + size.x, position.y, position.y + size.y }; }
		point get_tl() const { return { xlow, ylow }; }
		point size() const { return { xhigh - xlow, yhigh - ylow }; }
		point clamp( const point & p ) const { point out = p; if( p.x > xhigh ) out.x = xhigh; else if( p.x < xlow ) out.x = xlow; if( p.y > yhigh ) out.y = yhigh; else if( p.y < ylow ) out.y = ylow; return out; }
	};
	using rect = range2;
#endif

#ifndef _lazer_has_vector
	template <typename T>
	struct basevector 
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
	template <typename T> 
	using vector = basevector<T>;
#endif

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
	rect_pos_ GetRectPos( rect rect, const point & pos, const int & expand, const int & inflate, const int & corner_tolerance )
	{
		const auto & [x, y] = pos;

		if( rect.get_expanded( -inflate )[pos] )
			return rect_pos_inside;

		rect.expand( expand );

		if( !rect[pos] )
			return rect_pos_outside;

		if( x > rect.x.get_average() )
		{
			if( y > rect.y.get_average() )
			{
				if( x < ( rect.get_r() - expand - corner_tolerance ) )
					return rect_pos_bottom;
				if( y < ( rect.get_b() - expand - corner_tolerance ) )
					return rect_pos_right;
				return rect_pos_bottom_right;
			}
			if( x < ( rect.get_r() - expand - corner_tolerance ) )
				return rect_pos_top;
			if( y > ( rect.get_t() + expand + corner_tolerance ) )
				return rect_pos_right;
			return rect_pos_top_right;
		}

		if( y > rect.y.get_average() )
		{
			if( x > ( rect.get_l() + expand + corner_tolerance ) )
				return rect_pos_bottom;
			if( y < ( rect.get_b() - expand - corner_tolerance ) )
				return rect_pos_left;
			return rect_pos_bottom_left;
		}
		if( x > ( rect.get_l() + expand + corner_tolerance ) )
			return rect_pos_top;
		if( y > ( rect.get_t() + expand + corner_tolerance ) )
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

	#ifndef _lazer_ui_no_cursor
	struct SetCursorEventArgs { CursorType cursor; };
	#endif

	enum Result {
		Handled = 0,
		Pass	= 1,
	};

	/* Default ui callback */
	#define _lazer_ui_Pass_Event_ { return Pass; }

	/* Default update callback */
	#define _lazer_ui_Empty_Expression_ {}

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

	static window_uid g_NextWindow_uid = { 0x45, 0x1a4 }; // perfect

#endif /* _lazer_ui_config_ */

/* <helpers> */
#define _lazer_ui_bit(X) (1<<X)
/* </helpers> */


/* ============================================================================================================== */
/* ------------------------------------------------- Public API ------------------------------------------------- */
/* ============================================================================================================== */

/* <Forward Declarations> */
struct IEventHandler;	// Interface for message callbacks
struct Parent;			// Any Object which imbeds child windows
class Window;			// Represents a ui window
class DynamicWindow;	// Represents a ui window that can contain children and freely move
class WindowManager;

class Widget;
class Slider;
class Button;
class TextEdit;
/* </Forward Declarations> */


struct IEventHandler {

#ifndef _lazer_ui_no_cursor
	inline virtual Result OnSetCursor( SetCursorEventArgs & )
		_lazer_ui_Pass_Event_
#endif

	inline virtual Result OnMouseMove( MouseMoveEventArgs & )
		_lazer_ui_Pass_Event_

	inline virtual Result OnMouseLeave()
		_lazer_ui_Pass_Event_

	inline virtual Result OnKeyDown( KeyDownEventArgs & )
		_lazer_ui_Pass_Event_

	inline virtual Result OnKeyUp( KeyUpEventArgs & )
		_lazer_ui_Pass_Event_

	inline virtual Result OnTextInput( TextInputEventArgs & )
		_lazer_ui_Pass_Event_

	inline virtual void OnFocusGain()
		_lazer_ui_Empty_Expression_

	inline virtual void OnFocusLost()
		_lazer_ui_Empty_Expression_

	inline virtual void OnUpdate( float dt )
		_lazer_ui_Empty_Expression_

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
	window_uid getuid();
private:
	window_uid uid;
};

class Widget : public Window {};

class DynamicWindow : public Window, public Parent
{
public:
	enum Flags_ {
		Flags_None = 0,
		Flags_Movable = _lazer_ui_bit(0),
		Flags_Sizable = _lazer_ui_bit(1),

		Flags_Default = Flags_Movable | Flags_Sizable,
	};
	enum State_ {
		State_Default = 0,
		State_Moving = _lazer_ui_bit(0),

		State_SizingL = _lazer_ui_bit(1),
		State_SizingR = _lazer_ui_bit(2),
		State_SizingT = _lazer_ui_bit(3),
		State_SizingB = _lazer_ui_bit(4),
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
	virtual void OnResize() {};

#ifndef _lazer_ui_no_cursor
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
		_lazer_ui_Empty_Expression_

	virtual Result OnKeyDown( KeyDownEventArgs & args ) override;
	virtual Result OnKeyUp( KeyUpEventArgs & args ) override;
};

class Slider : public Widget 
{
public:
	bool active = false;

public:
	inline virtual void OnSlide()
		_lazer_ui_Empty_Expression_

	inline virtual void UpdateSlidePosition( point mouse )
		_lazer_ui_Empty_Expression_

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
		_lazer_ui_Empty_Expression_

	inline virtual void OnEdit()
		_lazer_ui_Empty_Expression_
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
	~WindowManager();

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
#ifndef _lazer_ui_no_cursor
	virtual Result OnSetCursor( SetCursorEventArgs & args ) override;
#endif
	virtual Result OnMouseLeave() override;

	virtual void OnUpdate( float dt ) override;

	void bringToFront( Window * window );
	void setFocus( Window * window );
	void addWindow( Window * window );
	void Release( Window * window ) override;

//	Window * findWindow( window_uid uid );
};



/* ============================================================================================================== */
/* ----------------------------------------------- Implementation ----------------------------------------------- */
/* ============================================================================================================== */

inline Window::Window() : uid( ++g_NextWindow_uid ) {}
inline Window::Window( const Window & src ) : uid( ++g_NextWindow_uid ) {}
inline void Window::Release() { if( parent ) parent->Release( this ); }
inline bool Window::isInside( point pos ) { return false; }
inline window_uid Window::getuid() { return uid; }


inline WindowManager::WindowManager( int width, int height ) { Rect = rect::from_tl( {}, { width, height } ); }
inline WindowManager::~WindowManager() { for( auto && w : windowStack ) delete w; }

inline void WindowManager::SetCapture( Window * window ) { capture = window; }
inline void WindowManager::SetFocus( Window * window ) { if( focus ) focus->OnFocusLost(); focus = window; if( focus ) focus->OnFocusGain(); }
inline void WindowManager::SetHover( Window * window ) { if( hover ) hover->OnMouseLeave(); hover = window; }

inline Window * WindowManager::GetCapture() { return capture; };
inline Window * WindowManager::GetFocus() { return focus; };
inline Window * WindowManager::GetHover() { return hover; };

inline Result WindowManager::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyLeftMouse || args.key == g_KeyRightMouse )
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
inline bool DynamicWindow::isInside( point pos ) { return Rect.get_expanded( 8 )[pos]; }
inline void DynamicWindow::OnMove( point & new_pos ) {}

inline Result DynamicWindow::OnSetCursor( SetCursorEventArgs & args ) {
	if( hover ) { if( !hover->OnSetCursor( args ) ) return Handled; }
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
		Rect = rect::from_tl( p - last, size );
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
	//	auto size = Rect.size();
	//	Rect = rect::from_tl( p - last, size );
	}
	else if( capture ) {
		return capture->OnMouseMove( args );
	}
	else {
		for( auto && w : widgets )
			if( w->isInside( args.pos - Rect.get_tl() ) )
			{
				hover = w;
				w->OnMouseMove( args );
				return Handled;
			}
		hover = nullptr;
	}
	return Handled;
}

inline Result DynamicWindow::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyLeftMouse || args.key == g_KeyRightMouse ) {
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
	if( args.key == g_KeyLeftMouse && flags & Flags_Movable ) {
		auto P = GetRectPos( Rect, g_MousePosition, 8, 4, 8 );
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
		last = GetMousePosition() - Rect.get_tl();
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
	else if( args.key == g_KeyLeftMouse ) {
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
	if( args.key == g_KeyLeftMouse ) {
		active = true;
		return Handled;
	}
	return Pass;
}

inline Result Button::OnKeyUp( KeyUpEventArgs & args ) {
	if( g_KeyLeftMouse == args.key ) {
		if( parent->GetHover() == this ) OnPressed();
		active = false;
		return Handled;
	}
	return Pass;
}

inline Result Slider::OnKeyDown( KeyDownEventArgs & args ) {
	if( args.key == g_KeyLeftMouse ) {
		parent->SetCapture( this );
		active = true;
		UpdateSlidePosition( GetMousePosition() );
		return Handled;
	}
	return Pass;
}

inline Result Slider::OnKeyUp( KeyUpEventArgs & args ) {
	if( g_KeyLeftMouse == args.key ) {
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

_lazer_ui_end_header_