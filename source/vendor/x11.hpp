#pragma once
#include <vendor/config.hpp>

#if !USE_CUSTOM_C_HEADERS
	#include <vendor/conflicts.hpp>
		#include <X11/X.h>
		#include <X11/Xatom.h>
		#include <X11/Xlib.h>
		#include <X11/Xutil.h>
		#include <X11/XKBlib.h>
		#include <X11/extensions/XI2.h>
		#include <X11/extensions/XInput2.h>
	#include <vendor/conflicts.hpp>
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// X.h

	using XID = unsigned long;
	using XColormap = XID;
	using XCursor = XID;
	using XWindow = XID;
	using KeySym = XID;
	using Pixmap = XID;

	#define KeyPress 2
	#define KeyRelease 3
	#define ButtonPress 4
	#define ButtonRelease 5
	#define MotionNotify 6
	#define EnterNotify 7
	#define LeaveNotify 8
	#define FocusIn 9
	#define FocusOut 10
	#define KeymapNotify 11
	#define Expose 12
	#define GraphicsExpose 13
	#define NoExpose 14
	#define VisibilityNotify 15
	#define CreateNotify 16
	#define DestroyNotify 17
	#define UnmapNotify 18
	#define MapNotify 19
	#define MapRequest 20
	#define ReparentNotify 21
	#define ConfigureNotify 22
	#define ConfigureRequest 23
	#define GravityNotify 24
	#define ResizeRequest 25
	#define CirculateNotify 26
	#define CirculateRequest 27
	#define PropertyNotify 28
	#define SelectionClear 29
	#define SelectionRequest 30
	#define SelectionNotify 31
	#define ColormapNotify 32
	#define ClientMessage 33
	#define MappingNotify 34
	#define GenericEvent 35
	#define LASTEvent 36

	#define None 0
	#define InputOutput 1
	#define AllocNone 0

	#define SelectionClear 29
	#define SelectionRequest 30
	#define SelectionNotify 31

	#define KeyPressMask (1 << 0)
	#define KeyReleaseMask (1 << 1)
	#define ButtonPressMask (1 << 2)
	#define ButtonReleaseMask (1 << 3)
	#define PointerMotionMask (1 << 6)
	#define StructureNotifyMask (1 << 17)
	#define SubstructureNotifyMask (1 << 19)
	#define SubstructureRedirectMask (1 << 20)
	#define FocusChangeMask (1 << 21)

	#define Button1 1
	#define Button2 2
	#define Button3 3
	#define Button4 4
	#define Button5 5

	#define PropModeReplace 0
	#define PropModePrepend 1
	#define PropModeAppend 2

	#define CurrentTime 0L
	#define AnyPropertyType 0L

	#define CWBackPixel (1 << 1)
	#define CWBorderPixel (1 << 3)
	#define CWEventMask (1 << 11)
	#define CWColormap (1 << 13)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Xatom.h

	using XAtom = unsigned long;

	#define XA_ATOM ((XAtom)4)
	#define XA_STRING ((XAtom)31)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Xlib.h

	using Bool = int;
	using XPointer = char *;
	using XTime = unsigned long;
	using XVisualID = unsigned long;
	struct XDisplay;
	struct XVisual;
	struct XComposeStatus;

	#define QueuedAlready 0
	#define QueuedAfterReading 1
	#define QueuedAfterFlush 2

	#define ScreenOfDisplay(dpy, scr) (&(dpy->screens[scr]))
	#define DefaultScreen(dpy) (dpy->default_screen)
	#define DefaultScreenOfDisplay(dpy) ScreenOfDisplay(dpy,DefaultScreen(dpy))
	#define DefaultRootWindow(dpy) (ScreenOfDisplay(dpy, dpy->default_screen)->root)
	#define DisplayWidth(dpy, scr) (ScreenOfDisplay(dpy, scr)->width)
	#define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy, scr)->height)

	#define BlackPixel(dpy, scr) (ScreenOfDisplay(dpy, scr)->black_pixel)
	#define WhitePixel(dpy, scr) (ScreenOfDisplay(dpy, scr)->white_pixel)

	struct XScreen
	{
		void *ext_data;
		XDisplay *display;
		XWindow root;
		int width;
		int height;
		int mwidth;
		int mheight;
		int ndepths;
		void *depths;
		int root_depth;
		void *root_visual;
		void *default_gc;
		XColormap cmap;
		unsigned long white_pixel;
		unsigned long black_pixel;
		int max_maps;
		int min_maps;
		int backing_store;
		int save_unders;
		long root_input_mask;
	};

	struct XDisplay
	{
		void *ext_data;
		void *private1;
		int fd;
		int private2;
		int proto_major_version;
		int proto_minor_version;
		char *vendor;
		XID private3;
		XID private4;
		XID private5;
		int private6;
		XID (*resource_alloc)
		(struct XDisplay *);
		int byte_order;
		int bitmap_unit;
		int bitmap_pad;
		int bitmap_bit_order;
		int nformats;
		void *pixmap_format;
		int private8;
		int release;
		void *private9;
		void *private10;
		int qlen;
		unsigned long last_request_read;
		unsigned long request;
		XPointer private11;
		XPointer private12;
		XPointer private13;
		XPointer private14;
		unsigned max_request_size;
		void *db;
		int (*private15)(struct XDisplay *);
		char *display_name;
		int default_screen;
		int nscreens;
		XScreen *screens;
		unsigned long motion_buffer;
		unsigned long private16;
		int min_keycode;
		int max_keycode;
		XPointer private17;
		XPointer private18;
		int private19;
		char *xdefaults;
	};

	struct XSetWindowAttributes
	{
		Pixmap background_pixmap;
		unsigned long background_pixel;
		Pixmap border_pixmap;
		unsigned long border_pixel;
		int bit_gravity;
		int win_gravity;
		int backing_store;
		unsigned long backing_planes;
		unsigned long backing_pixel;
		int save_under;
		long event_mask;
		long do_not_propagate_mask;
		int override_redirect;
		XColormap colormap;
		XCursor cursor;
	};

	struct XKeyEvent
	{
		int type;
		unsigned long serial;
		int send_event;
		XDisplay *display;
		XWindow window;
		XWindow root;
		XWindow subwindow;
		XTime xtime;
		int x, y;
		int x_root, y_root;
		unsigned int state;
		unsigned int keycode;
		int same_screen;
	};

	struct XButtonEvent
	{
		int type;
		unsigned long serial;
		int send_event;
		XDisplay *display;
		XWindow window;
		XWindow root;
		XWindow subwindow;
		XTime xtime;
		int x, y;
		int x_root, y_root;
		unsigned int state;
		unsigned int button;
		int same_screen;
	};

	struct XMotionEvent
	{
		int type;
		unsigned long serial;
		int send_event;
		XDisplay *display;
		XWindow window;
		XWindow root;
		XWindow subwindow;
		XTime xtime;
		int x, y;
		int x_root, y_root;
		unsigned int state;
		char is_hint;
		int same_screen;
	};


	struct XConfigureEvent
	{
		int type;
		unsigned long serial;
		int send_event;
		XDisplay *display;
		XWindow event;
		XWindow window;
		int x, y;
		int width, height;
		int border_width;
		XWindow above;
		int override_redirect;
	};

	struct XClientMessageEvent
	{
		int type;
		unsigned long serial;
		int send_event;
		XDisplay *display;
		XWindow window;
		XAtom message_type;
		int format;
		union
		{
			char b[20];
			short s[10];
			long l[5];
		} data;
	};

	struct XSelectionRequestEvent
	{
		int type;
		unsigned long serial;
		Bool send_event;
		XDisplay *display;
		XWindow owner;
		XWindow requestor;
		XAtom selection;
		XAtom target;
		XAtom property;
		XTime xtime;
	};

	struct XSelectionEvent
	{
		int type;
		unsigned long serial;
		Bool send_event;
		XDisplay *display;
		XWindow requestor;
		XAtom selection;
		XAtom target;
		XAtom property;
		XTime xtime;
	};

	struct XGenericEventCookie
	{
		int type;
		unsigned long serial;
		Bool send_event;
		XDisplay *display;
		int extension;
		int evtype;
		unsigned int cookie;
		void *data;
	};

	union XEvent
	{
		int type;
		XKeyEvent xkey;
		XButtonEvent xbutton;
		XMotionEvent xmotion;
		XConfigureEvent xconfigure;
		XClientMessageEvent xclient;
		XSelectionRequestEvent xselectionrequest;
		XSelectionEvent xselection;
		XGenericEventCookie xcookie;
		// ...
		long pad[24];
	};

	extern "C" XColormap XCreateColormap( XDisplay *, XWindow, XVisual *, int );
	extern "C" XWindow XCreateWindow( XDisplay *, XWindow, int, int, unsigned int, unsigned int, unsigned int, int,
		unsigned int, XVisual *, unsigned long, XSetWindowAttributes * );
	extern "C" int XFilterEvent( XEvent *, XWindow );
	extern "C" int XFree( void * );
	extern "C" XAtom XInternAtom( XDisplay *, const char *, int );
	extern "C" KeySym XLookupKeysym( XKeyEvent *, int );
	extern "C" int XMapRaised( XDisplay *, XWindow );
	extern "C" int XNextEvent( XDisplay *, XEvent * );
	extern "C" XDisplay *XOpenDisplay( const char * );
	extern "C" int XPending( XDisplay * );
	extern "C" int XSendEvent( XDisplay *, XWindow, int, long, XEvent * );
	extern "C" int XSetWMProtocols( XDisplay *, XWindow, XAtom *, int);
	extern "C" int XStoreName( XDisplay *, XWindow, const char *);
	extern "C" int XSync( XDisplay *, int );
	extern "C" int XQueryPointer( XDisplay *, XWindow, XWindow *, XWindow *, int *, int *, int *, int *,
		unsigned int * );
	extern "C" int XWarpPointer( XDisplay *, XWindow, XWindow, int, int, unsigned int, unsigned int, int, int );
	extern "C" int XMoveResizeWindow( XDisplay *, XWindow, int, int, unsigned int, unsigned int );
	extern "C" int XEventsQueued( XDisplay *, int );
	extern "C" int XPeekEvent( XDisplay *, XEvent * );
	extern "C" XWindow XGetSelectionOwner( XDisplay *, XAtom );
	extern "C" int XChangeProperty( XDisplay *, XWindow, XAtom, XAtom, int, int, unsigned char *, int );
	extern "C" int XSetSelectionOwner( XDisplay *, XAtom, XWindow, XTime );
	extern "C" int XConvertSelection( XDisplay *, XAtom, XAtom, XAtom, XWindow, XTime );
	extern "C" int XGetWindowProperty( XDisplay *, XWindow, XAtom, long, long, Bool, XAtom, XAtom *, int *,
		unsigned long *, unsigned long *, unsigned char ** );
	extern "C" int XResizeWindow( XDisplay *, XWindow, unsigned int, unsigned int );
	extern "C" int XMoveWindow( XDisplay *, XWindow, int, int );
	extern "C" int XFlush( XDisplay * );
	extern "C" Bool XQueryExtension( XDisplay *, const char* , int *, int *, int * );
	extern "C" Bool XGetEventData( XDisplay *, XGenericEventCookie * );
	extern "C" void XFreeEventData( XDisplay *, XGenericEventCookie * );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Xutil.h

	#define PMinSize (1 << 4)
	#define PMaxSize (1 << 5)

	struct XSizeHints
	{
		long flags;
		int x, y;
		int width, height;
		int min_width, min_height;
		int max_width, max_height;
		int width_inc, height_inc;
		struct { int x; int y; } min_aspect, max_aspect;
		int base_width, base_height;
		int win_gravity;
	};

	struct XVisualInfo
	{
		XVisual *visual;
		XVisualID visualid;
		int screen;
		int depth;
		int c_class;
		unsigned long red_mask;
		unsigned long green_mask;
		unsigned long blue_mask;
		int colormap_size;
		int bits_per_rgb;
	};

	extern "C" void XSetWMNormalHints( XDisplay *, XWindow, struct XSizeHints * );
	extern "C" int XLookupString( XKeyEvent *, char *, int, KeySym *, XComposeStatus * );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XKBlib.h

	extern "C" Bool XkbSetDetectableAutoRepeat( XDisplay *, Bool, Bool * );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XI2.h

	#define XISetMask(ptr, event) (((unsigned char*)(ptr))[(event)>>3] |=  (1 << ((event) & 7)))
	#define XIClearMask(ptr, event) (((unsigned char*)(ptr))[(event)>>3] &= ~(1 << ((event) & 7)))
	#define XIMaskIsSet(ptr, event) (((unsigned char*)(ptr))[(event)>>3] &   (1 << ((event) & 7)))
	#define XIMaskLen(event) (((event) >> 3) + 1)

	#define XIAllDevices 0
	#define XIAllMasterDevices 1

	#define XI_DeviceChanged 1
	#define XI_KeyPress 2
	#define XI_KeyRelease 3
	#define XI_ButtonPress 4
	#define XI_ButtonRelease 5
	#define XI_Motion 6
	#define XI_Enter 7
	#define XI_Leave 8
	#define XI_FocusIn 9
	#define XI_FocusOut 10
	#define XI_HierarchyChanged 11
	#define XI_PropertyEvent 12
	#define XI_RawKeyPress 13
	#define XI_RawKeyRelease 14
	#define XI_RawButtonPress 15
	#define XI_RawButtonRelease 16
	#define XI_RawMotion 17
	#define XI_TouchBegin 18 /* XI 2.2 */
	#define XI_TouchUpdate 19
	#define XI_TouchEnd 20
	#define XI_TouchOwnership 21
	#define XI_RawTouchBegin 22
	#define XI_RawTouchUpdate 23
	#define XI_RawTouchEnd 24
	#define XI_BarrierHit 25 /* XI 2.3 */
	#define XI_BarrierLeave 26
	#define XI_GesturePinchBegin 27 /* XI 2.4 */
	#define XI_GesturePinchUpdate 28
	#define XI_GesturePinchEnd 29
	#define XI_GestureSwipeBegin 30
	#define XI_GestureSwipeUpdate 31
	#define XI_GestureSwipeEnd 32
	#define XI_LASTEVENT XI_GestureSwipeEnd

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// XInput2.h

	struct XIEventMask
	{
		int deviceid;
		int mask_len;
		unsigned char *mask;
	};

	struct XIValuatorState
	{
		int mask_len;
		unsigned char *mask;
		double *values;
	};

	struct XIRawEvent
	{
		int type;
		unsigned long serial;
		Bool send_event;
		XDisplay *display;
		int extension;
		int evtype;
		XTime time;
		int deviceid;
		int sourceid;
		int detail;
		int flags;
		XIValuatorState valuators;
		double *raw_values;
	};

	extern "C" int XISelectEvents( XDisplay *, XWindow, XIEventMask *, int );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif