using System;

namespace LibPDBinding.Managed
{
    public sealed class Binding
    {
        private readonly IntPtr _handle;

        internal Binding(IntPtr ptr)
        {
            _handle = ptr;
        }
        ~Binding()
        {
            Dispose(false);
        }
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                PInvoke.unbind(_handle);
            }
        }
    }
}
