/**
 * @file ft260_driver.c
 * @author J.Zhu
 * @date 2018-12-09
 */

#include <ft260_driver.h>

/******************PRIVATE FUNCTIONS***************************/
const char *bus_type_str(int bus)
{
    switch (bus)
    {
    case BUS_USB:
        return "USB";
    case BUS_HIL:
        return "HIL";
    case BUS_BLUETOOTH:
        return "Bluetooth";
    case BUS_VIRTUAL:
        return "Virtual";
    default:
        return "Other";
    }
}

/**
 * @name get_hid_path
 * @brief return the device file path according to the given ids
 * @param vendor_id, product_id, interface_id
 * @return path of the file(string)
 */
char *get_hid_path(unsigned short vendor_id, unsigned short product_id, unsigned short interface_id)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *usb_dev;
    struct udev_device *intf_dev;
    char *ret_path = NULL;
    /* Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n");
        return NULL;
    }
    /* Create a list of the devices in the 'hidraw' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* udev_list_entry_foreach is a macro which expands to a loop. */
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;
        const char *dev_path;
        const char *str;
        unsigned short cur_vid;
        unsigned short cur_pid;
        unsigned short cur_interface_id;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        dev_path = udev_device_get_devnode(dev);
        /* Find the next parent device, with matching
 subsystem "usb" and devtype value "usb_device" */
        usb_dev = udev_device_get_parent_with_subsystem_devtype(
            dev, "usb", "usb_device");
        if (!usb_dev)
        {
            printf("Unable to find parent usb device.\n");
            return NULL;
        }
        str = udev_device_get_sysattr_value(usb_dev, "idVendor");
        cur_vid = (str) ? strtol(str, NULL, 16) : -1;
        str = udev_device_get_sysattr_value(usb_dev, "idProduct");
        cur_pid = (str) ? strtol(str, NULL, 16) : -1;
        intf_dev = udev_device_get_parent_with_subsystem_devtype(
            dev, "usb", "usb_interface");
        if (!intf_dev)
        {
            printf("Unable to find parent usb interface.\n");
            return NULL;
        }
        str = udev_device_get_sysattr_value(intf_dev, "bInterfaceNumber");
        cur_interface_id = (str) ? strtol(str, NULL, 16) : -1;
        if(FT260_INFO_VERBOSE)
          printf("vid=%x pid=%x interface=%d\n", cur_vid, cur_pid, cur_interface_id);
        if (cur_vid == vendor_id && cur_pid == product_id && cur_interface_id == interface_id)
        {
            ret_path = strdup(dev_path);
            udev_device_unref(dev);
            break;
        }
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return ret_path;
}

/******************PUBLIC FUNCTIONS***************************/
FT260_DEVICE_T* New_FT260(uint16_t vendor_id, uint16_t product_id, uint16_t interface_id)
{
    FT260_DEVICE_T* new_device_ptr = (FT260_DEVICE_T*)malloc(sizeof(FT260_DEVICE_T));
    new_device_ptr->vendor_id = vendor_id;
    new_device_ptr->product_id = product_id;
    new_device_ptr->interface_id = interface_id;
    new_device_ptr->i2c_clock_speed = FT260_I2C_CLK_SPEED;
    new_device_ptr->fd = -1;
    new_device_ptr->Open = Open;
    new_device_ptr->I2C_Setup = I2C_Setup;
    new_device_ptr->I2C_Write = I2C_Write;
    return new_device_ptr;
}

FT260_STATUS Open(FT260_DEVICE_T *self)
{
    int fd;
    int res, desc_size = 0;
    char buf[256];
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;
    char *device_path;

    device_path = get_hid_path(self->vendor_id, self->product_id, self->interface_id);

    fd = open(device_path, O_RDWR | O_NONBLOCK);

    if (fd < 0)
    {
        perror("Unable to open device");
        printf(PRED "FT260_Open fail!\n" PRST);
        close(fd);
        return FT260_FAIL;
    }
    else
    {
        if (FT260_INFO_VERBOSE)
        {
            memset(&rpt_desc, 0x0, sizeof(rpt_desc));
            memset(&info, 0x0, sizeof(info));
            memset(buf, 0x0, sizeof(buf));
            /* Get Report Descriptor Size */
            res = ioctl(fd, HIDIOCGRDESCSIZE, &desc_size);
            if (res < 0)
            {
                perror("HIDIOCGRDESCSIZE");
            }
            else
            {
                printf("Report Descriptor Size: %d\n", desc_size);
            }
            /* Get Raw Name */
            res = ioctl(fd, HIDIOCGRAWNAME(256), buf);
            if (res < 0)
            {
                perror("HIDIOCGRAWNAME");
            }
            else
            {
                printf("Raw Name: %s\n", buf);
            }
            /* Get Raw Info */
            res = ioctl(fd, HIDIOCGRAWINFO, &info);
            if (res < 0)
            {
                perror("HIDIOCGRAWINFO");
            }
            else
            {
                printf("Raw Info:\n");
                printf("\tbustype: %d (%s)\n",
                       info.bustype, bus_type_str(info.bustype));
                printf("\tvendor: 0x%04hx\n", info.vendor);
                printf("\tproduct: 0x%04hx\n", info.product);
            }
        }
        self->fd = fd;
        printf(PGRN "FT260_Open success!\n" PRST);
        return FT260_OK;
    }
}

FT260_STATUS I2C_Setup(FT260_DEVICE_T *self)
{
    uint8_t buf[4];
    int32_t res;
    /* Set Feature */
    buf[0] = 0xA1;
    /* Set I²C Clock Speed */
    buf[1] = 0x22;
    /* I2C clock speed */
    buf[2] = self->i2c_clock_speed & 0x00ff;
    buf[3] = self->i2c_clock_speed >> 8;

    res = ioctl(self->fd, HIDIOCSFEATURE(4), buf);
    if (res < 0)
    {
        perror("HIDIOCSFEATURE");
        printf(PRED "FT260_I2C_Setup fail!\n" PRST);
        return FT260_FAIL;
    }
    else
    {
        if(FT260_INFO_VERBOSE)
            printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
        printf(PGRN "FT260_I2C_Setup success!\n" PRST);
        return FT260_OK;
    }
}

FT260_STATUS I2C_Write(FT260_DEVICE_T *self,
                       uint8_t addr,
                       FT260_I2C_FLAG i2c_flag,
                       uint8_t *data_buf_ptr,
                       uint16_t data_buf_len,
                       int32_t *written_length)
{
    uint8_t write_buf[32];
    int16_t write_buf_len = 0;
    int32_t res;
    /* I2C write */
    write_buf[0] = 0xD0;
    /* Slave 7 bit address */
    write_buf[1] = addr;
    /* I2C Flag */
    write_buf[2] = i2c_flag;
    /* data len */
    write_buf[3] = data_buf_len;
    write_buf_len += 4;

    /* copy write data */
    memcpy(&write_buf[4], data_buf_ptr, data_buf_len);
    write_buf_len += data_buf_len;

    res = write(self->fd, write_buf, write_buf_len);

    if (res == write_buf_len)
    {
        // printf("write() wrote %d bytes\n", res);
        *written_length = res - 4;
        return FT260_OK;
    }
    else
    {
        perror("write");
        *written_length = -1;
        return FT260_FAIL;
    }
}