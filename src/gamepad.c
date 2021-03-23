#include "config.h"

#include "gamepad.h"
#include "gamepad_argp.h"
#include "kinematics_twist_t.h"

#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>

#include <lcm/lcm.h>
#include <libevdev/libevdev.h>

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
  kinematics_twist_t twi = { 0 };

  int epfd = epoll_create(1);
  if (-1 == epfd) {
    perror("epoll_create");
    fputs("failed to create epoll file descriptor\n", stderr);
    exit(EXIT_FAILURE);
  }

  int ctfd = open(args.device, O_RDONLY | O_NONBLOCK);
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

  const int abs_x_min = libevdev_get_abs_minimum(dev, ABS_X);
  const int abs_x_max = libevdev_get_abs_maximum(dev, ABS_X);
  const int abs_y_min = libevdev_get_abs_minimum(dev, ABS_Y);
  const int abs_y_max = libevdev_get_abs_maximum(dev, ABS_Y);
  const int abs_rx_min = libevdev_get_abs_minimum(dev, ABS_RX);
  const int abs_rx_max = libevdev_get_abs_maximum(dev, ABS_RX);
  const int abs_ry_min = libevdev_get_abs_minimum(dev, ABS_RY);
  const int abs_ry_max = libevdev_get_abs_maximum(dev, ABS_RY);

  memset(&pev, 0, sizeof(pev));
  pev.events = EPOLLIN;
  pev.data.fd = ctfd;
  rc = epoll_ctl(epfd, EPOLL_CTL_ADD, ctfd, &pev);
  if (-1 == rc) {
    perror("epoll_ctl");
    fprintf(stderr, "failed to add input fd %d to epoll fd %d\n", ctfd, epfd);
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
          switch (iev.code) {
            case ABS_X:
              twi.angular[2] = normalize(iev.value, abs_x_min, abs_x_max);
              break;
            case ABS_Y:
              twi.linear[2] = -normalize(iev.value, abs_y_min, abs_y_max);
              break;
            case ABS_RX:
              twi.linear[1] = normalize(iev.value, abs_rx_min, abs_rx_max);
              break;
            case ABS_RY:
              twi.linear[0] = normalize(iev.value, abs_ry_min, abs_ry_max);
              break;
            default:
              fprintf(stderr,
                      "unhandled code: %s, value: %d\n",
                      libevdev_event_code_get_name(iev.type, iev.code),
                      iev.value);
          }
          twi.utime = utime();
          kinematics_twist_t_publish(lcm, TWIST_OUTPUT_CHANNEL, &twi);
        }
      } else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
        fputs("libevdev_next_event returned LIBEVDEV_READ_STATUS_SYNC\n",
              stderr);
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
  close(epfd);
  exit(EXIT_FAILURE); // success runs forever
}
