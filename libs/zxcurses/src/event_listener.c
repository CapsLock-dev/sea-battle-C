#include "zxcurses/event_listener.h"
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

static int signal_fd = -1;
static int epoll_fd = -1;
static int timer_fd = -1;

static int epoll_add(int fd, unsigned int events) {
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

bool init_event_listener(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGWINCH);
    sigaddset(&mask, SIGINT);
    if(sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        return false;
    }
    signal_fd = signalfd(-1, &mask, SFD_NONBLOCK);
    if (signal_fd == -1) {
        return false;
    }
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        return false;
    }
    if (epoll_add(signal_fd, EPOLLIN) < 0) {
        return false;
    }
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) < 0) {
        return false;
    }
    if (epoll_add(STDIN_FILENO, EPOLLIN) < 0) {
        return false;
    }
    timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0) {
        return false;
    }
    struct itimerspec timer_spec = {
        .it_value.tv_nsec = 0, .it_value.tv_sec = 1, 
        .it_interval.tv_nsec = 0, .it_interval.tv_sec = 1
    };
    if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) < 0) {
        return false;
    }
    if (epoll_add(timer_fd, EPOLLIN) < 0) {
        return false;
    }
    return true;
}

bool (*g_on_signal)(int fd, void* context) = NULL;
bool (*g_on_stdin)(int fd, void* context) = NULL;
bool (*g_on_timer)(int fd, void* context) = NULL;
bool (*g_on_tick)(void* context) = NULL;

void set_on_signal(bool(*handler)(int fd, void* context)) {
    g_on_signal = handler;
}

void set_on_stdin(bool(*handler)(int fd, void* context)) {
    g_on_stdin = handler;
}

void set_on_timer(bool(*handler)(int fd, void* context)) {
    g_on_timer = handler;
}

void set_on_tick(bool(*handler)(void* context)) {
    g_on_tick = handler;
}

void main_loop(void* context) {
    bool running = true;
    write(STDOUT_FILENO, "\033[2J\033[H", 7);
    if (g_on_tick != NULL) g_on_tick(context);
    while(running) {
        struct epoll_event events[4];
        int ready_fds = epoll_wait(epoll_fd, events, 4, -1);
        if (ready_fds == -1) running = false;
        for (int i=0; i<ready_fds; ++i) {
            int fd = events[i].data.fd;
            if (fd == STDIN_FILENO && g_on_stdin != NULL) {
                running = g_on_stdin(fd, context);
            } else if (fd == signal_fd && g_on_signal != NULL) {
                running = g_on_signal(fd, context);
            } else if (fd == timer_fd && g_on_timer != NULL) {
                g_on_timer(fd, context);
            }
        }
        if (g_on_tick != NULL) g_on_tick(context);
    }
    close(timer_fd);
    close(signal_fd);
    close(epoll_fd);
}
