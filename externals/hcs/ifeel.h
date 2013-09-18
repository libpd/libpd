#define USB_IFEEL_BUZZ_IOCTL _IOW('U', 1, struct ifeel_command)

struct ifeel_command {
    unsigned int strength;
    unsigned int delay;
    unsigned int count;
};
