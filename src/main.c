#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>

int main() {
    // Init connection
    int screen_number;
    xcb_connection_t *connection = xcb_connect(NULL, &screen_number);
    int failure = xcb_connection_has_error(connection);
    if (failure) {
        fprintf(stderr, "could not open connection to X server: (error code %i)\n", failure);
        return failure;
    }

    // Setup screen
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(connection));

    xcb_screen_t *screen;
    uint32_t mask = XCB_CW_EVENT_MASK;
	uint32_t values[] = {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
		| XCB_EVENT_MASK_STRUCTURE_NOTIFY
		| XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
		| XCB_EVENT_MASK_PROPERTY_CHANGE
    };
    for (; screen_iter.rem; screen_number--, xcb_screen_next(&screen_iter)) {
        screen = screen_iter.data;
        if (screen != NULL) {
	        //xcb_change_window_attributes(connection, screen->root, mask, values);
        }
    }

    // Create cursor
    xcb_font_t font = xcb_generate_id(connection);
    xcb_open_font(connection, font, strlen("cursor"), "cursor");

    xcb_cursor_t cursor = xcb_generate_id(connection);
    xcb_create_glyph_cursor(connection,
                            cursor,
                            font, font,
                            68, 69,
                            0, 0, 0,
                            0, 0, 0);
    mask = XCB_CW_CURSOR;
    values[0] = cursor;
    xcb_change_window_attributes(connection, screen->root, mask, values);

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(connection))) {
        printf("%x\n", e->response_type);
        free(e);
    }

    xcb_free_cursor(connection, cursor);
    xcb_close_font(connection, font);
    xcb_disconnect(connection);
}
