extern void *eglGetProcAddress(const char *name);

void *glXGetProcAddress(const char *name)
{
  return eglGetProcAddress(name);
}

void *glXGetProcAddressARB(const char *name)
{
  return eglGetProcAddress(name);
}

const char *glXGetClientString(void *dpy, int name)
{
  return "";
}

int glXQueryVersion(void *dpy, int *major, int *minor)
{
  return 0;
}
