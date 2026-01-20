#pragma once

#include <core/types.hpp>
#include <core/list.hpp>
#include <core/factory.hpp>
#include <core/math.hpp>

#include <manta/input.hpp>
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
	extern ObjectInstance hoverWidgetID;
	extern UIContext *hoverWidgetContext;
	extern void update( Delta delta );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct UIScissor
{
	UIScissor() :
		scissoring { false }, x1 { 0 }, y1 { 0 }, x2 { 0 }, y2 { 0 } { };
	UIScissor( int x1, int y1, int x2, int y2 ) :
		scissoring { true }, x1 { x1 }, y1 { y1 }, x2 { x2 }, y2 { y2 } { }

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
	bool destroy_widget( ObjectInstance &widget );
	ObjectInstance create_widget( u16 type, ObjectInstance parent = { } );

	ObjectInstance create_widget_window( u16 type, int x, int y, int width, int height,
		bool resizable = true, ObjectInstance parent = { } );

	ObjectInstance create_widget_scrollable_region( u16 type, int x, int y, int width, int height,
		bool hasVerticalScrollbar, bool hasHorizontalScrollbar, ObjectInstance parent = { } );

	ObjectInstance create_widget_button( u16 type, int x, int y, int width, int height,
		ObjectInstance parent = { } );

	ObjectInstance create_widget_checkbox( u16 type, int x, int y, int width, int height, bool enabled,
		ObjectInstance parent = { } );

	ObjectInstance create_widget_slider( u16 type, int x, int y, int width, int height,
		bool vertical = false, float value = 0.0f, int steps = 0, ObjectInstance parent = { } );

	ObjectInstance create_widget_scrollbar( u16 type, int x, int y, int width, int height,
		bool vertical = false, float value = 0.0f, ObjectInstance parent = { } );

	ObjectInstance create_widget_textbox( u16 type, int x, int y, int width, int height,
		TextFormat format = { }, ObjectInstance parent = { } );

	ObjectInstance create_widget_textline( u16 type, int x, int y, int width, int height,
		TextFormat format = { }, ObjectInstance parent = { } );

	ObjectInstance create_widget_labeltext( u16 type, int x, int y, const char *label,
		int width = 0, int height = 0, ObjectInstance parent = { } );

public:
	template <Object_t T> ObjectHandle<T> handle( const ObjectInstance &widget ) const
	{
		DEBUG( if constexpr( T != Object::UIWidget ) { Assert( object_is_child_of( T, Object::UIWidget ) ) } )
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

	float_v2 position_float_v2( float x, float y );
	int_v2 position_i32( int x, int y );

	void widget_send_top( ObjectInstance widget );
	ObjectInstance get_widget_hover();
	ObjectInstance get_widget_top();

	void update_widgets( Delta delta );
	void render_widgets( Delta delta, Alpha alpha = { } );

	void update_widget( const ObjectInstance &widget, Delta delta );
	void render_widget( const ObjectInstance &widget, Delta delta, Alpha alpha );

private:
	ObjectInstance instantiate_widget( u16 type, ObjectInstance parent );

	void update_widget( ObjectHandle<Object::UIWidget> &widgetHandle, Delta delta );
	void render_widget( ObjectHandle<Object::UIWidget> &widgetHandle, Delta delta, Alpha alpha );

	bool test_widget( ObjectHandle<Object::UIWidget> &widgetHandle );
	void register_widget( const ObjectInstance &widget );
	void release_widget( const ObjectInstance &widget );

private:
	ObjectContext objects { ObjectCategory::UI };
	List<ObjectInstance> widgets;
	ObjectInstance hoverWidget;
	bool clearing = false;

	float_m44 matrix;
	UIScissor scissor;
	Keyboard keyboard;
	Mouse mouse;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UICONTEXT_VALIDATE(uiContext, prefixMessage) \
	AssertMsg( uiContext != nullptr, \
		"%s%s", prefixMessage, ": invalid UIContext (bypassed constructor?)" );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////