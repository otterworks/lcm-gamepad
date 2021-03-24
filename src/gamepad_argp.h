#include "config.h"
#include "gamepad.h"

#include <argp.h>
#include <inttypes.h>

extern char** environ;

const char* argp_program_version = PACKAGE_STRING;

const char* argp_program_bug_address = PACKAGE_BUGREPORT;

static char doc[] = "lcm-gamepad";

static char args_doc[] = "";

static struct argp_option options[] = {
  { "verbose", 'v', 0, 0, "say more" },
  { "quiet", 'q', 0, 0, "say less" },
  { "config", 'c', "FILE", 0, "read config from FILE" },
  { "device", 'd', "FILE", 0, "use gamepad device at FILE" },
  { 0 }
};

struct arguments
{
  int8_t verbosity;
  char* config_file;
  char* device;
};

static error_t
parse_opt(int key, char* arg, struct argp_state* state)
{
  struct arguments* args = state->input;
  switch (key) {
    case 'v':
      args->verbosity += 1;
      break;
    case 'q':
      args->verbosity -= 1;
      break;
    case 'c':
      args->config_file = arg;
      break;
    case 'd':
      args->device = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        argp_usage(state);
      } else {
        args->device = arg;
      }
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 0)
        argp_usage(state);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

struct arguments args; // just leave it as a global for now
