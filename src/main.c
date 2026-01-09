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
	if (d == NULL) return 1;
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

	Stack *stack = stack_init();
	if (stack == NULL) {
		goto close_file;
	}

	read_indexs(&f, &stack);


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
					traverse(&stack);
					break;
				case 33:
					// print_supported_targets(d, w);
					read_clipboard_data(d, w, clip, property, target, &f,
					                    &stack);
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

	stack_deinit(&stack);

close_file:
	file_deinit(&f);

close_window:
	XDestroyWindow(d, w);
	XCloseDisplay(d);
	return 0;
}
