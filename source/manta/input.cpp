#include <manta/input.hpp>

#include <core/debug.hpp>

#include <manta/window.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Keyboard KEYBOARD_DEFAULT;
Keyboard *Keyboard::keyboard = &KEYBOARD_DEFAULT;

static Mouse MOUSE_DEFAULT;
Mouse *Mouse::mouse = &MOUSE_DEFAULT;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Keyboard::State &Keyboard::state()
{
	return keyboard->keyboardState;
}


void Keyboard::reset_active()
{
	Keyboard::keyboard = &KEYBOARD_DEFAULT;
}


void Keyboard::set_active( Keyboard &keyboard )
{
	Keyboard::keyboard = &keyboard;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Keyboard::keyboard_check( u8 key )
{
	return keyboardState.keyCurrent[key];
}


bool Keyboard::keyboard_check_pressed( u8 key )
{
	return keyboardState.keyCurrent[key] && !keyboardState.keyPrevious[key];
}


bool Keyboard::keyboard_check_pressed_repeat( u8 key )
{
	return keyboard_check_pressed( key ) || keyboardState.keyRepeat[key];
}


bool Keyboard::keyboard_check_released( u8 key )
{
	return !keyboardState.keyCurrent[key] && keyboardState.keyPrevious[key];
}


bool Keyboard::keyboard_check_any()
{
	for( int i = 0; i < 256; i++ )
	{
		if( Keyboard::keyboard_check( i ) ) { return true; }
	}

	return false;
}


bool Keyboard::keyboard_check_pressed_any()
{
	for( int i = 0; i < 256; i++ )
	{
		if( Keyboard::keyboard_check_pressed( i ) ) { return true; }
	}

	return false;
}


bool Keyboard::keyboard_check_pressed_repeat_any()
{
	for( int i = 0; i < 256; i++ )
	{
		if( Keyboard::keyboard_check_pressed_repeat( i ) ) { return true; }
	}

	return false;
}


bool Keyboard::keyboard_check_released_any()
{
	for( int i = 0; i < 256; i++ )
	{
		if( Keyboard::keyboard_check_released( i ) ) { return true; }
	}

	return false;
}

bool Keyboard::keyboard_has_input()
{
	return keyboardState.inputBuffer[0] != '\0';
}


char *Keyboard::keyboard_input_buffer()
{
	return keyboardState.inputBuffer;
}


void Keyboard::keyboard_update( Delta delta )
{
	// Key State
	for( int key = 0; key < 256; key++ )
	{
		keyboardState.keyPrevious[key] = keyboardState.keyCurrent[key];
		keyboardState.keyRepeat[key] = false;
	}

	// Reset Input Buffer
	keyboardState.inputBuffer[0] = '\0';
}


void Keyboard::keyboard_clear()
{
	// Reset Key States
	for( int key = 0; key < 256; key++ )
	{
		keyboardState.keyCurrent[key] = false;
		keyboardState.keyPrevious[key] = false;
		keyboardState.keyRepeat[key] = false;
		keyboardState.keyRepeatTimer[key] = 0.0;
	}

	// Clear Input Buffer
	keyboardState.inputBuffer[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mouse::State &Mouse::state()
{
	return mouse->mouseState;
}


void Mouse::reset_active()
{
	Mouse::mouse = &MOUSE_DEFAULT;
}


void Mouse::set_active( Mouse &mouse )
{
	Mouse::mouse = &mouse;
}


float Mouse::x()
{
#if DPI_WINDOW
	return Mouse::state().positionX * Window::scale;
#else
	return Mouse::state().positionX;
#endif
}


float Mouse::y()
{
#if DPI_WINDOW
	return Mouse::state().positionY * Window::scale;
#else
	return Mouse::state().positionY;
#endif
}


float Mouse::x_previous()
{
#if DPI_WINDOW
	return Mouse::state().positionXPrevious * Window::scale;
#else
	return Mouse::state().positionXPrevious;
#endif
}


float Mouse::y_previous()
{
#if DPI_WINDOW
	return Mouse::state().positionYPrevious * Window::scale;
#else
	return Mouse::state().positionYPrevious;
#endif
}


float Mouse::x_logical()
{
	return Mouse::state().positionX;
}


float Mouse::y_logical()
{
	return Mouse::state().positionY;
}


float Mouse::x_logical_previous()
{
	return Mouse::state().positionXPrevious;
}


float Mouse::y_logical_previous()
{
	return Mouse::state().positionYPrevious;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Mouse::mouse_check( u8 button )
{
	return ( mouseState.buttonCurrent & button );
}


bool Mouse::mouse_check_pressed( u8 button )
{
	return ( mouseState.buttonCurrent & button ) && !( mouseState.buttonPrevious & button );
}


bool Mouse::mouse_check_released( u8 button )
{
	return ( button & mouseState.buttonCurrent ) == 0 && ( button & mouseState.buttonPrevious ) != 0;
}


bool Mouse::mouse_check_wheel_up()
{
	return mouseState.wheelY < 0;
}


bool Mouse::mouse_check_wheel_down()
{
	return mouseState.wheelY > 0;
}


bool Mouse::mouse_check_wheel_left()
{
	return mouseState.wheelX < 0;
}


bool Mouse::mouse_check_wheel_right()
{
	return mouseState.wheelX > 0;
}


float Mouse::mouse_wheel_velocity_x()
{
	return mouseState.wheelXVelocity;
}


float Mouse::mouse_wheel_velocity_y()
{
	return mouseState.wheelYVelocity;
}


bool Mouse::mouse_moved()
{
	return mouseState.positionX != mouseState.positionXPrevious ||
		mouseState.positionY != mouseState.positionYPrevious;
}


void Mouse::mouse_get_position_precise( double &x, double &y )
{
	CoreWindow::mouse_get_position( x, y );
}


void Mouse::mouse_set_position( int x, int y )
{
	CoreWindow::mouse_set_position( x, y );
}


void Mouse::mouse_update( Delta delta )
{
	mouseState.buttonPrevious = mouseState.buttonCurrent;

	mouseState.wheelX = 0;
	mouseState.wheelY = 0;
	mouseState.wheelXVelocity = 0.0f;
	mouseState.wheelYVelocity = 0.0f;
}


void Mouse::mouse_clear()
{
	mouseState.wheelX = 0;
	mouseState.wheelY = 0;
	mouseState.wheelXVelocity = 0.0f;
	mouseState.wheelYVelocity = 0.0f;

	mouseState.buttonCurrent = 0;
	mouseState.buttonPrevious = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////