#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <file_manager.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <x11_clipboard.h>

int main() {
	Display *d = XOpenDisplay(NULL);
	if (!d) return 1;
	Window w =
	    XCreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, 100, 100, 0, 0, 0);
	XSelectInput(d, w, KeyPressMask | KeyReleaseMask | PropertyChangeMask);
	XMapWindow(d, w);

	Atom clip = XInternAtom(d, "CLIPBOARD", True);

	Atom target = XInternAtom(d, "UTF8_STRING", False);
	Atom property = XInternAtom(d, "MY_PROP", False);

	File_t f;
	if (!file_init(&f, "clipboard", "index")) {
		goto close_window;
	}

	ClipQueue *clipqueue = clipqueue_init();
	if (!clipqueue) {
		goto close_file;
	}

	read_indexs(&f, &clipqueue);


	XEvent ev;
	int quit = 0;
	static int count = 0;
	while (!quit) {
		XNextEvent(d, &ev);
		if (ev.type == KeyPress) {
			XKeyEvent *kv = (XKeyEvent *)&ev;
			switch (kv->keycode) {
				case 24:
					quit = 1;
					break;
				case 28:
					traverse(&clipqueue);
					break;
				case 33:
					// print_supported_targets(d, w);
					read_clipboard_data(d, w, clip, property, target, &f,
					                    &clipqueue);
					break;
				default:
					printf("Keypressed %d\n", kv->keycode);
					break;
			}
		}
		if (ev.type == SelectionClear) {
			printf("Owner Changed\n");
		}
		if (ev.type == SelectionRequest) {
			send_selection_notify_event(d, w, &ev, count);
		}
	}

	clipqueue_deinit(&clipqueue);

close_file:
	file_deinit(&f);

close_window:
	XDestroyWindow(d, w);
	XCloseDisplay(d);
	return 0;
}
