#ifndef CL_ZXCURSES_EVENT_LISTENER_H
#define CL_ZXCURSES_EVENT_LISTENER_H

bool init_event_listener(void);
void main_loop(void* context);

void set_on_signal(bool(*handler)(int fd, void* context));
void set_on_stdin(bool(*handler)(int fd, void* context));
void set_on_timer(bool(*handler)(int fd, void* context));
void set_on_tick(bool(*handler)(void* context));

#endif
