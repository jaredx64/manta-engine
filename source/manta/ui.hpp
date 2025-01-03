#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/factory.hpp>

#include <manta/input.hpp>
#include <manta/vector.hpp>
#include <manta/objects.system.hpp>
#include <manta/text.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysObjects { class UIWidget_t; } // fwd declaration for UIWidget.object

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace SysUI
{
	extern Keyboard keyboard;
	extern Mouse mouse;

	extern Matrix matrix; // Matrix for widget-relative coordinate -> screen coordinate

	extern bool init();
	extern bool free();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIScissor
{
	UIScissor() : scissoring{ false }, x1{ 0 }, y1{ 0 }, x2{ 0 }, y2{ 0 } { };
	UIScissor( int x1, int y1, int x2, int y2 ) : scissoring{ true }, x1{ x1 }, y1{ y1 }, x2{ x2 }, y2{ y2 } { }

	bool operator==( const UIScissor &s ) const
	{
		return scissoring == s.scissoring &&
		       x1 == s.x1 &&
		       y1 == s.y1 &&
		       x2 == s.x2 &&
		       y2 == s.y2;
	}

	bool scissoring;
	int x1, y1, x2, y2;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIWidgetDescription
{
	bool renderEnabled = true; // Determines whether to call EVENT_RENDER
	bool updateEnabled = true; // Determines whether to call EVENT_UPDATE
	bool glowing = false; // Represents a glowing visual state, i.e. first-time user experience (FTUE)1
	bool locked = false; // Prevents user interaction with the widget
	bool hoverable = true; // Enables detection of mouse hover (EVENT_TEST)
	bool container = false; // Restricts child mouse hover events to occur only when this widget is also hovered
	bool sortable = false; // Allows the widget to be reordered when clicked
};

struct UIButtonDescription : UIWidgetDescription
{
	int x = 0;
	int y = 0;
	int width = 64;
	int height = 32;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UIContext
{
_PUBLIC:
	friend class SysObjects::UIWidget_t;

_PUBLIC:
	void init();
	void free();
	void clear();

_PUBLIC:
	bool destroy_widget( Object &widget );
	Object create_widget( u16 type, Object parent = { } );

	Object create_widget_window( u16 type, int x, int y, int width, int height,
		bool resizable = true, Object parent = { } );

	Object create_widget_button( u16 type, int x, int y, int width, int height,
		Object parent = { } );

	Object create_widget_checkbox( u16 type, int x, int y, int width, int height, bool enabled,
		Object parent = { } );

	Object create_widget_slider( u16 type, int x, int y, int width, int height,
		bool vertical = false, float value = 0.0f, int steps = 0, Object parent = { } );

	Object create_widget_scrollbar( u16 type, int x, int y, int width, int height,
		bool vertical = false, float value = 0.0f, Object parent = { } );

	Object create_widget_textbox( u16 type, int x, int y, int width, int height,
		TextFormat format = { }, Object parent = { } );

	Object create_widget_textline( u16 type, int x, int y, int width, int height,
		TextFormat format = { }, Object parent = { } );

	Object create_widget_labeltext( u16 type, int x, int y, const char *label,
		int width = 0, int height = 0, Object parent = { } );

_PUBLIC:
	template <int N> ObjectHandle<N> handle( const Object &widget ) const
	{
		Assert( N == UIWidget || object_is_child_of( N, UIWidget ) );
		AssertMsg( N == widget.type || object_is_child_of( widget.type, N ), "h: %u w: %u", N, widget.type );
		return ObjectHandle<N>{ objects.get_object_pointer( widget ) };
	}

	Matrix matrix_get() { return matrix; }
	void matrix_set( const Matrix &matrix );
	void matrix_multiply( const Matrix &matrix );

	UIScissor scissor_get() { return scissor; }
	void scissor_set( const UIScissor &scissor );
	void scissor_set( int x1, int y1, int x2, int y2 ) { scissor_set( UIScissor { x1, y1, x2, y2 } ); }
	void scissor_set_nested( const UIScissor &scissor );
	void scissor_set_nested( int x1, int y1, int x2, int y2 ) { scissor_set_nested( UIScissor { x1, y1, x2, y2 } ); }

	floatv2 position_floatv2( const float x, const float y );
	intv2 position_i32( const int x, const int y );

	void widget_send_top( Object widget );
	Object get_widget_hover();
	Object get_widget_top();

	void update_widgets( const Delta delta );
	void render_widgets( const Delta delta );

	void update_widget( const Object &widget, const Delta delta );
	void render_widget( const Object &widget, const Delta delta );

_PRIVATE:
	Object instantiate_widget( u16 type, Object parent );

	void update_widget( ObjectHandle<UIWidget> &widgetHandle, const Delta delta );
	void render_widget( ObjectHandle<UIWidget> &widgetHandle, const Delta delta );

	bool test_widget( ObjectHandle<UIWidget> &widgetHandle );
	void register_widget( const Object &widget );
	void release_widget( const Object &widget );

_PRIVATE:
	ObjectContext objects { CategoryUI };
	List<Object> widgets;
	Object hoverWidget;
	bool clearing = false;

	Matrix matrix;
	UIScissor scissor;
	Keyboard keyboard;
	Mouse mouse;
};
constexpr usize s = sizeof( UIContext );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UICONTEXT_VALIDATE(uiContext, prefixMessage) \
	AssertMsg( uiContext != nullptr, \
		"%s%s", prefixMessage, ": invalid UIContext (bypassed constructor?)" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////