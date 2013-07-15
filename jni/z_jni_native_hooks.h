int libpd_sync_init(int input_channels, int output_channels, int sample_rate);
int libpd_sync_process_raw(const float *inBuf, float *outBuf);
