#include <Fission/Core/Window.hh>
#include <Fission/Core/Engine.hh>
#include "../Common.h"
#include <thread>
#include <chrono>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_event.h>
#include <xcb/xcb_keysyms.h>

extern fs::Engine engine;
xcb_key_symbols_t *syms;

__FISSION_BEGIN__

#define EM(MASK_NAME) XCB_EVENT_MASK_ ## MASK_NAME

static xcb_intern_atom_reply_t *atom_wm_delete_window = nullptr;

int Window::_linux_handle_event(xcb_generic_event_t* event) {
  switch(XCB_EVENT_RESPONSE_TYPE(event)) {
    default: {
      printf("Unknown event: %d\n", event->response_type);
    }
    break; case XCB_CLIENT_MESSAGE: {
        if ((*(xcb_client_message_event_t *)event).data.data32[0] == (*atom_wm_delete_window).atom) {
            return 1;
        }
    }
    break; case XCB_EXPOSE: {
      xcb_expose_event_t *ev = (xcb_expose_event_t *)event;
      printf ("Window %ld exposed. Region to be redrawn at location (%d,%d), with dimension (%d,%d)\n", ev->window, ev->x, ev->y, ev->width, ev->height);
      width = ev->width; height = ev->height;
    }
    break; case XCB_BUTTON_PRESS: {
      printf("\n");
    }
    break; case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *ev = (xcb_button_release_event_t *)event;
      printf ("Button %d released in window %ld, at coordinates (%d,%d)\n", ev->detail, ev->event, ev->event_x, ev->event_y);
    }
    break; case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)event;
      printf ("Mouse moved in window %ld, at coordinates (%d,%d)\n", ev->event, ev->event_x, ev->event_y);
    }
    break; case XCB_ENTER_NOTIFY: {
      xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t *)event;
      printf ("Mouse entered window %ld, at coordinates (%d,%d)\n", ev->event, ev->event_x, ev->event_y);
    }
    break; case XCB_LEAVE_NOTIFY: {
      xcb_leave_notify_event_t *ev = (xcb_leave_notify_event_t *)event;
      printf ("Mouse left window %ld, at coordinates (%d,%d)\n", ev->event, ev->event_x, ev->event_y);
    }
    break; case XCB_KEY_PRESS: {
      xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
      xcb_keysym_t keysym = xcb_key_press_lookup_keysym(syms, ev, 0);
      printf ("Key pressed in window keycode=%d, keysym=%d\n", ev->detail, keysym);

      Event event{
          .timestamp = 0u, // TODO: fix me
          .type = Event_Key_Down,
          .key_up = {
              .key_id = (u32)keysym,
          }
      };
      event_queue.append(event);
    }
    break; case XCB_KEY_RELEASE: {
      xcb_key_release_event_t *ev = (xcb_key_release_event_t *)event;
      xcb_keysym_t keysym = xcb_key_press_lookup_keysym(syms, ev, 0);
      printf ("Key released in window %ld\n", ev->event);

      Event event{
          .timestamp = 0u, // TODO: fix me
          .type = Event_Key_Up,
          .key_up = {
              .key_id = (u32)keysym,
          }
      };
      event_queue.append(event);
    }
    }
    return 0;
  }

void Window::_linux_thread_main(struct Window* window) {
  printf("Enter window thread\n");
  xcb_generic_event_t *e;
  
  while ((e = xcb_wait_for_event(window->_connection))) {
    if (window->_linux_handle_event(e))
      break;
    free(e);
  }
  free(e);
  engine.flags &=~ engine.fRunning;
  printf("Exit window thread\n");
}

void Window::create(struct Window_Create_Info* info) {
    const xcb_setup_t *setup;
    xcb_screen_iterator_t iter;
    int scr;

    const char *display_envar = getenv("DISPLAY");
    if (display_envar == nullptr || display_envar[0] == '\0') {
        printf("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    _connection = xcb_connect(nullptr, &scr);
    if (xcb_connection_has_error(_connection) > 0) {
        printf("Cannot connect to XCB.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    syms = xcb_key_symbols_alloc(_connection);

    setup = xcb_get_setup(_connection);
    iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) xcb_screen_next(&iter);

    _screen = iter.data;

    {
    uint32_t value_mask, value_list[32];

    _id = xcb_generate_id(_connection);

    value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    value_list[0] = _screen->black_pixel;
    value_list[1] = EM(STRUCTURE_NOTIFY)
      | EM(KEY_RELEASE)
      | EM(KEY_PRESS)
      | EM(EXPOSURE);

    width = 100; height = 100;
    xcb_create_window(_connection, XCB_COPY_FROM_PARENT, _id, _screen->root, 0, 0, width, height, 0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, _screen->root_visual, value_mask, value_list);

    xcb_map_window(_connection, _id);

    /* Magic code that will send notification when window is destroyed */
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(_connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(_connection, 0, 16, "WM_DELETE_WINDOW");
    atom_wm_delete_window = xcb_intern_atom_reply(_connection, cookie2, 0);

    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _id, (*reply).atom, 4, 32, 1, &(*atom_wm_delete_window).atom);

    free(reply);
    }

  //  xcb_atom_t m_utf8_string_atom;
  //  xcb_atom_t m_net_wm_name_atom;
//
  //  xcb_intern_atom_cookie_t  utf8_string_cookie = xcb_intern_atom(connection, 0, 11, "UTF8_STRING");
  //  xcb_intern_atom_reply_t*  utf8_string_reply  = xcb_intern_atom_reply(connection, utf8_string_cookie, 0);
  //  m_utf8_string_atom = utf8_string_reply->atom;
  //  free(utf8_string_reply);
//
  //  xcb_intern_atom_cookie_t  net_wm_name_cookie = xcb_intern_atom(connection, 0, 12, "_NET_WM_NAME");
  //  xcb_intern_atom_reply_t*  net_wm_name_reply  = xcb_intern_atom_reply(connection, net_wm_name_cookie, 0);
  //  m_net_wm_name_atom = net_wm_name_reply->atom;
  //  free(net_wm_name_reply);
//
  //  std::u8string t = u8"Sandbox";
  //  // Set window name.
  //  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, id, XCB_ATOM_WM_NAME, m_utf8_string_atom, 8, t.size(), t.data());
  //  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, id, m_net_wm_name_atom, m_utf8_string_atom, 8, t.size(), t.data());


    // Force the x/y coordinates to 100,100 results are identical in
    // consecutive
    // runs
    std::array<uint32_t, 2> const coords = {100, 100};
    xcb_configure_window(_connection, _id, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords.data());

    xcb_flush(_connection);

    _thread = std::thread(_linux_thread_main, this);
}
void Window::set_title(string const& title) {}
void Window::close() {  }
bool Window::is_minimized() { return false; }
bool Window::exists() const { return true; }
void Window::set_mode(Window_Mode mode) {}
void Window::sleep_until_not_minimized() {}
void Window::set_using_mouse_deltas(bool use) {}

Window::~Window() {
    printf("called window destructor\n");
    _thread.join();
    xcb_destroy_window(_connection, _id);
    xcb_disconnect(_connection);
}

__FISSION_END__
