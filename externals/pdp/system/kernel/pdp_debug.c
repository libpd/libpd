#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "pdp_post.h"

int pdp_debug_sigtrap_on_assert;


void pdp_assert_hook (char *condition, char *file, int line)
{
    pdp_post("PDP_ASSERT (%s) failed in file %s, line %u. ", condition, file, line);
    pdp_post("%s.\n", pdp_debug_sigtrap_on_assert ? "sending SIGTRAP" : "continuing");

    if (pdp_debug_sigtrap_on_assert) kill(getpid(), SIGTRAP);
}


void pdp_debug_setup(void)
{
    pdp_debug_sigtrap_on_assert = 1;
}
