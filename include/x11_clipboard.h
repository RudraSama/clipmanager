#ifndef X11_CLIPBOARD_H_
#define X11_CLIPBOARD_H_

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <clipqueue.h>
#include <file_manager.h>

/* Chunk for XGetWindowProperty
 * 1 Chunk = 4 bytes
 * */
#define CHUNK 4096  // 4096 * 4 = 16384


void become_clipboard_owner(Display *d, Window w);

void read_clipboard_data(Display *d, Window w, Atom clip, Atom property,
                         Atom target, File_t *f, ClipQueue **clipqueue);

void print_supported_targets(Display *d, Window w);

void send_selection_notify_event(Display *d, Window w, XEvent *ev, int count);

#endif
