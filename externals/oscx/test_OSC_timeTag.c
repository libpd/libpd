/*
 * Copyright (c) 1997 Regents of the University of California.
 * All rights reserved.
 *
 * The name of the University may not be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE
 * IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 */

/*
 test_OSC_timeTag.c
 Matt Wright, 5/30/97
*/

#include <stdio.h>
#include "OSC_timeTag.h"

main() {
    OSCTimeTag now, later;

    now = OSCTT_CurrentTime();
    printf("Now it's %llu (0x%llx)\n", now, now);

    printf("Immediately would be %llu (0x%llx)\n", OSCTT_Immediately(),
	   OSCTT_Immediately());

    later = OSCTT_PlusSeconds(now, 1.0f);
    printf("One second from now would be %llu (0x%llx)\n", later, later);

    now = OSCTT_CurrentTime();
    printf("And *now* it's %llu (0x%llx)\n", now, now);
}

