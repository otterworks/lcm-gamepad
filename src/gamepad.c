#include "config.h"

#include "gamepad.h"
#include "gamepad_argp.h"
#include "mx_joy6_t.h"
#include "mx_joyButtonNumber_t.h"

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/timerfd.h>

#include <lcm/lcm.h>
#include <libevdev/libevdev.h>

const struct itimerspec oits = { { 0, OUTPUT_PERIOD }, { 0, OUTPUT_PERIOD } };

static inline int64_t
utime(void)
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  return (int64_t)(t.tv_sec * 1000000) + (int64_t)(t.tv_nsec / 1000);
}

static inline double
normalize(const int v, const int min, const int max)
{
  double half_range = 0.5 * (max - min);
  double sv = v - half_range;
  return sv / half_range;
}

int
main(int argc, char** argv)
{
  args.verbosity = 0;
  args.device = DEV_DEFAULT;
  argp_parse(&argp, argc, argv, 0, 0, &args);

  int rc = 0;
  struct epoll_event pev = { 0 };
  struct libevdev* dev = NULL;
  struct input_event iev = { 0 };
  mx_joy6_t joy = { 0 };
  mx_joyButtonNumber_t btn = { 0 };

  int epfd = epoll_create(1);
  if (-1 == epfd) {
    perror("epoll_create");
    fputs("failed to create epoll file descriptor\n", stderr);
    exit(EXIT_FAILURE);
  }

  int otfd = timerfd_create(CLOCK_MONOTONIC, 0);
  if (-1 == otfd) {
    perror("timerfd_create");
    fprintf(stderr, "failed to create timer for output\n");
    exit(EXIT_FAILURE);
  }
  if (-1 == timerfd_settime(otfd, 0, &oits, NULL)) {
    perror("timerfd_settime");
    fprintf(stderr,
            "failed to set timer %d: %ld ns, interval %ld ns\n",
            otfd,
            oits.it_value.tv_nsec,
            oits.it_interval.tv_nsec);
    exit(EXIT_FAILURE);
  }

  int ctfd = open(args.device, O_RDONLY | O_NONBLOCK);
  if (-1 == ctfd) {
    perror("open");
    fprintf(stderr, "tried to open %s\n", args.device);
    exit(EXIT_FAILURE);
  }
  if (0 != libevdev_new_from_fd(ctfd, &dev)) {
    perror("libevdev_new_from_fd");
    fputs("failed to initialize libevdev\n", stderr);
    exit(EXIT_FAILURE);
  }
  if (args.verbosity > -1) {
    printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
  }
  if (args.verbosity > 0) {
    printf("Input device ID: bus %#x vendor %#x product %#x\n",
           libevdev_get_id_bustype(dev),
           libevdev_get_id_vendor(dev),
           libevdev_get_id_product(dev));
  }
  if (!libevdev_has_event_type(dev, EV_ABS)) {
    fputs("Input device cannot describe absolute axis value changes.\n",
          stderr);
    exit(EXIT_FAILURE);
  }

  const int surge_min = libevdev_get_abs_minimum(dev, EV_SURGE);
  const int surge_max = libevdev_get_abs_maximum(dev, EV_SURGE);
  const int sway_min = libevdev_get_abs_minimum(dev, EV_SWAY);
  const int sway_max = libevdev_get_abs_maximum(dev, EV_SWAY);
  const int heave_min = libevdev_get_abs_minimum(dev, EV_HEAVE);
  const int heave_max = libevdev_get_abs_maximum(dev, EV_HEAVE);
  const int yaw_min = libevdev_get_abs_minimum(dev, EV_YAW);
  const int yaw_max = libevdev_get_abs_maximum(dev, EV_YAW);

  pev.events = EPOLLIN;
  pev.data.fd = otfd;
  rc = epoll_ctl(epfd, EPOLL_CTL_ADD, pev.data.fd, &pev);
  if (-1 == rc) {
    perror("epoll_ctl");
    fprintf(stderr,
            "failed to add output timer fd %d to epoll fd %d\n",
            pev.data.fd,
            epfd);
  }
  pev.data.fd = ctfd;
  rc = epoll_ctl(epfd, EPOLL_CTL_ADD, pev.data.fd, &pev);
  if (-1 == rc) {
    perror("epoll_ctl");
    fprintf(
      stderr, "failed to add input fd %d to epoll fd %d\n", pev.data.fd, epfd);
  }

  lcm_t* lcm = lcm_create(NULL);
  if (NULL == lcm) {
    fputs("failed to create LCM instance\n", stderr);
    exit(EXIT_FAILURE);
  }

  memset(&pev, 0, sizeof(pev));
  uint64_t expirations = 0;
  while (1 == epoll_wait(epfd, &pev, 1, -1)) { // single-event, blocking
    assert(pev.events & EPOLLIN);
    if (ctfd == pev.data.fd) {
      rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &iev);
      if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
        if (args.verbosity > 1) {
          printf("Event type: %s, code: %s, value: %d\n",
                 libevdev_event_type_get_name(iev.type),
                 libevdev_event_code_get_name(iev.type, iev.code),
                 iev.value);
        }
        if (iev.type == EV_ABS) {
          joy.utime = utime();
          switch (iev.code) {
            case EV_SURGE:
              joy.joyval[XJ_SURGE] = normalize(iev.value, surge_min, surge_max);
              break;
            case EV_SWAY:
              joy.joyval[XJ_SWAY] = normalize(iev.value, sway_min, sway_max);
              break;
            case EV_HEAVE:
              joy.joyval[XJ_HEAVE] = -normalize(iev.value, heave_min, heave_max);
              break;
            case EV_YAW:
              joy.joyval[XJ_YAW] = normalize(iev.value, yaw_min, yaw_max);
              break;
            default:
              fprintf(stderr,
                      "unhandled code: %s, value: %d\n",
                      libevdev_event_code_get_name(iev.type, iev.code),
                      iev.value);
          }
          // DO NOT publish on LCM at the event rate from the controller,
          // let the timerfd handle that
        } else if (iev.type == EV_KEY) {
          btn.utime = utime();
          switch (iev.code) {
            case EV_AUTO_DEPTH:
              btn.buttonNumber = XJ_AUTO_DEPTH;
              break;
            case EV_AUTO_HEADING:
              btn.buttonNumber = XJ_AUTO_HEADING;
              break;
            case EV_START_TRACKING:
              btn.buttonNumber = XJ_START_TRACKING;
              break;
            default:
              fprintf(stderr,
                      "unhandled code: %s, value: %d\n",
                      libevdev_event_code_get_name(iev.type, iev.code),
                      iev.value);
          }
          mx_joyButtonNumber_t_publish(lcm, BUTTON_OUTPUT_CHANNEL, &btn);
          btn.utime = 0;
          btn.buttonNumber = 0;
        }
      } else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
        fputs("libevdev_next_event returned LIBEVDEV_READ_STATUS_SYNC\n",
              stderr);
      }
    } else if (otfd == pev.data.fd) {
      size_t n = read(pev.data.fd, &expirations, 8);
      if (joy.utime != 0) {
        joy.utime = utime();
        mx_joy6_t_publish(lcm, STICK_OUTPUT_CHANNEL, &joy);
        joy.utime = 0;
      }
    }
  }

  if (0 != errno) {
    perror("epoll_wait");
  } else {
    fputs("single-event blocking epoll_wait apparently timed out"
          " or returned more than one event",
          stderr);
  }

  lcm_destroy(lcm);
  libevdev_free(dev);
  close(ctfd);
  close(otfd);
  close(epfd);
  exit(EXIT_FAILURE); // success runs forever
}
