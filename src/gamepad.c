#include "config.h"

#include "gamepad.h"
#include "gamepad_argp.h"
#include "kinematics_twist_t.h"

#include <fcntl.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>

#include <sys/epoll.h>

#include <lcm/lcm.h>

static inline int64_t
utime(void)
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME, &t);
  return (int64_t)(t.tv_sec * 1000000) + (int64_t)(t.tv_nsec / 1000);
}

int
main(int argc, char** argv)
{
  args.verbosity = 0;
  args.device = DEV_DEFAULT;
  argp_parse(&argp, argc, argv, 0, 0, &args);

  struct epoll_event ev = { 0 };
  kinematics_twist_t twi = { 0 };

  int epfd = epoll_create(1); // arg is supposed to be vestigal
  if (-1 == epfd) {
    perror("epoll_create");
    fputs("failed to create epoll file descriptor\n", stderr);
    exit(EXIT_FAILURE);
  }

  int ctfd = open(args.device, O_RDONLY); // fd for controller
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = ctfd;
  int rv = epoll_ctl(epfd, EPOLL_CTL_ADD, ctfd, &ev);
  if (-1 == rv) {
    perror("epoll_ctl");
    fprintf(stderr, "failed to add input fd %d to epoll fd %d\n", ctfd, epfd);
  }

  lcm_t* lcm = lcm_create(NULL);
  if (NULL == lcm) {
    fputs("failed to create LCM instance\n", stderr);
    exit(EXIT_FAILURE);
  }

  memset(&ev, 0, sizeof(ev));
  uint64_t expirations = 0;
  while (1 == epoll_wait(epfd, &ev, 1, -1)) { // single-event, blocking
    // TODO: assert ( ev.events & EPOLLIN )
    if (ctfd == ev.data.fd) {
      kinematics_twist_t_publish(lcm, TWIST_OUTPUT_CHANNEL, &twi);
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
  close(ctfd);
  close(epfd);
  exit(EXIT_FAILURE); // success runs forever
}
