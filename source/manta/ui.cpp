#include <manta/ui.hpp>

#include <core/math.hpp>

#include <manta/objects.hpp>
#include <manta/gfx.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ObjectInstance UI::hoverWidgetID = ObjectInstance { };
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

void UI::update( Delta delta )
{
	if( Mouse::check( mb_left ) ) { return; }
	hoverWidgetID = ObjectInstance { };
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
		for( ObjectInstance &widget : widgets ) { objects.destroy( widget ); }
		widgets.clear();
	}
	clearing = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIContext::destroy_widget( ObjectInstance &widget )
{
	if( !widget ) { return true; }

	AssertMsg( object_is_child_of( widget.type, Object::UIWidget ),
		"Attempting to destroy UIWidget of UIWidget type! (%s)", CoreObjects::TYPE_NAME[widget.type] );

	return objects.destroy( widget );
}


ObjectInstance UIContext::instantiate_widget( u16 type, ObjectInstance parent )
{
	AssertMsg( object_is_child_of( type, Object::UIWidget ),
		"Attempting to create widget of non-UIWidget type! (%s)", CoreObjects::TYPE_NAME[type] );

	ObjectInstance widget = objects.create( type );
	if( auto handle = objects.handle<Object::UIWidget>( widget ); handle )
	{
		handle->context = this;
		handle->parent = parent;
		handle->hoverable = true;
		handle->sortable = true;
		handle->container = false;
	}

	return widget;
}


ObjectInstance UIContext::create_widget( u16 type, ObjectInstance parent )
{
	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget>( widget ); handle ) { handle->init(); }
	return widget;
}


ObjectInstance UIContext::create_widget_window( u16 type, int x, int y, int width, int height,
	bool resizeable, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_Window ||
		object_is_child_of( type, Object::UIWidget_Window ),
		"Attempting to create button of non-UIWidget_Window type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_Window>( widget ); handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->resizable = resizeable;
		handle->init();
	}

	return widget;
}


ObjectInstance UIContext::create_widget_scrollable_region( u16 type, int x, int y, int width, int height,
	bool hasVerticalScrollbar, bool hasHorizontalScrollbar, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_ScrollableRegion ||
		object_is_child_of( type, Object::UIWidget_ScrollableRegion ),
		"Attempting to create button of non-UIWidget_ScrollableRegion type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_ScrollableRegion>( widget ); handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->viewWidth = width;
		handle->viewHeight = height;
		handle->hideScrollbarVertical = false;
		handle->hideScrollbarHorizontal = false;
		handle->init();
	}

	return widget;
}


ObjectInstance UIContext::create_widget_button( u16 type, int x, int y, int width, int height, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_Button ||
		object_is_child_of( type, Object::UIWidget_Button ),
		"Attempting to create button of non-UIWidget_Button type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_Button>( widget ); handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->init();
	}

	return widget;
}


ObjectInstance UIContext::create_widget_checkbox( u16 type, int x, int y, int width, int height, bool enabled,
	ObjectInstance parent )
{
	AssertMsg( type ==Object::UIWidget_Checkbox ||
		object_is_child_of( type, Object::UIWidget_Checkbox ),
		"Attempting to create button of non-UIWidget_Checkbox type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_Checkbox>( widget ); handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;
		handle->enabled = enabled;
		handle->init();
	}

	return widget;
}


ObjectInstance UIContext::create_widget_slider( u16 type, int x, int y, int width, int height, bool vertical,
	float value, int steps, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_Slider ||
		object_is_child_of( type, Object::UIWidget_Slider ),
		"Attempting to create button of non-UIWidget_Slider type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_Slider>( widget ); handle )
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

	return widget;
}


ObjectInstance UIContext::create_widget_scrollbar( u16 type, int x, int y, int width, int height, bool vertical,
	float value, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_Scrollbar ||
		object_is_child_of( type, Object::UIWidget_Scrollbar ),
		"Attempting to create button of non-UIWidget_Scrollbar type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_Scrollbar>( widget ); handle )
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

	return widget;
}


ObjectInstance UIContext::create_widget_textbox( u16 type, int x, int y, int width, int height,
	TextFormat format, ObjectInstance parent )
{
	AssertMsg( type ==Object::UIWidget_TextBox ||
		object_is_child_of( type, Object::UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_TextBox>( widget ); handle )
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

	return widget;
}


ObjectInstance UIContext::create_widget_textline( u16 type, int x, int y, int width, int height,
	TextFormat format, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_TextBox ||
		object_is_child_of( type, Object::UIWidget_TextBox ),
		"Attempting to create button of non-UIWidget_TextBox type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_TextBox>( widget ); handle )
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

	return widget;
}


ObjectInstance UIContext::create_widget_labeltext( u16 type, int x, int y, const char *label,
	int width, int height, ObjectInstance parent )
{
	AssertMsg( type == Object::UIWidget_LabelText ||
		object_is_child_of( type, Object::UIWidget_LabelText ),
		"Attempting to create button of non-UIWidget_LabelText type! (%s)", CoreObjects::TYPE_NAME[type] );

	const ObjectInstance widget = instantiate_widget( type, parent );
	if( auto handle = objects.handle<Object::UIWidget_LabelText>( widget ); handle )
	{
		handle->x = x;
		handle->y = y;
		handle->width = width;
		handle->height = height;

		handle->set_label( label );

		handle->init();
	}

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
		Gfx::scissor_reset();
		return;
	}

	scissor = s;
	Gfx::scissor_set( scissor.x1, scissor.y1, scissor.x2, scissor.y2 );
}


void UIContext::scissor_set_nested( const UIScissor &s )
{
	if( !s.scissoring )
	{
		scissor = UIScissor { };
		Gfx::scissor_reset();
		return;
	}

	if( !scissor.scissoring ) { scissor_set( s ); return; }
	if( s.x1 >= scissor.x2 || s.x2 < scissor.x1 || s.y1 >= scissor.y2 || s.y2 < scissor.y1 ) { return; }

	scissor.scissoring = true;
	scissor.x1 = max( scissor.x1, s.x1 );
	scissor.y1 = max( scissor.y1, s.y1 );
	scissor.x2 = min( scissor.x2, s.x2 );
	scissor.y2 = min( scissor.y2, s.y2 );

	Gfx::scissor_set( scissor.x1, scissor.y1, scissor.x2, scissor.y2 );
}


float_v2 UIContext::position_float_v2( float x, float y )
{
	return float_v2( x, y ).multiply( this->matrix );
}


int_v2 UIContext::position_i32( int x, int y )
{
	return int_v2( x, y ).multiply( this->matrix );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::widget_send_top( ObjectInstance widget )
{
	// If widget is a child, we must find its parent
	ObjectHandle<Object::UIWidget> handle = objects.handle<Object::UIWidget>( widget );
	Assert( handle );
	while( handle && handle->parent )
	{
		widget = handle->parent;
		handle = objects.handle<Object::UIWidget>( widget );
		Assert( handle );
	}

	// Sortable?
	if( !handle->sortable ) { return; }

	// Push widget to top of stack
	for( usize i = 0; i < widgets.count(); i++ )
	{
		const ObjectInstance &w = widgets[i];
		if( w != widget ) { continue; }

		widgets.remove( i );
		widgets.add( widget );
		return;
	}

	// If reached, we were passed an invalid widget
	AssertMsg( false, "Widget not registered in UIContext" );
}


ObjectInstance UIContext::get_widget_hover()
{
	return hoverWidget;
}


ObjectInstance UIContext::get_widget_top()
{
	if( widgets.count() == 0 ) { return ObjectInstance { }; }
	return widgets[widgets.count() - 1];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widgets( Delta delta )
{
	// Mouse & Top Widget
	hoverWidget = ( !Mouse::check( mb_left ) ) ? ObjectInstance { } : hoverWidget;
	for( auto widget = widgets.rbegin(); widget != widgets.rend(); ++widget )
	{
		ObjectHandle<Object::UIWidget> handle = objects.handle<Object::UIWidget>( *widget );
		Assert( handle );
		if( !handle->hoverable || handle->parent ) { continue; }
		if( test_widget( handle ) ) { break; }
	}
	if( hoverWidget && Mouse::check_pressed( mb_left ) ) { widget_send_top( hoverWidget ); }

	// Update Widgets
	for( const ObjectInstance &widget : widgets )
	{
		ObjectHandle<Object::UIWidget> handle = objects.handle<Object::UIWidget>( widget );
		Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->updateEnabled ) { continue; }
		update_widget( handle, delta );
	}
}


void UIContext::render_widgets( Delta delta, Alpha alpha )
{
	// Draw Widgets
	for( const ObjectInstance &widget : widgets )
	{
		ObjectHandle<Object::UIWidget> handle = objects.handle<Object::UIWidget>( widget );
		Assert( handle );
		if( handle->parent ) { continue; }
		if( !handle->renderEnabled ) { continue; }
		render_widget( handle, delta, alpha );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( const ObjectInstance &widget, Delta delta )
{
	ObjectHandle<Object::UIWidget> widgetHandle = objects.handle<Object::UIWidget>( widget );
	if( widgetHandle ) { update_widget( widgetHandle, delta ); }
}


void UIContext::render_widget( const ObjectInstance &widget, Delta delta, Alpha alpha )
{
	ObjectHandle<Object::UIWidget> widgetHandle = objects.handle<Object::UIWidget>( widget );
	if( widgetHandle ) { render_widget( widgetHandle, delta, alpha ); }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::update_widget( ObjectHandle<Object::UIWidget> &widgetHandle, Delta delta )
{
	// Widget
	if( widgetHandle->callbackOnUpdate ) { widgetHandle->callbackOnUpdate( *this, widgetHandle->id ); }
	widgetHandle->update( delta );

	// Children
	if( widgetHandle->children.count() > 0 )
	{
		const float_m44 matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( ObjectInstance &child : widgetHandle->children )
		{
			ObjectHandle<Object::UIWidget> childHandle = objects.handle<Object::UIWidget>( child );
			if( childHandle ) { update_widget( childHandle, delta ); }
		}

		matrix_set( matrix );
	}
}


void UIContext::render_widget( ObjectHandle<Object::UIWidget> &widgetHandle, Delta delta, Alpha alpha )
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

		for( ObjectInstance &child : widgetHandle->children )
		{
			ObjectHandle<Object::UIWidget> childHandle = objects.handle<Object::UIWidget>( child );
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


bool UIContext::test_widget( ObjectHandle<Object::UIWidget> &widgetHandle )
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
		const float_m44 matrix = matrix_get();
		widgetHandle->apply_matrix();

		for( auto child = widgetHandle->children.rbegin(); child != widgetHandle->children.rend(); ++child )
		{
			ObjectHandle<Object::UIWidget> childHandle = objects.handle<Object::UIWidget>( *child );
			if( childHandle && childHandle->hoverable && test_widget( childHandle ) )
			{
				matrix_set( matrix );
				return true;
			}
		}

		matrix_set( matrix );
	}

	return hovering;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIContext::register_widget( const ObjectInstance &widget )
{
	// Fetch Widget Handle
	ObjectHandle<Object::UIWidget> widgetHandle = objects.handle<Object::UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = false;

	// Register Widget in UIContext
	widgets.add( widget );

	// Register Widget on Parent
	ObjectHandle<Object::UIWidget> parentHandle = objects.handle<Object::UIWidget>( widgetHandle->parent );
	if( parentHandle ) { parentHandle->children.add( widget ); }
}


void UIContext::release_widget( const ObjectInstance &widget )
{
	// Early out if UIContext clear()
	if( clearing ) { return; }

	// Fetch Widget Handle
	ObjectHandle<Object::UIWidget> widgetHandle = objects.handle<Object::UIWidget>( widget );
	Assert( widgetHandle );
	widgetHandle->releasing = true;

	// Destroy Children
	for( ObjectInstance &child : widgetHandle->children ) { objects.destroy( child ); }

	// Release Widget from Parent
	ObjectHandle<Object::UIWidget> parentHandle = objects.handle<Object::UIWidget>( widgetHandle->parent );
	if( parentHandle && !parentHandle->releasing )
	{
		for( usize i = 0; i < parentHandle->children.count(); i++ )
		{
			const ObjectInstance &w = parentHandle->children[i];
			if( w == widget ) { parentHandle->children.remove( i ); break; }
		}
	}

	// Release Widget from UIContext
	for( usize i = 0; i < widgets.count(); i++ )
	{
		const ObjectInstance &w = widgets[i];
		if( w == widget ) { widgets.remove( i ); break; }
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////