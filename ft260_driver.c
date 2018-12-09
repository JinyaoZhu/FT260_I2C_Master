/**
 * @file ft260_driver.c
 * @author J.Zhu
 * @date 2018-12-09
 */

#include <ft260_driver.h>

int g_fd; // file descriptor

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


FT260_STATUS FT260_Open()
{
    int fd;
    int res, desc_size = 0;
    char buf[256];
    struct hidraw_report_descriptor rpt_desc;
    struct hidraw_devinfo info;
    char *device_path;

    device_path = get_hid_path(FT260_VENDOR_ID, FT260_PRODUCT_ID, FT260_INTERFACE_ID);

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

        g_fd = fd;
        printf(PGRN "FT260_Open success!\n" PRST);
        return FT260_OK;
    }
}


FT260_STATUS FT260_I2C_Setup()
{
    uint8_t buf[4];
    int32_t res;
    /* Set Feature */
    buf[0] = 0xA1;
    /* Set I²C Clock Speed */
    buf[1] = 0x22; 
    /* I2C_SPEED 100Kbps */
    buf[2] = 0x64; 
    buf[3] = 0x00;
    
    res = ioctl(g_fd, HIDIOCSFEATURE(4), buf);
    if (res < 0)
    {
        perror("HIDIOCSFEATURE");
        printf(PRED "FT260_I2C_Setup fail!\n" PRST);
        return FT260_FAIL;
    }
    else
    {
        printf("ioctl HIDIOCGFEATURE returned: %d\n", res);
        printf(PGRN "FT260_I2C_Setup success!\n" PRST);
        return FT260_OK;
    }
}


FT260_STATUS FT260_I2C_Write(uint8_t addr,
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

    res = write(g_fd, write_buf, write_buf_len);

    if (res == write_buf_len)
    {
        // printf("write() wrote %d bytes\n", res);
        *written_length = res-4;
        return FT260_OK;
    }
    else
    {
        // printf("Error: %d\n", errno);
        perror("write");
        *written_length = -1;
        return FT260_FAIL;
    }
}