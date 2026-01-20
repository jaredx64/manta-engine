#pragma once

#include <config.hpp>

#include <core/debug.hpp>
#include <core/types.hpp>
#include <core/math.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Keyboard
{
private:
	static Keyboard *keyboard;

public:
	Keyboard()
	{
		keyboard_clear();
	}

#if COMPILE_DEBUG
	~Keyboard()
	{
		if( !Debug::memoryLeakDetection ) { return; }
		AssertMsg( Keyboard::keyboard != this, "Keyboard: destructor called for primary bound Keyboard!" );
	}
#endif

	struct State
	{
		bool keyCurrent[256];
		bool keyPrevious[256];
		bool keyRepeat[256];
		float keyRepeatTimer[256];
		char inputBuffer[16];
	};

	static State &state();

	static void reset_active();
	static void set_active( Keyboard &keyboard );

	static bool check( u8 key ) { return keyboard->keyboard_check( key ); }
	static bool check_pressed( u8 key ) { return keyboard->keyboard_check_pressed( key ); }
	static bool check_pressed_repeat( u8 key ) { return keyboard->keyboard_check_pressed_repeat( key ); }
	static bool check_released( u8 key ) { return keyboard->keyboard_check_released( key ); }
	static bool check_any() { return keyboard->keyboard_check_any(); }
	static bool check_pressed_any() { return keyboard->keyboard_check_pressed_any(); }
	static bool check_pressed_repeat_any() { return keyboard->keyboard_check_pressed_repeat_any(); }
	static bool check_released_any() { return keyboard->keyboard_check_released_any(); }

	static bool has_input() { return keyboard->keyboard_has_input(); }
	static char *input_buffer() { return keyboard->keyboard_input_buffer(); }

	static void update( Delta delta ) { return keyboard->keyboard_update( delta ); }
	static void clear() { return keyboard->keyboard_clear(); }

private:
	bool keyboard_check( u8 key );
	bool keyboard_check_pressed( u8 key );
	bool keyboard_check_pressed_repeat( u8 key );
	bool keyboard_check_released( u8 key );

	bool keyboard_check_any();
	bool keyboard_check_pressed_any();
	bool keyboard_check_pressed_repeat_any();
	bool keyboard_check_released_any();

	bool keyboard_has_input();
	char *keyboard_input_buffer();

	void keyboard_update( Delta delta );
	void keyboard_clear();

public:
	State keyboardState;
};


enum
{
#if WINDOW_WINDOWS || WINDOW_NONE
	vk_tab = 0x09,
	vk_enter = 0x0D,
	vk_backspace = 0x08,
	vk_shift = 0x10,
	vk_control = 0x11,
	vk_command = vk_control,
	vk_escape = 0x1B,
	vk_space = 0x20,
	vk_equal = 0xBB,
	vk_minus = 0xBD,
	vk_pageup = 0x21,
	vk_pagedown = 0x22,
	vk_end = 0x23,
	vk_home = 0x24,
	vk_insert = 0x2D,
	vk_delete = 0x2E,
	vk_alt = 0x12,
	vk_capslock = 0x14,
	vk_up = 0x26,
	vk_right = 0x27,
	vk_down = 0x28,
	vk_left = 0x25,
	vk_f1 = 0x70,
	vk_f2 = 0x71,
	vk_f3 = 0x72,
	vk_f4 = 0x73,
	vk_f5 = 0x74,
	vk_f6 = 0x75,
	vk_f7 = 0x76,
	vk_f8 = 0x77,
	vk_f9 = 0x78,
	vk_f10 = 0x79,
	vk_f11 = 0x7A,
	vk_f12 = 0x7B,
	vk_tilde = 0xC0,
	vk_keypad0 = 0x2D,
	vk_keypad1 = 0x23,
	vk_keypad2 = 0x28,
	vk_keypad3 = 0x22,
	vk_keypad4 = 0x25,
	vk_keypad5 = 0x0C,
	vk_keypad6 = 0x27,
	vk_keypad7 = 0x24,
	vk_keypad8 = 0x26,
	vk_keypad9 = 0x21,
	vk_a = 'A',
	vk_b = 'B',
	vk_c = 'C',
	vk_d = 'D',
	vk_e = 'E',
	vk_f = 'F',
	vk_g = 'G',
	vk_h = 'H',
	vk_i = 'I',
	vk_j = 'J',
	vk_k = 'K',
	vk_l = 'L',
	vk_m = 'M',
	vk_n = 'N',
	vk_o = 'O',
	vk_p = 'P',
	vk_q = 'Q',
	vk_r = 'R',
	vk_s = 'S',
	vk_t = 'T',
	vk_u = 'U',
	vk_v = 'V',
	vk_w = 'W',
	vk_x = 'X',
	vk_y = 'Y',
	vk_z = 'Z',
	vk_0 = '0',
	vk_1 = '1',
	vk_2 = '2',
	vk_3 = '3',
	vk_4 = '4',
	vk_5 = '5',
	vk_6 = '6',
	vk_7 = '7',
	vk_8 = '8',
	vk_9 = '9',
#elif WINDOW_COCOA
	vk_tab = 0x30,
	vk_enter = 0x24,
	vk_delete = 0x75,
	vk_backspace = 0x33,
	vk_pageup = 0x74,
	vk_pagedown = 0x79,
	vk_end = 0x77,
	vk_home = 0x73,
	vk_alt = 0xFC,
	vk_shift = 0xFE,
	vk_control = 0xFF,
	vk_command = 0xFD,
	vk_escape = 0x35,
	vk_space = 0x31,
	vk_equal = 0x18,
	vk_minus = 0x1B,
	vk_up = 0x7E,
	vk_right = 0x7C,
	vk_down = 0x7D,
	vk_left = 0x7B,
	vk_f1 = 0x7A,
	vk_f2 = 0x78,
	vk_f3 = 0x63,
	vk_f4 = 0x76,
	vk_f5 = 0x60,
	vk_f6 = 0x61,
	vk_f7 = 0x62,
	vk_f8 = 0x64,
	vk_f9 = 0x65,
	vk_f10 = 0x6D,
	vk_f11 = 0x67,
	vk_f12 = 0x6F,
	vk_tilde = 0x32,
	vk_keypad0 = 0,
	vk_keypad1 = 0,
	vk_keypad2 = 0,
	vk_keypad3 = 0,
	vk_keypad4 = 0,
	vk_keypad5 = 0,
	vk_keypad6 = 0,
	vk_keypad7 = 0,
	vk_keypad8 = 0,
	vk_keypad9 = 0,
	vk_a = 0x00,
	vk_b = 0x0B,
	vk_c = 0x08,
	vk_d = 0x02,
	vk_e = 0x0E,
	vk_f = 0x03,
	vk_g = 0x05,
	vk_h = 0x04,
	vk_i = 0x22,
	vk_j = 0x26,
	vk_k = 0x28,
	vk_l = 0x25,
	vk_m = 0x2E,
	vk_n = 0x2D,
	vk_o = 0x1F,
	vk_p = 0x23,
	vk_q = 0x0C,
	vk_r = 0x0F,
	vk_s = 0x01,
	vk_t = 0x11,
	vk_u = 0x20,
	vk_v = 0x09,
	vk_w = 0x0D,
	vk_x = 0x07,
	vk_y = 0x10,
	vk_z = 0x06,
	vk_0 = 0x1D,
	vk_1 = 0x12,
	vk_2 = 0x13,
	vk_3 = 0x14,
	vk_4 = 0x15,
	vk_5 = 0x17,
	vk_6 = 0x16,
	vk_7 = 0x1A,
	vk_8 = 0x1C,
	vk_9 = 0x19,
#elif WINDOW_X11
	vk_alt = 0xE9,
	vk_alt_r = 0xEA,
	vk_tab = 0x09,
	vk_pageup = 0x55,
	vk_pagedown = 0x56,
	vk_end = 0x57,
	vk_home = 0x50,
	vk_delete = 0xFF,
	vk_enter = 0x0D,
	vk_backspace = 0x08,
	vk_shift = 0xE1,
	vk_shift_r = 0xE2,
	vk_control = 0xE3,
	vk_control_r = 0xE4,
	vk_command = vk_control,
	vk_escape = 0x1B,
	vk_space = 0x20,
	vk_equal = 0x3D,
	vk_minus = 0x2D,
	vk_up = 0x52,
	vk_right = 0x53,
	vk_down = 0x54,
	vk_left = 0x51,
	vk_f1 = 0xBE,
	vk_f2 = 0xBF,
	vk_f3 = 0xC0,
	vk_f4 = 0xC1,
	vk_f5 = 0xC2,
	vk_f6 = 0xC3,
	vk_f7 = 0xC4,
	vk_f8 = 0xC5,
	vk_f9 = 0xC6,
	vk_f10 = 0xC7,
	vk_f11 = 0xC8,
	vk_f12 = 0xC9,
	vk_tilde = 0x60,
	vk_keypad0 = 0x9E,
	vk_keypad1 = 0x9C,
	vk_keypad2 = 0x99,
	vk_keypad3 = 0x9B,
	vk_keypad4 = 0x96,
	vk_keypad5 = 0x9D,
	vk_keypad6 = 0x98,
	vk_keypad7 = 0x95,
	vk_keypad8 = 0x97,
	vk_keypad9 = 0x9A,
	vk_a = 'a',
	vk_b = 'b',
	vk_c = 'c',
	vk_d = 'd',
	vk_e = 'e',
	vk_f = 'f',
	vk_g = 'g',
	vk_h = 'h',
	vk_i = 'i',
	vk_j = 'j',
	vk_k = 'k',
	vk_l = 'l',
	vk_m = 'm',
	vk_n = 'n',
	vk_o = 'o',
	vk_p = 'p',
	vk_q = 'q',
	vk_r = 'r',
	vk_s = 's',
	vk_t = 't',
	vk_u = 'u',
	vk_v = 'v',
	vk_w = 'w',
	vk_x = 'x',
	vk_y = 'y',
	vk_z = 'z',
	vk_0 = '0',
	vk_1 = '1',
	vk_2 = '2',
	vk_3 = '3',
	vk_4 = '4',
	vk_5 = '5',
	vk_6 = '6',
	vk_7 = '7',
	vk_8 = '8',
	vk_9 = '9',
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mouse
{
private:
	static Mouse *mouse;

public:
	Mouse()
	{
		mouse_clear();
	}

#if COMPILE_DEBUG
	~Mouse()
	{
		if( !Debug::memoryLeakDetection ) { return; }
		AssertMsg( Mouse::mouse != this, "Mouse: destructor called for primary bound Mouse!" );
	}
#endif

	struct State
	{
		float positionX, positionY;
		float positionXPrevious, positionYPrevious;
		int wheelX, wheelY;
		float wheelXVelocity, wheelYVelocity;
		int buttonCurrent, buttonPrevious;
	};

	static State &state();

	static void reset_active();
	static void set_active( Mouse &mouse );

	static float x();
	static float y();
	static float x_previous();
	static float y_previous();

	static float x_logical();
	static float y_logical();
	static float x_logical_previous();
	static float y_logical_previous();

	static bool check( u8 button ) { return mouse->mouse_check( button ); }
	static bool check_pressed( u8 button ) { return mouse->mouse_check_pressed( button ); }
	static bool check_released( u8 button ) { return mouse->mouse_check_released( button ); }

	static bool check_wheel_up() { return mouse->mouse_check_wheel_up(); }
	static bool check_wheel_down() { return mouse->mouse_check_wheel_down(); }
	static bool check_wheel_left() { return mouse->mouse_check_wheel_left(); }
	static bool check_wheel_right() { return mouse->mouse_check_wheel_right(); }

	static float wheel_velocity_x() { return mouse->mouse_wheel_velocity_x(); }
	static float wheel_velocity_y() { return mouse->mouse_wheel_velocity_y(); }

	static bool moved() { return mouse->mouse_moved(); }

	static void get_position_precise( double &x, double & y ) { mouse->mouse_get_position_precise( x, y ); }
	static void set_position( int x, int y ) { mouse->mouse_set_position( x, y ); }

	static void update( Delta delta ) { mouse->mouse_update( delta ); }
	static void clear() { mouse->mouse_clear(); }

private:
	bool mouse_check( u8 button );
	bool mouse_check_pressed( u8 button );
	bool mouse_check_released( u8 button );

	bool mouse_check_wheel_up();
	bool mouse_check_wheel_down();
	bool mouse_check_wheel_left();
	bool mouse_check_wheel_right();

	float mouse_wheel_velocity_x();
	float mouse_wheel_velocity_y();

	bool mouse_moved();

	void mouse_get_position_precise( double &x, double &y );
	void mouse_set_position( int x, int y );

	void mouse_update( Delta delta );
	void mouse_clear();

public:
	State mouseState;
};


enum
{
#if WINDOW_WINDOWS || WINDOW_NONE
	mb_left = 1 << 0,
	mb_right = 1 << 1,
	mb_middle = 1 << 4,
#elif WINDOW_COCOA
	mb_left = 1 << 0,
	mb_right = 1 << 1,
	mb_middle = 1 << 2,
#elif WINDOW_X11
	mb_left = 1 << 1,
	mb_right = 1 << 3,
	mb_middle = 1 << 2,
#endif
};

#define mouse_x ( Mouse::x() )
#define mouse_y ( Mouse::y() )
#define mouse_x_prev ( Mouse::x_previous() )
#define mouse_y_prev ( Mouse::y_previous() )

#define mouse_x_logical ( Mouse::x_logical() )
#define mouse_y_logical ( Mouse::y_logical() )
#define mouse_x_logical_prev ( Mouse::x_logical_previous() )
#define mouse_y_logical_prev ( Mouse::y_logical_previous() )

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////