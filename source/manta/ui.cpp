#include <manta/ui.hpp>

#include <core/math.hpp>

#include <manta/objects.hpp>
#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Object UI::hoverWidgetID = Object { };
UIContext *UI::hoverWidgetContext = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreUI::init()
{
	return true;
}


bool CoreUI::free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UI::update( const Delta delta )
{
	if( Mouse::check( mb_left ) ) { return; }
	hoverWidgetID = Object { };
	hoverWidgetContext = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::init()
{
	objects.init();
	widgets.init();
	matrix = float_m44_build_identity();
}


void UIContext::free()
{
	clear();
	widgets.free();
	objects.free();
}


void UIContext::clear()
{
	// Destroy All Widgets
	clearing = true;
	{
		for( Object &widget : widgets ) { objects.destroy( widget ); }
		widgets.clear();
	}
	clearing = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIContext::destroy_widget( Object &widget )
{
	// Validate Type
	if( !widget ) { return true; }
	AssertMsg( object_is_child_of( widget.type,ObjectType::UIWidget ),
		"Attempting to destroy UIWidget of UIWidget type! (%s)", CoreObjects::TYPE_NAME[widget.type] );

	// Destroy widget
	return objects.destroy( widget );
}


Object UIContext::instantiate_widget( u16 type, Object parent )
{
	// Validate Type
	AssertMsg( object_is_child_of( type, ObjectType::UIWidget ),
		"Attempting to create widget of non-UIWidget type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	Object widget = objects.create( type );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget> widgetHandle = objects.handle<ObjectType::UIWidget>( widget );
	if( widgetHandle )
	{
		// State
		widgetHandle->context = this;
		widgetHandle->parent = parent;

		// Properties
		widgetHandle->hoverable = true;
		widgetHandle->sortable = true;
		widgetHandle->container = false;
	}

	// Return
	return widget;
}


Object UIContext::create_widget( u16 type, Object parent )
{
	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget> handle = objects.handle<ObjectType::UIWidget>( widget );
	if( handle ) { handle->init(); }

	// Return
	return widget;
}


Object UIContext::create_widget_window( u16 type, int x, int y, int width, int height,
	bool resizeable, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_Window ||
		object_is_child_of( type, ObjectType::UIWidget_Window ),
		"Attempting to create button of non-UIWidget_Window type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_Window> handle = objects.handle<ObjectType::UIWidget_Window>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->resizable = resizeable;
		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_scrollable_region( u16 type, int x, int y, int width, int height,
	const bool hasVerticalScrollbar, const bool hasHorizontalScrollbar, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_ScrollableRegion ||
		object_is_child_of( type, ObjectType::UIWidget_ScrollableRegion ),
		"Attempting to create button of non-UIWidget_ScrollableRegion type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_ScrollableRegion> handle =
		objects.handle<ObjectType::UIWidget_ScrollableRegion>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->viewWidth = width;
		handle->viewHeight = height;
		handle->hideScrollbarVertical = false;//!hasVerticalScrollbar;
		handle->hideScrollbarHorizontal = false;//!hasHorizontalScrollbar;
		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_button( u16 type, int x, int y, int width, int height, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_Button ||
		object_is_child_of( type, ObjectType::UIWidget_Button ),
		"Attempting to create button of non-UIWidget_Button type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_Button> handle = objects.handle<ObjectType::UIWidget_Button>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_checkbox( u16 type, int x, int y, int width, int height, bool enabled, Object parent )
{
	// Validate Type
	AssertMsg( type ==ObjectType::UIWidget_Checkbox ||
		object_is_child_of( type, ObjectType::UIWidget_Checkbox ),
		"Attempting to create button of non-UIWidget_Checkbox type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_Checkbox> handle = objects.handle<ObjectType::UIWidget_Checkbox>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->enabled = enabled;
		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_slider( u16 type, int x, int y, int width, int height, bool vertical,
	float value, int steps, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_Slider ||
		object_is_child_of( type, ObjectType::UIWidget_Slider ),
		"Attempting to create button of non-UIWidget_Slider type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_Slider> handle = objects.handle<ObjectType::UIWidget_Slider>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;

		handle->vertical = vertical;
		handle->value = clamp( value, 0.0f, 1.0f );
		handle->valueTo = handle->value;
		handle->steps = steps;

		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_scrollbar( u16 type, int x, int y, int width, int height, bool vertical,
	float value, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_Scrollbar ||
		object_is_child_of( type, ObjectType::UIWidget_Scrollbar ),
		"Attempting to create button of non-UIWidget_Scrollbar type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_Scrollbar> handle = objects.handle<ObjectType::UIWidget_Scrollbar>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;

		handle->vertical = vertical;
		handle->value = clamp( value, 0.0f, 1.0f );
		handle->valueTo = handle->value;

		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_textbox( u16 type, int x, int y, int width, int height,
	TextFormat format, Object parent )
{
	// Validate Type
	AssertMsg( type ==ObjectType::UIWidget_TextBox ||
		object_is_child_of( type, ObjectType::UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_TextBox> handle = objects.handle<ObjectType::UIWidget_TextBox>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->set_format_default( format );

		handle->limitWidth = 0;
		handle->limitHeight = 0;
		handle->limitCharacters = 0;
		handle->allowWordWrap = true;

		handle->init();
	}

	// Return
	return widget;
}


Object UIContext::create_widget_textline( u16 type, int x, int y, int width, int height,
	TextFormat format, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_TextBox ||
		object_is_child_of( type, ObjectType::UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_TextBox> handle = objects.handle<ObjectType::UIWidget_TextBox>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->set_format_default( format );
		handle->init();

		handle->limitWidth = 0;
		handle->limitHeight = 0;
		handle->limitCharacters = 0;
		handle->allowWordWrap = false;

		handle->input.allowNewlines = false;
		handle->input.allowTabs = false;
	}

	// Return
	return widget;
}


Object UIContext::create_widget_labeltext( u16 type, int x, int y, const char *label,
	int width, int height, Object parent )
{
	// Validate Type
	AssertMsg( type == ObjectType::UIWidget_LabelText ||
		object_is_child_of( type, ObjectType::UIWidget_LabelText ),
		"Attempting to create button of non-UIWidget_LabelText type! (%s)", CoreObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<ObjectType::UIWidget_LabelText> handle = objects.handle<ObjectType::UIWidget_LabelText>( widget );
	if( handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;

		handle->set_label( label );

		handle->init();
	}

	// Return
	return widget;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::matrix_set( const float_m44 &m )
{
	this->matrix = m;
}


void UIContext::float_m44_multiply( const float_m44 &m )
{
	matrix = ::float_m44_multiply( matrix, m );
}


void UIContext::scissor_set( const UIScissor &s )
{
	if( !s.scissoring )
	{
		scissor = UIScissor { };
		Gfx::reset_scissor();
		return;
	}

	// Set Scissor
	scissor = s;
	Gfx::set_scissor( scissor.x1, scissor.y1, scissor.x2, scissor.y2 );

	//draw_rectangle( 0, 0, 2000, 2000, c_red );
}


void UIContext::scissor_set_nested( const UIScissor &s )
{
	if( !s.scissoring )
	{
		scissor = UIScissor { };
		Gfx::reset_scissor();
		return;
	}

	// Nest Scissor
	if( !scissor.scissoring ) { scissor_set( s ); return; }
	scissor.scissoring = true;
	scissor.x1 = max( scissor.x1, s.x1 );
	scissor.y1 = max( scissor.y1, s.y1 );
	scissor.x2 = min( scissor.x2, s.x2 );
	scissor.y2 = min( scissor.y2, s.y2 );
	Gfx::set_scissor( scissor.x1, scissor.y1, scissor.x2, scissor.y2 );

	//draw_rectangle( 0, 0, 2000, 2000, c_purple );
}


float_v2 UIContext::position_float_v2( const float x, const float y )
{
	return float_v2( x, y ).multiply( this->matrix );
}


int_v2 UIContext::position_i32( const int x, const int y )
{
	return int_v2( x, y ).multiply( this->matrix );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::widget_send_top( Object widget )
{
	// If widget is a child, we must find its parent
	ObjectHandle<ObjectType::UIWidget> handle = objects.handle<ObjectType::UIWidget>( widget );
	Assert( handle );
	while( handle && handle->parent )
	{
		widget = handle->parent;
		handle = objects.handle<ObjectType::UIWidget>( widget );
		Assert( handle );
	}

	// Sortable?
	if( !handle->sortable ) { return; }

	// Push widget to top of stack
	for( usize i = 0; i < widgets.count(); i++ )
	{
		const Object &w = widgets[i];
		if( w != widget ) { continue; }

		widgets.remove( i );
		widgets.add( widget );
		return;
	}

	// If reached, we were passed an invalid widget
	AssertMsg( false, "Widget not registered in UIContext" );
}


Object UIContext::get_widget_hover()
{
	return hoverWidget;
}


Object UIContext::get_widget_top()
{
	if( widgets.count() == 0 ) { return Object { }; }
	return widgets[widgets.count() - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widgets( const Delta delta )
{
	// Mouse & Top Widget
	hoverWidget = ( !Mouse::check( mb_left ) ) ? Object { } : hoverWidget;
	for( auto widget = widgets.rbegin(); widget != widgets.rend(); ++widget )
	{
		ObjectHandle<ObjectType::UIWidget> handle = objects.handle<ObjectType::UIWidget>( *widget );
		Assert( handle );
		if( !handle->hoverable || handle->parent ) { continue; }
		if( test_widget( handle ) ) { break; }
	}
	if( hoverWidget && Mouse::check_pressed( mb_left ) ) { widget_send_top( hoverWidget ); }

	// Update Widgets
	for( const Object &widget : widgets )
	{
		ObjectHandle<ObjectType::UIWidget> handle = objects.handle<ObjectType::UIWidget>( widget );
		Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->updateEnabled ) { continue; }
		update_widget( handle, delta );
	}
}


void UIContext::render_widgets( const Delta delta, const Alpha alpha )
{
	// Draw Widgets
	for( const Object &widget : widgets )
	{
		ObjectHandle<ObjectType::UIWidget> handle = objects.handle<ObjectType::UIWidget>( widget );
		Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->renderEnabled ) { continue; }
		render_widget( handle, delta, alpha );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( const Object &widget, const Delta delta )
{
	// Update Widget
	ObjectHandle<ObjectType::UIWidget> widgetHandle = objects.handle<ObjectType::UIWidget>( widget );
	if( widgetHandle ) { update_widget( widgetHandle, delta ); }
}


void UIContext::render_widget( const Object &widget, const Delta delta, const Alpha alpha )
{
	// Draw Widget
	ObjectHandle<ObjectType::UIWidget> widgetHandle = objects.handle<ObjectType::UIWidget>( widget );
	if( widgetHandle ) { render_widget( widgetHandle, delta, alpha ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle, const Delta delta )
{
	// Update Widget
	if( widgetHandle->callbackOnUpdate ) { widgetHandle->callbackOnUpdate( *this, widgetHandle->id ); }
	widgetHandle->update( delta );

	// Update Children
	if( widgetHandle->children.count() > 0 )
	{
		// Update Matrix
		const float_m44 matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( Object &child : widgetHandle->children )
		{
			ObjectHandle<ObjectType::UIWidget> childHandle = objects.handle<ObjectType::UIWidget>( child );
			if( childHandle ) { update_widget( childHandle, delta ); }
		}

		// Reset matrix
		matrix_set( matrix );
	}
}


void UIContext::render_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle,
	const Delta delta, const Alpha alpha )
{
	// Skip invisible widgets
	if( !widgetHandle->renderEnabled ) { return; }

	// Cache Scissor
	const UIScissor scissor = scissor_get();

	// Draw Widget
	const Alpha widgetTransparency = widgetHandle->transparency * alpha;

	if( widgetHandle->callbackOnRender )
	{
		widgetHandle->callbackOnRender( *this, widgetHandle->id, widgetTransparency );
	}

	widgetHandle->render( delta, widgetTransparency );

	// Draw Children
	if( widgetHandle->children.count() > 0 )
	{
		// Update Matrix
		const float_m44 matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( Object &child : widgetHandle->children )
		{
			ObjectHandle<ObjectType::UIWidget> childHandle = objects.handle<ObjectType::UIWidget>( child );
			if( childHandle )
			{
				render_widget( childHandle, delta, childHandle->transparency * widgetTransparency );
			}
		}

		// Reset Matrix
		matrix_set( matrix );
	}

	// Reset Scissor
	if( scissor_get() != scissor ) { scissor_set( scissor ); }
}


bool UIContext::test_widget( ObjectHandle<ObjectType::UIWidget> &widgetHandle )
{
	// Skip invisible widgets
	if( !widgetHandle->renderEnabled ) { return false; }

	// Test Widget
	const bool hovering = widgetHandle->contains_point( float_v2 { mouse_x, mouse_y } );
	if( hovering )
	{
		if( !Mouse::check( mb_left ) ) { hoverWidget = widgetHandle->id; } // TODO: Clean this
		UI::hoverWidgetID = hoverWidget;
		UI::hoverWidgetContext = this;
	}

	// Test Children
	if( widgetHandle->children.count() > 0 && ( hovering || !widgetHandle->container ) )
	{
		// Update Matrix
		const float_m44 matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( auto child = widgetHandle->children.rbegin(); child != widgetHandle->children.rend(); ++child )
		{
			ObjectHandle<ObjectType::UIWidget> childHandle = objects.handle<ObjectType::UIWidget>( *child );
			if( childHandle && childHandle->hoverable && test_widget( childHandle ) )
			{
				matrix_set( matrix );
				return true;
			}
		}

		// Reset Matrix
		matrix_set( matrix );
	}

	// Failure
	return hovering;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::register_widget( const Object &widget )
{
	// Fetch Widget Handle
	ObjectHandle<ObjectType::UIWidget> widgetHandle = objects.handle<ObjectType::UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = false;

	// Register Widget in UIContext
	widgets.add( widget );

	// Register Widget on Parent
	ObjectHandle<ObjectType::UIWidget> parentHandle = objects.handle<ObjectType::UIWidget>( widgetHandle->parent );
	if( parentHandle ) { parentHandle->children.add( widget ); }
}


void UIContext::release_widget( const Object &widget )
{
	// Early out if UIContext clear()
	if( clearing ) { return; }

	// Fetch Widget Handle
	ObjectHandle<ObjectType::UIWidget> widgetHandle = objects.handle<ObjectType::UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = true;

	// Destroy Children
	for( Object &child : widgetHandle->children ) { objects.destroy( child ); }

	// Release Widget from Parent
	ObjectHandle<ObjectType::UIWidget> parentHandle = objects.handle<ObjectType::UIWidget>( widgetHandle->parent );
	if( parentHandle && !parentHandle->releasing )
	{
		for( usize i = 0; i < parentHandle->children.count(); i++ )
		{
			const Object &w = parentHandle->children[i];
			if( w == widget ) { parentHandle->children.remove( i ); break; }
		}
	}

	// Release Widget from UIContext
	for( usize i = 0; i < widgets.count(); i++ )
	{
		const Object &w = widgets[i];
		if( w == widget ) { widgets.remove( i ); break; }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////