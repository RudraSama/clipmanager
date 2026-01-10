#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <clip_metadata.h>
#include <clipqueue.h>
#include <file_manager.h>
#include <murmur_hash.h>
#include <stdlib.h>
#include <time_utils.h>
#include <x11_clipboard.h>

void become_clipboard_owner(Display *d, Window w) {
	Atom clip = XInternAtom(d, "CLIPBOARD", True);
	Window owner;
	while ((owner = XGetSelectionOwner(d, clip)) != w) {
		XSetSelectionOwner(d, clip, w, CurrentTime);
	}
	printf("I am owner\n");
}

void read_clipboard_data(Display *d, Window w, Atom clip, Atom property,
                         Atom target, File_t *f, ClipQueue **clipqueue) {
	XConvertSelection(d, clip, target, property, w, CurrentTime);

	Atom type;
	int fmt;
	unsigned long nitems;
	unsigned long bytes_rem;
	unsigned char *data;
	unsigned long offset = 0;

	size_t total_bytes = 0;
	uint64_t data_offset = 0;

	unsigned char *buffer = NULL;

	bool INCR = 0;

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
					init_write_data(f, 1, &data_offset);
					XDeleteProperty(d, w, property);
					INCR = 1;
					continue;
				}
			}

			do {
				Bool delete_property = (INCR ? True : False);

				XGetWindowProperty(d, w, property, offset, CHUNK,
				                   delete_property, AnyPropertyType, &type,
				                   &fmt, &nitems, &bytes_rem, &data);

				uint32_t items_bytes = nitems * (fmt / 8);

				// Advance offset by returned bytes in 32-bit units
				offset += (items_bytes + 3) / 4;
				total_bytes += items_bytes;

				if (data) {
					hash_update(&m, (const char *)data);

					if (INCR) {
						write_data(f, data, items_bytes);
					} else {
						if (!buffer) {
							size_t total_size = bytes_rem + items_bytes;
							buffer = malloc(total_size);
							if (!buffer) {
								XFree(data);
								return;
							}
						}

						memcpy((total_bytes - items_bytes) + buffer, data,
						       items_bytes);
					}

					XFree(data);
				}

			} while (bytes_rem);

			offset = 0;
			if (nitems == 0 || !INCR) break;
		}
	}

	hash_finalize(&m);

	ClipMetadata_t c = {.data = buffer,
	                    .timestamp = get_timestamp(),
	                    .hash = m.hash,
	                    .data_size = total_bytes,
	                    .offset = data_offset,
	                    .new_clip = true,
	                    .is_INCR = INCR};
	add(clipqueue, &c);

	if (INCR) {
		finish_write_data(f);
		Index_t index = {index.offset = (uint32_t)c.offset,
		                 index.timestamp = c.timestamp,
		                 index.data_size = c.data_size, index.hash = m.hash};
		write_index(f, &index);
	}

	if (buffer != NULL) {
		free(buffer);
	}
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
