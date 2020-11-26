#define PROCFILE_NAME "rootkit"

#define ENV_VAR "root"
#define ENV_PASS "yes"

#define MIN(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
