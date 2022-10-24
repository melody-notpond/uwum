#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>

int spawn(char** args) {
    int pid = fork();

    if (pid == 0) {
        execvp(args[0], args);
        perror("failed to spawn process");
        exit(1);
    }

    return pid;
}

int main() {
    chdir(getenv("HOME"));
    perror("chdir");

    // Init connection
    int screen_number;
    xcb_connection_t *connection = xcb_connect(NULL, &screen_number);
    int failure = xcb_connection_has_error(connection);
    if (failure) {
        fprintf(stderr, "could not open connection to X server: (error code %i)\n", failure);
        return failure;
    }

    /*
    xcb_font_t font = xcb_generate_id(connection);
    xcb_open_font(connection, font, strlen("cursor"), "cursor");

    xcb_cursor_t cursor = xcb_generate_id(connection);
    xcb_create_glyph_cursor(connection,
                            cursor,
                            font, font,
                            68, 69,
                            0, 0, 0,
                            0, 0, 0);
    */
    uint32_t mask = XCB_CW_EVENT_MASK; // | XCB_CW_CURSOR;
    uint32_t values[] = {
        XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_ENTER_WINDOW
        | XCB_EVENT_MASK_LEAVE_WINDOW
        | XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE,
        //cursor
    };

    // Setup screen
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
    xcb_screen_t *screen;
    for (; screen_iter.rem; screen_number--, xcb_screen_next(&screen_iter)) {
        screen = screen_iter.data;
        if (screen != NULL) {
            xcb_void_cookie_t cookie = xcb_change_window_attributes_checked(connection, screen->root, mask, values);
            xcb_generic_error_t* error;
            if ((error = xcb_request_check(connection, cookie))) {
                fprintf(stderr, "error when assigning root: %i\n", error->error_code);
                free(error);
                xcb_disconnect(connection);
                return 1;
            }
        }
    }

    xcb_generic_event_t *e;
    while ((e = xcb_wait_for_event(connection))) {
        int status;
        waitpid(-1, &status, WNOHANG);
        switch (e->response_type) {
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose = (xcb_expose_event_t *) e;

                printf ("Window %i exposed. Region to be redrawn at location (%i,%i), with dimension (%i,%i)\n",
                        expose->window, expose->x, expose->y, expose->width, expose->height );
                break;
            }
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t *bp = (xcb_button_press_event_t *) e;
                // print_modifiers (bp->state);

                switch (bp->detail) {
                case 4:
                    printf ("Wheel Button up in window %i, at coordinates (%i,%i)\n",
                            bp->event, bp->event_x, bp->event_y );
                    break;
                case 5:
                    printf ("Wheel Button down in window %i, at coordinates (%i,%i)\n",
                            bp->event, bp->event_x, bp->event_y );
                    break;
                default:
                    printf ("Button %i pressed in window %i, at coordinates (%i,%i)\n",
                            bp->detail, bp->event, bp->event_x, bp->event_y );
                    break;
                }
                break;
            }
            case XCB_BUTTON_RELEASE: {
                xcb_button_release_event_t *br = (xcb_button_release_event_t *)e;
                // print_modifiers(br->state);

                printf ("Button %i released in window %i, at coordinates (%i,%i)\n",
                        br->detail, br->event, br->event_x, br->event_y );
                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)e;

                printf ("Mouse moved in window %i, at coordinates (%i,%i)\n",
                        motion->event, motion->event_x, motion->event_y );
                break;
            }
            case XCB_ENTER_NOTIFY: {
                xcb_enter_notify_event_t *enter = (xcb_enter_notify_event_t *)e;

                printf ("Mouse entered window %i, at coordinates (%i,%i)\n",
                        enter->event, enter->event_x, enter->event_y );
                break;
            }
            case XCB_LEAVE_NOTIFY: {
                xcb_leave_notify_event_t *leave = (xcb_leave_notify_event_t *)e;

                printf ("Mouse left window %i, at coordinates (%i,%i)\n",
                        leave->event, leave->event_x, leave->event_y );
                break;
            }
            case XCB_KEY_PRESS: {
                xcb_key_press_event_t *kp = (xcb_key_press_event_t *)e;
                // print_modifiers(kp->state);

                printf ("Key pressed in window %i\n",
                        kp->event);
                if (kp->detail == 36 && kp->state == XCB_MOD_MASK_4) {
                    spawn((char* []) { "kitty", NULL });
                }
                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t *kr = (xcb_key_release_event_t *)e;
                // print_modifiers(kr->state);

                printf ("Key released in window %i\n",
                        kr->event);
                break;
            }
            default:
                /* Unknown event type, ignore it */
                printf ("Unknown event: %i\n",
                        e->response_type);
                break;
        };
        free(e);
    }

    //xcb_free_cursor(connection, cursor);
    //xcb_close_font(connection, font);
    xcb_disconnect(connection);
}
