#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <murmur_hash.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Chunk for XGetWindowProperty
 * 1 Chunk = 4 bytes
 * */
#define CHUNK 4096  // 4096 * 4 = 16384


void read_clipboard_data(Display *d, Window w, Atom clip, Atom property,
                         Atom target) {
	XConvertSelection(d, clip, target, property, w, CurrentTime);

	Atom type;
	int fmt;
	unsigned long nitems;
	unsigned long bytes_rem;
	unsigned char *data;

	unsigned long offset = 0;
	unsigned long total_bytes = 0;

	int INCR = 0;


	Murmur_t m;
	hash_init(&m);

	XEvent ev;
	while (1) {
		XNextEvent(d, &ev);
		if (ev.type == SelectionNotify ||
		    (INCR && ev.type == PropertyNotify && ev.xproperty.state == 0)) {
			if (!INCR) {
				XGetWindowProperty(d, w, property, 0, 0, False, AnyPropertyType,
				                   &type, &fmt, &nitems, &bytes_rem, &data);
				if (type == XInternAtom(d, "INCR", True)) {
					// Deleting Property to Initialize INCR stream
					// write_data_init
					XDeleteProperty(d, w, property);
					INCR = 1;
					continue;
				} else {
					// write_data_init
				}
			}

			do {
				// True will delete property (for INCR).
				XGetWindowProperty(d, w, property, offset * CHUNK, CHUNK,
				                   (INCR ? True : False), AnyPropertyType,
				                   &type, &fmt, &nitems, &bytes_rem, &data);
				offset++;
				total_bytes += nitems;
				if (data) {
					hash_update(&m, (const char *)data);
					// printf("%s\n", data);
					XFree(data);
				}

			} while (bytes_rem);

			offset = 0;
			if (nitems == 0 || !INCR) break;
		}
	}
	hash_finalize(&m);
	printf("%u\n", m.hash);
}

void print_supported_targets(Display *d, Window w) {
	Atom clip = XInternAtom(d, "CLIPBOARD", False);
	Atom target = XInternAtom(d, "TARGETS", False);
	Atom property = XInternAtom(d, "MY_PROP", False);
	XConvertSelection(d, clip, target, property, w, CurrentTime);

	XEvent ev;
	while (1) {
		XNextEvent(d, &ev);
		if (ev.type == SelectionNotify) {
			printf("Listing Targets -\n");
			if (ev.xselection.property) {
				Atom type;
				int fmt;
				unsigned long nitems, bytes_rem;
				unsigned char *data;
				XGetWindowProperty(d, w, property, 0, ~0, False, XA_ATOM, &type,
				                   &fmt, &nitems, &bytes_rem, &data);
				if (data) {
					Atom *supported = (Atom *)data;
					for (int i = 0; i < nitems; i++) {
						char *name = XGetAtomName(d, supported[i]);
						printf("%s\n", name);
						free(name);
					}
					free(data);
				}
				break;
			}
		}
	}
}

void send_selection_notify_event(Display *d, Window w, XEvent *ev, int count) {
	XSelectionRequestEvent *k = (XSelectionRequestEvent *)ev;

	printf("Target - %s\n", XGetAtomName(d, k->target));
	printf("Selection - %s\n", XGetAtomName(d, k->selection));
	printf("Property - %s\n", XGetAtomName(d, k->property));

	Atom utf8 = XInternAtom(d, "UTF8_STRING", False);
	Atom string = XInternAtom(d, "STRING", False);
	Atom targets = XInternAtom(d, "TARGETS", False);

	XEvent res;
	res.xselection.type = SelectionNotify;
	res.xselection.serial = k->serial;
	res.xselection.display = d;
	res.xselection.requestor = k->requestor;
	res.xselection.selection = k->selection;
	res.xselection.target = k->target;
	res.xselection.time = k->time;

	if (k->target == targets) {
		Atom data[16] = {utf8,
		                 string,
		                 XInternAtom(d, "TIMESTAMP", False),
		                 XInternAtom(d, "TARGETS", False),
		                 XInternAtom(d, "MULTIPLE", False),
		                 XInternAtom(d, "SAVE_TARGETS", False),
		                 XInternAtom(d, "text/html", False),
		                 XInternAtom(d, "text/_moz_htmlcontext", False),
		                 XInternAtom(d, "text/_moz_htmlinfo", False),
		                 XInternAtom(d, "UTF8_STRING", False),
		                 XInternAtom(d, "COMPOUND_TEXT", False),
		                 XInternAtom(d, "TEXT", False),
		                 XInternAtom(d, "STRING", False),
		                 XInternAtom(d, "text/plain;charset=utf-8", False),
		                 XInternAtom(d, "text/plain", False),
		                 XInternAtom(d, "text/x-moz-url-priv", False)};
		XChangeProperty(d, k->requestor, k->property, XA_ATOM, 32,
		                PropModeReplace, (unsigned char *)data, 16);
		res.xselection.property = k->property;
	} else if (k->target == utf8 || k->target == string) {
		//		Node node;
		//		if (head) {
		//			while (head->next) {
		//				head = head->next;
		//			}
		//			node = *head;
		//		} else {
		//			node.next = NULL;
		//			node.data = "HELLO";
		//			node.total_size = 5;
		//		}
		//		XChangeProperty(d, k->requestor, k->property, k->target, 8,
		//		                PropModeReplace, (unsigned char *)node.data,
		//		                node.total_size);
		res.xselection.property = k->property;
	} else {
		printf("I don't know man\n");
	}
	XSendEvent(d, k->requestor, False, 0, &res);
	XFlush(d);
}

void become_clipboard_owner(Display *d, Window w) {
	Atom clip = XInternAtom(d, "CLIPBOARD", True);
	Window owner;
	while ((owner = XGetSelectionOwner(d, clip)) != w) {
		XSetSelectionOwner(d, clip, w, CurrentTime);
	}
	printf("I am owner\n");
}

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
					break;
				case 33:
					// print_supported_targets(d, w);
					read_clipboard_data(d, w, clip, property, target);
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


close_window:
	XDestroyWindow(d, w);
	XCloseDisplay(d);
	return 0;
}
