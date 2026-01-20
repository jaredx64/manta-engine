#pragma once
#include <vendor/config.hpp>

#if USE_OFFICIAL_HEADERS
	#include <vendor/conflicts.hpp>
		#include <X11/Xlib.h>
		#include <X11/Xutil.h>
		#include <X11/Xatom.h>
		#include <X11/XKBlib.h>
	#include <vendor/conflicts.hpp>
#else
	using Bool = int;
	using XAtom = unsigned long;
	using XTime = unsigned long;
	using XVisualID = unsigned long;
	using XID = unsigned long;
	using XPointer = char *;
	using XColormap = XID;
	using XCursor = XID;
	using XWindow = XID;
	using KeySym = XID;
	using Pixmap = XID;

	#define XA_ATOM ((XAtom)4)
	#define XA_STRING ((XAtom)31)

	#define ScreenOfDisplay(dpy, scr) (&(dpy->screens[scr]))
	#define DefaultScreen(dpy) (dpy->default_screen)
	#define DefaultScreenOfDisplay(dpy) ScreenOfDisplay(dpy,DefaultScreen(dpy))
	#define DefaultRootWindow(dpy) (ScreenOfDisplay(dpy, dpy->default_screen)->root)
	#define DisplayWidth(dpy, scr) (ScreenOfDisplay(dpy, scr)->width)
	#define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy, scr)->height)

	#define BlackPixel(dpy, scr) (ScreenOfDisplay(dpy, scr)->black_pixel)
	#define WhitePixel(dpy, scr) (ScreenOfDisplay(dpy, scr)->white_pixel)

	#define CWBackPixel (1 << 1)
	#define CWBorderPixel (1 << 3)
	#define CWEventMask (1 << 11)
	#define CWColormap (1 << 13)

	#define PMinSize (1 << 4)
	#define PMaxSize (1 << 5)

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

	#define KeyPress 2
	#define KeyRelease 3
	#define ButtonPress 4
	#define ButtonRelease 5
	#define MotionNotify 6
	#define FocusIn 9
	#define FocusOut 10
	#define ConfigureNotify 22
	#define ClientMessage 33

	#define None 0
	#define InputOutput 1
	#define AllocNone 0

	#define QueuedAlready 0
	#define QueuedAfterReading 1
	#define QueuedAfterFlush 2

	#define SelectionClear 29
	#define SelectionRequest 30
	#define SelectionNotify 31

	#define PropModeReplace 0
	#define PropModePrepend 1
	#define PropModeAppend 2

	#define CurrentTime 0L
	#define AnyPropertyType 0L

	struct XDisplay;
	struct XVisual;
	struct XComposeStatus;

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
		// ...
		long pad[24];
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

	struct XSizeHints
	{
		long flags;
		int x, y;
		int width, height;
		int min_width, min_height;
		int max_width, max_height;
		int width_inc, height_inc;
		struct
		{
			int x;
			int y;
		} min_aspect, max_aspect;
		int base_width, base_height;
		int win_gravity;
	};

	extern "C" XColormap XCreateColormap(XDisplay *, XWindow, XVisual *, int);
	extern "C" XWindow XCreateWindow(XDisplay *, XWindow, int, int, unsigned int, unsigned int, unsigned int, int,
		unsigned int, XVisual *, unsigned long, XSetWindowAttributes *);
	extern "C" int XFilterEvent(XEvent *, XWindow);
	extern "C" int XFree(void *);
	extern "C" XAtom XInternAtom(XDisplay *, const char *, int);
	extern "C" KeySym XLookupKeysym(XKeyEvent *, int);
	extern "C" int XMapRaised(XDisplay *, XWindow);
	extern "C" int XNextEvent(XDisplay *, XEvent *);
	extern "C" XDisplay *XOpenDisplay(const char *);
	extern "C" int XPending(XDisplay *);
	extern "C" int XSendEvent(XDisplay *, XWindow, int, long, XEvent *);
	extern "C" void XSetWMNormalHints(XDisplay *, XWindow, struct XSizeHints *);
	extern "C" int XSetWMProtocols(XDisplay *, XWindow, XAtom *, int);
	extern "C" int XStoreName(XDisplay *, XWindow, const char *);
	extern "C" int XSync(XDisplay *, int);
	extern "C" int XQueryPointer(XDisplay *, XWindow, XWindow *, XWindow *, int *, int *, int *, int *,
		unsigned int *);
	extern "C" int XWarpPointer(XDisplay *, XWindow, XWindow, int, int, unsigned int, unsigned int, int, int);
	extern "C" int XLookupString(XKeyEvent *, char *, int, KeySym *, XComposeStatus *);
	extern "C" int XMoveResizeWindow(XDisplay *, XWindow, int, int, unsigned int, unsigned int);
	extern "C" Bool XkbSetDetectableAutoRepeat(XDisplay *, Bool, Bool *);
	extern "C" int XEventsQueued(XDisplay *, int);
	extern "C" int XPeekEvent(XDisplay *, XEvent *);
	extern "C" XWindow XGetSelectionOwner(XDisplay *, XAtom);
	extern "C" int XChangeProperty(XDisplay *, XWindow, XAtom, XAtom, int, int, unsigned char *, int);
	extern "C" int XSetSelectionOwner(XDisplay *, XAtom, XWindow, XTime);
	extern "C" int XConvertSelection(XDisplay *, XAtom, XAtom, XAtom, XWindow, XTime);
	extern "C" int XGetWindowProperty(XDisplay *, XWindow, XAtom, long, long, Bool, XAtom, XAtom *, int *,
		unsigned long *, unsigned long *, unsigned char **);
	extern "C" int XResizeWindow(XDisplay *, XWindow, unsigned int, unsigned int);
	extern "C" int XMoveWindow(XDisplay *, XWindow, int, int);
	extern "C" int XFlush(XDisplay *);
#endif