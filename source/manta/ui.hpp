#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/factory.hpp>

#include <manta/input.hpp>
#include <manta/vector.hpp>
#include <manta/objects.system.hpp>
#include <manta/text.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreObjects { class UIWidget_t; } // fwd declaration for UIWidget.object
class UIContext;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace CoreUI
{
	extern Keyboard keyboard;
	extern Mouse mouse;

	extern float_m44 matrix; // Matrix for widget-relative coordinate -> screen coordinate

	extern bool init();
	extern bool free();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace UI
{
	extern Object hoverWidgetID;
	extern UIContext *hoverWidgetContext;
	extern void update( const Delta delta );
}

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
public:
	friend class CoreObjects::UIWidget_t;

public:
	void init();
	void free();
	void clear();

public:
	bool destroy_widget( Object &widget );
	Object create_widget( u16 type, Object parent = { } );

	Object create_widget_window( u16 type, int x, int y, int width, int height,
		bool resizable = true, Object parent = { } );

	Object create_widget_scrollable_region( u16 type, int x, int y, int width, int height,
		const bool hasVerticalScrollbar, const bool hasHorizontalScrollbar, Object parent = { } );

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

public:
	template <u16 T> ObjectHandle<T> handle( const Object &widget ) const
	{
		Assert( T == ObjectType::UIWidget || object_is_child_of( T, ObjectType::UIWidget ) );
		if( !( T == widget.type || object_is_child_of( widget.type, T ) ) ) { return ObjectHandle<T>{ nullptr }; }
		return ObjectHandle<T>{ objects.get_object_pointer( widget ) };
	}

	float_m44 matrix_get() { return matrix; }
	void matrix_set( const float_m44 &matrix );
	void float_m44_multiply( const float_m44 &matrix );

	UIScissor scissor_get() { return scissor; }
	void scissor_set( const UIScissor &scissor );
	void scissor_set( int x1, int y1, int x2, int y2 ) { scissor_set( UIScissor { x1, y1, x2, y2 } ); }
	void scissor_set_nested( const UIScissor &scissor );
	void scissor_set_nested( int x1, int y1, int x2, int y2 ) { scissor_set_nested( UIScissor { x1, y1, x2, y2 } ); }

	float_v2 position_float_v2( const float x, const float y );
	int_v2 position_i32( const int x, const int y );

	void widget_send_top( Object widget );
	Object get_widget_hover();
	Object get_widget_top();

	void update_widgets( const Delta delta );
	void render_widgets( const Delta delta, const Alpha alpha = { } );

	void update_widget( const Object &widget, const Delta delta );
	void render_widget( const Object &widget, const Delta delta, const Alpha alpha );

private:
	Object instantiate_widget( u16 type, Object parent );

	void update_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle, const Delta delta );
	void render_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle, const Delta delta, const Alpha alpha );

	bool test_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle );
	void register_widget( const Object &widget );
	void release_widget( const Object &widget );

private:
	ObjectContext objects { ObjectCategory::UI };
	List<Object> widgets;
	Object hoverWidget;
	bool clearing = false;

	float_m44 matrix;
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