#include <gfsmDebug.h>
#include <gfsmMem.h>
#include <gfsmConfig.h>

#ifdef GFSM_DEBUG_ENABLED
# define GFSM_MEM_DEBUG
# define GFSM_ALLOC_DEBUG
#endif /* GFSM_DEBUG_ENABLED */

void gfsm_debug_init(void) {
#if defined(GFSM_MEM_DEBUG)
  g_mem_set_vtable(glib_mem_profiler_table);
# if defined(GFSM_ALLOC_DEBUG)
  gfsm_allocators_enable();
# endif /* GFSM_ALLOC_DEBUG */
#endif /* GFSM_MEM_DEBUG */
  return;
}

void gfsm_debug_finish(void) {
#if defined(GFSM_MEM_DEBUG)
  g_blow_chunks();
# if defined(GFSM_ALLOC_DEBUG)
  gfsm_allocators_free();
# endif /* GFSM_ALLOC_DEBUG */
#endif /* GFSM_MEM_DEBUG */
  return;
}

void gfsm_debug_print(void) {
#if defined(GFSM_MEM_DEBUG)
  g_mem_profile();
#endif /* GFSM_MEM_DEBUG */
  return;
}
