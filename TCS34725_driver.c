#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "tcs34725_driver"
#define CLASS_NAME  "tcs34725"
#define DEVICE_NAME "tcs34725"

#define TCS34725_ADDRESS     0x29
#define COMMAND_BIT          0x80
#define Enable_REG           0x00
#define RGBC_TIMING_REG      0x01
#define Wait_TIME_REG        0x03
#define RGBC_INT_THRESHOLD   0x04
#define Persistence_Register 0x0C
#define CONFIG               0x0D
#define CONTROL              0x0F
#define ID_SENSOR            0x12
#define STATUS               0x13
#define CDATA                0x14
#define RDATA                0x16
#define GDATA                0x18
#define BDATA                0x1A

// IOCTL commands
#define TCS34725_IOCTL_MAGIC 't'
#define TCS34725_IOCTL_READ_C _IOR(TCS34725_IOCTL_MAGIC, 1, int)
#define TCS34725_IOCTL_READ_R _IOR(TCS34725_IOCTL_MAGIC, 2, int)
#define TCS34725_IOCTL_READ_G _IOR(TCS34725_IOCTL_MAGIC, 3, int)
#define TCS34725_IOCTL_READ_B _IOR(TCS34725_IOCTL_MAGIC, 4, int)
#define TCS34725_IOCTL_INIT _IOW(TCS34725_IOCTL_MAGIC, 5, struct tcs34725_config)
#define TCS34725_IOCTL_READ_ChuanHoa _IOR(TCS34725_IOCTL_MAGIC, 6, int)

static struct i2c_client *tcs34725_client;
static struct class* tcs34725_class = NULL;
static struct device* tcs34725_device = NULL;
static int major_number;

u16 cdata, rdata, gdata, bdata;
u8  cdata_1, rdata_1, gdata_1, bdata_1;

struct tcs34725_config {
    uint8_t enable_reg;
    uint8_t rgbc_timing;
    uint8_t wait_time;
    uint8_t persistence;
    uint8_t config;
    uint8_t control;
};

struct rgb_values {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static uint16_t ReadSensor(struct i2c_client *client, uint8_t reg)
{
    int low, high;
    low = i2c_smbus_read_byte_data(client, COMMAND_BIT | reg);
    if (low < 0){
        printk(KERN_ERR "Failed to read low byte from reg 0x%02X\n", reg);
        return -EIO; 
    }
    high = i2c_smbus_read_byte_data(client, COMMAND_BIT | (reg + 1));
    if (high < 0){
        printk(KERN_ERR "Failed to read high byte from reg 0x%02X\n", reg + 1);
        return -EIO; 
    }
    return (high << 8) | low;
}

static long tcs34725_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct tcs34725_config config;
    
    switch (cmd) {
        case TCS34725_IOCTL_INIT:
            if (copy_from_user(&config, (struct tcs34725_config __user *)arg, sizeof(config)))
                return -EFAULT;

            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | Enable_REG, config.enable_reg) < 0) {
                printk(KERN_ERR "Failed to write Enable_REG\n");
                return -EIO;
            }
            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | RGBC_TIMING_REG, config.rgbc_timing) < 0) {
                printk(KERN_ERR "Failed to write RGBC_TIMING_REG\n");
                return -EIO;
            }
            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | Wait_TIME_REG, config.wait_time) < 0) {
                printk(KERN_ERR "Failed to write WAIT_TIME_REG\n");
                return -EIO;
            }
            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | Persistence_Register, config.persistence) < 0) {
                printk(KERN_ERR "Failed to write PERSISTENCE_REG\n");
                return -EIO;
            }
            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | CONFIG, config.config) < 0) {
                printk(KERN_ERR "Failed to write CONFIG_REG\n");
                return -EIO;
            }
            if (i2c_smbus_write_byte_data(tcs34725_client, COMMAND_BIT | CONTROL, config.control) < 0) {
                printk(KERN_ERR "Failed to write CONTROL_REG\n");
                return -EIO;
            }
            
            printk("TCS34725 configured successfully\n");
            break;
        
        case TCS34725_IOCTL_READ_C:
            cdata = ReadSensor(tcs34725_client, CDATA);
            if (copy_to_user((u16 __user *)arg, &cdata, sizeof(cdata))) {
                return -EFAULT;
            }
            break;

        case TCS34725_IOCTL_READ_R:
            rdata = ReadSensor(tcs34725_client, RDATA);
            if (copy_to_user((u16 __user *)arg, &rdata, sizeof(rdata))) {
                return -EFAULT;
            }
            break;  

        case TCS34725_IOCTL_READ_G:
            gdata = ReadSensor(tcs34725_client, GDATA);
            if (copy_to_user((u16 __user *)arg, &gdata, sizeof(gdata))) {
                return -EFAULT;
            }
            break; 

        case TCS34725_IOCTL_READ_B:
            bdata = ReadSensor(tcs34725_client, BDATA);
            if (copy_to_user((u16 __user *)arg, &bdata, sizeof(bdata))) {
                return -EFAULT;
            }
            break; 

        case TCS34725_IOCTL_READ_ChuanHoa:
            struct rgb_values rgb;
            cdata = ReadSensor(tcs34725_client, CDATA);
            rdata = ReadSensor(tcs34725_client, RDATA);
            gdata = ReadSensor(tcs34725_client, GDATA);
            bdata = ReadSensor(tcs34725_client, BDATA);
            if (cdata == 0) cdata = 1; // Tranh chia cho 0
            rgb.r = (rdata * 255) / cdata;
            rgb.g = (gdata * 255) / cdata;
            rgb.b = (bdata * 255) / cdata;
            if (copy_to_user((struct rgb_values __user *)arg, &rgb, sizeof(rgb))) {
                return -EFAULT;
            }
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static int tcs34725_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "TCS34725 device opened\n");
    return 0;
}

static int tcs34725_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "tcs34725 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = tcs34725_open,
    .unlocked_ioctl = tcs34725_ioctl,
    .release = tcs34725_release,
};

static int tcs34725_probe(struct i2c_client *client)
{
    printk("ACCESSED");
    tcs34725_client = client;

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    tcs34725_class = class_create(CLASS_NAME);
    if (IS_ERR(tcs34725_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(tcs34725_class);
    }

    tcs34725_device = device_create(tcs34725_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(tcs34725_device)) {
        class_destroy(tcs34725_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(tcs34725_device);
    }

    printk(KERN_INFO "TCS43725 driver installed\n");
    return 0;
}

static void tcs34725_remove(struct i2c_client *client)
{
    device_destroy(tcs34725_class, MKDEV(major_number, 0));
    class_unregister(tcs34725_class);
    class_destroy(tcs34725_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "TCS34725_driver removed\n");
}

static const struct of_device_id tcs34725_of_match[] = {
    { .compatible = "amsco,tcs34725", },
    { },
};

MODULE_DEVICE_TABLE(of, tcs34725_of_match);

static struct i2c_driver tcs34725_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = tcs34725_of_match,
    },
    .probe      = tcs34725_probe,
    .remove     = tcs34725_remove,
};

static int __init tcs34725_init(void)
{
    printk(KERN_INFO "Initializing TCS34725 driver\n");
    return i2c_add_driver(&tcs34725_driver);
}

static void __exit tcs34725_exit(void)
{
    printk(KERN_INFO "Exiting TCS34725 driver\n");
    i2c_del_driver(&tcs34725_driver);
}

module_init(tcs34725_init);
module_exit(tcs34725_exit);

MODULE_AUTHOR("TEAM");
MODULE_DESCRIPTION("TCS34725 I2C Client Driver with IOCTL Interface");
MODULE_LICENSE("GPL");

