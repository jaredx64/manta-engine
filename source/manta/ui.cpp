#include <manta/ui.hpp>

#include <manta/objects.hpp>
#include <manta/gfx.hpp>
#include <manta/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SysUI::init()
{
	return true;
}


bool SysUI::free()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::init()
{
	objects.init();
	widgets.init();
	matrix = matrix_build_identity();
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
	AssertMsg( object_is_child_of( widget.type, UIWidget ),
		"Attempting to destroy UIWidget of UIWidget type! (%s)", SysObjects::TYPE_NAME[widget.type] );

	// Destroy widget
	return objects.destroy( widget );
}


Object UIContext::instantiate_widget( u16 type, Object parent )
{
	// Validate Type
	AssertMsg( object_is_child_of( type, UIWidget ),
		"Attempting to create widget of non-UIWidget type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	Object widget = objects.create( type );

	// Initialize Widget
	ObjectHandle<UIWidget> widgetHandle = objects.handle<UIWidget>( widget );
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
	ObjectHandle<UIWidget> handle = objects.handle<UIWidget>( widget );
	if( handle ) { handle->init(); }

	// Return
	return widget;
}


Object UIContext::create_widget_window( u16 type, int x, int y, int width, int height,
	bool resizeable, Object parent )
{
	// Validate Type
	AssertMsg( type == UIWidget_Window || object_is_child_of( type, UIWidget_Window ),
		"Attempting to create button of non-UIWidget_Window type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_Window> handle = objects.handle<UIWidget_Window>( widget );
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


Object UIContext::create_widget_button( u16 type, int x, int y, int width, int height, Object parent )
{
	// Validate Type
	AssertMsg( type == UIWidget_Button || object_is_child_of( type, UIWidget_Button ),
		"Attempting to create button of non-UIWidget_Button type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_Button> handle = objects.handle<UIWidget_Button>( widget );
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
	AssertMsg( type == UIWidget_Checkbox || object_is_child_of( type, UIWidget_Checkbox ),
		"Attempting to create button of non-UIWidget_Checkbox type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_Checkbox> handle = objects.handle<UIWidget_Checkbox>( widget );
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
	AssertMsg( type == UIWidget_Slider || object_is_child_of( type, UIWidget_Slider ),
		"Attempting to create button of non-UIWidget_Slider type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_Slider> handle = objects.handle<UIWidget_Slider>( widget );
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
	AssertMsg( type == UIWidget_Scrollbar || object_is_child_of( type, UIWidget_Scrollbar ),
		"Attempting to create button of non-UIWidget_Scrollbar type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_Scrollbar> handle = objects.handle<UIWidget_Scrollbar>( widget );
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
	AssertMsg( type == UIWidget_TextBox || object_is_child_of( type, UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_TextBox> handle = objects.handle<UIWidget_TextBox>( widget );
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
	AssertMsg( type == UIWidget_TextBox || object_is_child_of( type, UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_TextBox> handle = objects.handle<UIWidget_TextBox>( widget );
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
	AssertMsg( type == UIWidget_LabelText || object_is_child_of( type, UIWidget_LabelText ),
		"Attempting to create button of non-UIWidget_LabelText type! (%s)", SysObjects::TYPE_NAME[type] );

	// Create Widget
	const Object widget = instantiate_widget( type, parent );

	// Initialize Widget
	ObjectHandle<UIWidget_LabelText> handle = objects.handle<UIWidget_LabelText>( widget );
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

void UIContext::matrix_set( const Matrix &m )
{
	this->matrix = m;
}


void UIContext::matrix_multiply( const Matrix &m )
{
	matrix = ::matrix_multiply( matrix, m );
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


floatv2 UIContext::position_floatv2( const float x, const float y )
{
	return floatv2( x, y ).multiply( this->matrix );
}


intv2 UIContext::position_i32( const int x, const int y )
{
	return intv2( x, y ).multiply( this->matrix );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::widget_send_top( Object widget )
{
	// If widget is a child, we must find its parent
	ObjectHandle<UIWidget> handle = objects.handle<UIWidget>( widget ); Assert( handle );
	while( handle && handle->parent )
	{
		widget = handle->parent;
		handle = objects.handle<UIWidget>( widget ); Assert( handle );
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
	if( !Mouse::check( mb_left ) )
	{
		hoverWidget = Object { };
		for( auto widget = widgets.rbegin(); widget != widgets.rend(); ++widget )
		{
			ObjectHandle<UIWidget> handle = objects.handle<UIWidget>( *widget ); Assert( handle );
			if( !handle->hoverable || handle->parent ) { continue; }
			if( test_widget( handle ) ) { break; }
		}
	}
	if( hoverWidget && Mouse::check_pressed( mb_left ) ) { widget_send_top( hoverWidget ); }

	// Update Widgets
	for( const Object &widget : widgets )
	{
		ObjectHandle<UIWidget> handle = objects.handle<UIWidget>( widget ); Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->updateEnabled ) { continue; }
		update_widget( handle, delta );
	}
}


void UIContext::render_widgets( const Delta delta )
{
	// Draw Widgets
	for( const Object &widget : widgets )
	{
		ObjectHandle<UIWidget> handle = objects.handle<UIWidget>( widget ); Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->renderEnabled ) { continue; }
		render_widget( handle, delta );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( const Object &widget, const Delta delta )
{
	// Update Widget
	ObjectHandle<UIWidget> widgetHandle = objects.handle<UIWidget>( widget );
	if( widgetHandle ) { update_widget( widgetHandle, delta ); }
}


void UIContext::render_widget( const Object &widget, const Delta delta )
{
	// Draw Widget
	ObjectHandle<UIWidget> widgetHandle = objects.handle<UIWidget>( widget );
	if( widgetHandle ) { render_widget( widgetHandle, delta ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( ObjectHandle<UIWidget> &widgetHandle, const Delta delta )
{
	// Update Widget
	if( widgetHandle->callbackOnUpdate ) { widgetHandle->callbackOnUpdate( *this, widgetHandle->id ); }
	widgetHandle->update( delta );

	// Update Children
	if( widgetHandle->children.count() > 0 )
	{
		// Update Matrix
		const Matrix matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( Object &child : widgetHandle->children )
		{
			ObjectHandle<UIWidget> childHandle = objects.handle<UIWidget>( child );
			if( childHandle ) { update_widget( childHandle, delta ); }
		}

		// Reset matrix
		matrix_set( matrix );
	}
}


void UIContext::render_widget( ObjectHandle<UIWidget> &widgetHandle, const Delta delta )
{
	// Skip invisible widgets
	if( !widgetHandle->renderEnabled ) { return; }

	// Cache Scissor
	const UIScissor scissor = scissor_get();

	// Draw Widget
	if( widgetHandle->callbackOnRender ) { widgetHandle->callbackOnRender( *this, widgetHandle->id ); }
	widgetHandle->render( delta );

	// Draw Children
	if( widgetHandle->children.count() > 0 )
	{
		// Update Matrix
		const Matrix matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( Object &child : widgetHandle->children )
		{
			ObjectHandle<UIWidget> childHandle = objects.handle<UIWidget>( child );
			if( childHandle ) { render_widget( childHandle, delta ); }
		}

		// Reset Matrix
		matrix_set( matrix );
	}

	// Reset Scissor
	if( scissor_get() != scissor ) { scissor_set( scissor ); }
}


bool UIContext::test_widget( ObjectHandle<UIWidget> &widgetHandle )
{
	// Skip invisible widgets
	if( !widgetHandle->renderEnabled ) { return false; }

	// Test Widget
	const bool hovering = widgetHandle->contains_point( floatv2 { mouse_x, mouse_y } );
	if( hovering ) { hoverWidget = widgetHandle->id; }

	// Test Children
	if( widgetHandle->children.count() > 0 && ( hovering || !widgetHandle->container ) )
	{
		// Update Matrix
		const Matrix matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( auto child = widgetHandle->children.rbegin(); child != widgetHandle->children.rend(); ++child )
		{
			ObjectHandle<UIWidget> childHandle = objects.handle<UIWidget>( *child );
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
	ObjectHandle<UIWidget> widgetHandle = objects.handle<UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = false;

	// Register Widget in UIContext
	widgets.add( widget );

	// Register Widget on Parent
	ObjectHandle<UIWidget> parentHandle = objects.handle<UIWidget>( widgetHandle->parent );
	if( parentHandle ) { parentHandle->children.add( widget ); }
}


void UIContext::release_widget( const Object &widget )
{
	// Early out if UIContext clear()
	if( clearing ) { return; }

	// Fetch Widget Handle
	ObjectHandle<UIWidget> widgetHandle = objects.handle<UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = true;

	// Destroy Children
	for( Object &child : widgetHandle->children ) { objects.destroy( child ); }

	// Release Widget from Parent
	ObjectHandle<UIWidget> parentHandle = objects.handle<UIWidget>( widgetHandle->parent );
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