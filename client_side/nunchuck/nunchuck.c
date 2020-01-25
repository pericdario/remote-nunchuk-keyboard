#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

MODULE_LICENSE("Dual BSD/GPL");
//MODULE_LICENSE("GPL");

/* if defined, timer callback will implement LED0 flashing and
   SW0 reading after each interval */
#define TEST


#define GPIO_BASE_ADDR (0x3F200000)

/* GPIO registers base address. */
#define BCM2708_PERI_BASE   (0x3F000000)
#define GPIO_BASE           (BCM2708_PERI_BASE + 0x200000)
#define GPIO_ADDR_SPACE_LEN (0xB4)
//--

//Handle GPIO: 0-9
/* GPIO Function Select 0. */
#define GPFSEL0_OFFSET (0x00000000)

//Handle GPIO: 10-19
/* GPIO Function Select 1. */
#define GPFSEL1_OFFSET (0x00000004)

//Handle GPIO: 20-29
/* GPIO Function Select 2. */
#define GPFSEL2_OFFSET (0x00000008)

//Handle GPIO: 30-39
/* GPIO Function Select 3. */
#define GPFSEL3_OFFSET (0x0000000C)

//Handle GPIO: 40-49
/* GPIO Function Select 4. */
#define GPFSEL4_OFFSET (0x00000010)

//Handle GPIO: 50-53
/* GPIO Function Select 5. */
#define GPFSEL5_OFFSET (0x00000014)
//--

//GPIO: 0-31
/* GPIO Pin Output Set 0. */
#define GPSET0_OFFSET (0x0000001C)

//GPIO: 32-53
/* GPIO Pin Output Set 1. */
#define GPSET1_OFFSET (0x00000020)
//--

//GPIO: 0-31
/* GPIO Pin Output Clear 0. */
#define GPCLR0_OFFSET (0x00000028)

//GPIO: 32-53
/* GPIO Pin Output Clear 1. */
#define GPCLR1_OFFSET (0x0000002C)
//--

//GPIO: 0-31
/* GPIO Pin Level 0. */
#define GPLEV0_OFFSET (0x00000034)

//GPIO: 32-53
/* GPIO Pin Level 1. */
#define GPLEV1_OFFSET (0x00000038)
//--

//GPIO: 0-53
/* GPIO Pin Pull-up/down Enable. */
#define GPPUD_OFFSET (0x00000094)

//GPIO: 0-31
/* GPIO Pull-up/down Clock Register 0. */
#define GPPUDCLK0_OFFSET (0x00000098)

//GPIO: 32-53
/* GPIO Pull-up/down Clock Register 1. */
#define GPPUDCLK1_OFFSET (0x0000009C)
//--

#define BSC1_BASE_ADDR (0x3F804000)
 
/* BSC1 registers i hit */
#define BSC1_REG_C (BSC1_BASE_ADDR + 0x00000000)
#define BSC1_REG_S (BSC1_BASE_ADDR + 0x00000004)
#define BSC1_REG_DLEN (BSC1_BASE_ADDR + 0x00000008)
#define BSC1_REG_SLAVE_ADDR (BSC1_BASE_ADDR + 0x0000000C)
#define BSC1_REG_FIFO (BSC1_BASE_ADDR + 0x00000010)
#define BSC1_REG_DIV (BSC1_BASE_ADDR + 0x00000014)
#define BSC1_REG_DEL (BSC1_BASE_ADDR + 0x00000018)
#define BSC1_REG_CLKT (BSC1_BASE_ADDR + 0x0000001C)

#define START_TRANSFER_SEND (0x00008080)
#define START_TRANSFER_RECIVE (0x00008081)
#define CLEAR_STATUS (0x00000302)
#define SETUP_CTRL_SEND (0x00008110)
#define SETUP_CTRL_RECIVE (0x00008031)

#define GPFSEL0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000000)
#define GPFSEL1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000004)
#define GPFSEL2_BASE_ADDR (GPIO_BASE_ADDR + 0x00000008)
#define GPFSEL3_BASE_ADDR (GPIO_BASE_ADDR + 0x0000000C)
#define GPFSEL4_BASE_ADDR (GPIO_BASE_ADDR + 0x00000010)
#define GPFSEL5_BASE_ADDR (GPIO_BASE_ADDR + 0x00000014)
#define GPSET0_BASE_ADDR (GPIO_BASE_ADDR + 0x0000001C)
#define GPSET1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000020)
#define GPCLR0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000028)
#define GPCLR1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000002C)
#define GPLEV0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000034)
#define GPLEV1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000038)
#define GPPUD_BASE_ADDR (GPIO_BASE_ADDR + 0x00000094)
#define GPPUDCLK0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000098)
#define GPPUDCLK1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000009C)


/* Using gpio as alternate */
#define GPIO_DIRECTION_ALT0 (4)

#define I2C_ADDRESS 0x52      /* I2C Address for comunication */
#define I2C_BUS     1 

volatile void *reg_c = NULL;
volatile void *reg_dlen = NULL;
volatile void *reg_slave_addr = NULL;
volatile void *reg_fifo = NULL;
volatile void *reg_s = NULL;
volatile void *reg_div = NULL;


/* PUD - GPIO Pin Pull-up/down */
typedef enum {PULL_NONE = 0, PULL_DOWN = 1, PULL_UP = 2} PUD;
//--

//000 = GPIO Pin 'x' is an input
//001 = GPIO Pin 'x' is an output
// By default GPIO pin is being used as an input
typedef enum {GPIO_DIRECTION_IN = 0, GPIO_DIRECTION_OUT = 1} DIRECTION;
//--

/* GPIO pins available on connector p1. */
#define GPIO_02 (2)
#define GPIO_03 (3)
#define GPIO_04 (4)
#define GPIO_05 (5)
#define GPIO_06 (6)
#define GPIO_07 (7)
#define GPIO_08 (8)
#define GPIO_09 (9)
#define GPIO_10 (10)
#define GPIO_11 (11)
#define GPIO_12 (12)
#define GPIO_13 (13)
#define GPIO_14 (14)
#define GPIO_15 (15)
#define GPIO_16 (16)
#define GPIO_17 (17)
#define GPIO_18 (18)
#define GPIO_19 (19)
#define GPIO_20 (20)
#define GPIO_21 (21)
#define GPIO_22 (22)
#define GPIO_23 (23)
#define GPIO_24 (24)
#define GPIO_25 (25)
#define GPIO_26 (26)
#define GPIO_27 (27)

int buff_len = 7;
char *buffer;

/* Declaration of gpio_driver.c functions */
int gpio_driver_init(void);
void gpio_driver_exit(void);
static int gpio_driver_open(struct inode *, struct file *);
static int gpio_driver_release(struct inode *, struct file *);
static ssize_t gpio_driver_read(struct file *, char *buf, size_t , loff_t *);
static ssize_t gpio_driver_write(struct file *, const char *buf, size_t , loff_t *);

/* Structure that declares the usual file access functions. */
struct file_operations gpio_driver_fops =
{
    open    :   gpio_driver_open,
    release :   gpio_driver_release,
    read    :   gpio_driver_read,
    write   :   gpio_driver_write
};

/* Declaration of the init and exit functions. */
module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

/* Global variables of the driver */

/* Major number. */
int major;

/* Buffer to store data. */
#define BUF_LEN 80
char* gpio_driver_buffer;

/* Blink timer vars. */

/*
 * GetGPFSELReg function
 *  Parameters:
 *   pin    - number of GPIO pin;
 *
 *   return - GPFSELn offset from GPIO base address, for containing desired pin control
 *  Operation:
 *   Based on the passed GPIO pin number, finds the corresponding GPFSELn reg and
 *   returns its offset from GPIO base address.
 */
unsigned int GetGPFSELReg(char pin){
        unsigned int addr;
     if(pin >= 0 && pin <10)
        addr = GPFSEL0_BASE_ADDR;
    else if(pin >= 10 && pin <20)
        addr = GPFSEL1_BASE_ADDR;
    else if(pin >= 20 && pin <30)
        addr = GPFSEL2_BASE_ADDR;
    else if(pin >= 30 && pin <40)
        addr = GPFSEL3_BASE_ADDR;
    else if(pin >= 40 && pin <50)
        addr = GPFSEL4_BASE_ADDR;
    else /*if(pin >= 50 && pin <53) */
        addr = GPFSEL5_BASE_ADDR;
  
  return addr;
}

char GetGPIOPinOffset(char pin){
    
    if(pin >= 0 && pin <10)
        pin = pin;
    else if(pin >= 10 && pin <20)
        pin -= 10;
    else if(pin >= 20 && pin <30)
        pin -= 20;
    else if(pin >= 30 && pin <40)
        pin -= 30;
    else if(pin >= 40 && pin <50)
        pin -= 40;
    else /*if(pin >= 50 && pin <53) */
        pin -= 50;
    return pin;
}

void SetInternalPullUpDown(char pin, char value){
    
    unsigned int base_addr_gppud; 
    unsigned int base_addr_gppudclk; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of GPIO Pull-up/down Register (GPPUD). */
    base_addr_gppud = GPPUD_BASE_ADDR;
    
    /* Get base address of GPIO Pull-up/down Clock Register (GPPUDCLK). */
    base_addr_gppudclk = (pin < 32) ? GPPUDCLK0_BASE_ADDR : GPPUDCLK1_BASE_ADDR;

    /* Get pin offset in register . */
    pin = (pin < 32) ? pin : pin - 32;
    
    /* Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
       to remove the current Pull-up/down). */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(value, addr);

    /* Wait 150 cycles ^  this provides the required set-up time for the control signal */
    
    /* Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
       modify ^  NOTE only the pads which receive a clock will be modified, all others will
       retain their previous state. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp |= mask;        
    iowrite32(tmp, addr);

    /* Wait 150 cycles ^  this provides the required hold time for the control signal */
    
    /* Write to GPPUD to remove the control signal. */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(PULL_NONE, addr);

    /* Write to GPPUDCLK0/1 to remove the clock. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp &= (~mask);        
    iowrite32(tmp, addr);
}

void SetGpioPinDirection(char pin, char direction){

    unsigned int base_addr; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of function selection register. */
    base_addr = GetGPFSELReg(pin);

    /* Calculate gpio pin offset. */
    pin = GetGPIOPinOffset(pin);    
    
    /* Set gpio pin direction. */
    addr = ioremap(base_addr, 4);
    tmp = ioread32(addr);

    mask = ~(0b111 << (pin*3));
    tmp &= mask;

    mask = (direction & 0b111) << (pin*3);
    tmp |= mask;

    iowrite32(tmp, addr);
}

int send_data(char *buff, int n){

    unsigned int temp;
    short int i;

    /* Clear S reg */
    iowrite32(CLEAR_STATUS, reg_s);

    /* Setup C reg */
    iowrite32(SETUP_CTRL_SEND, reg_c);

    /* Write to FIFO reg */
    for(i = 0; i < n; i++){
        iowrite32((unsigned int)buff[i], reg_fifo);
    }

    /* Setup DLEN reg */
    iowrite32((unsigned int)n, reg_dlen);
    
    /* Starting transfer */
    iowrite32(START_TRANSFER_SEND, reg_c);

    /* Polling */
    do {
        temp = ioread32(reg_s);
    } while(!(temp & (1 << 1))); // While !DONE

    temp = ioread32(reg_s);
    temp &= 1 << 8;
    
    /* If there is error transfer */
    if(temp)
        return -1;
    else
        return 0;

}

int receive_data(char *buff, int n){

    unsigned int temp;
    unsigned int temp_d;
    unsigned short i;

    i = 0;

    memset(buff, '\0', n);
    
    /* Clear status register before new transmision */
    iowrite32(CLEAR_STATUS, reg_s);

    /* Ready C reg for read, clear fifo */
    iowrite32(SETUP_CTRL_RECIVE, reg_c);

    /* Set expected data length */
    iowrite32((unsigned int)n, reg_dlen);
    
    /* Start transfer */
    iowrite32(START_TRANSFER_RECIVE, reg_c);

    /* Waiting for DONE = 1 */
    do{
        temp = ioread32(reg_s);
    }while(!(temp & (1 << 1)));

    /* Reading data from fifo while there is some */
    do{
        temp = ioread32(reg_s);
        temp &= 1<<5;
        temp_d = ioread32(reg_fifo);
        buff[i] = temp_d;
        i++;
        if(i == n)
            break;                  
    }while(temp);

    temp = ioread32(reg_s);
    temp &= 1 << 8;
    
    if(temp)
        return -1;
    else
        return 0;

}

void init_nuunchuk(void)
{  
    int flag;

    char init_message1[] = {0xF0, 0x55};  
    char init_message2[] = {0xFB, 0x00};

    flag = send_data(init_message1, ARRAY_SIZE(init_message1));

    if (flag < 0) {
        printk(KERN_ALERT "Unable to init nunchuk(message 1)");
        return -1;
    }

    buffer = kmalloc(buff_len, GFP_KERNEL);
    if(!buffer)
    {
        kfree(buffer);
        return -1;
    }


    mdelay(10);
    
    flag = send_data(init_message2, ARRAY_SIZE(init_message2));

    if (flag < 0) {
        printk(KERN_ALERT "Unable to init nunchuk(message 2)");
        return -1;
    }


    


}




/*
 * Initialization:
 *  1. Register device driver
 *  2. Allocate buffer
 *  3. Initialize buffer
 *  4. Map GPIO Physical address space to virtual address
 *  5. Initialize GPIO pins
 *  6. Init and start the high resoultion timer
 */
int gpio_driver_init(void)
{
    int result = -1;

    printk(KERN_INFO "Inserting gpio_driver module\n");

    /* Registering device. */
    result = register_chrdev(0, "gpio_driver", &gpio_driver_fops);
    if (result < 0)
    {
        printk(KERN_INFO "gpio_driver: cannot obtain major number %d\n", major);
        return result;
    }

    major = result;
    printk(KERN_INFO "gpio_driver major number is %d\n", major);

    if (request_mem_region(BSC1_BASE_ADDR, 0x20, "gpio_driver") == NULL) {
        printk(KERN_INFO "Data memory not available\n");
	    unregister_chrdev(major, "gpio_driver");
        return -1;
    }

    /* Initialize GPIO pins. */
    SetGpioPinDirection(GPIO_02, GPIO_DIRECTION_ALT0);
    SetGpioPinDirection(GPIO_03, GPIO_DIRECTION_ALT0);
    SetInternalPullUpDown(GPIO_02, PULL_UP);
    SetInternalPullUpDown(GPIO_03, PULL_UP);
   
    reg_c = ioremap(BSC1_REG_C, 4);
    reg_dlen = ioremap(BSC1_REG_DLEN, 4);
    reg_slave_addr = ioremap(BSC1_REG_SLAVE_ADDR, 4);
    reg_fifo = ioremap(BSC1_REG_FIFO, 4);
    reg_s = ioremap(BSC1_REG_S, 4);
    reg_div = ioremap(BSC1_REG_DIV, 4);

    iowrite32(I2C_ADDRESS, reg_slave_addr);		

    iowrite32(0x000009C4, reg_div);

    init_nuunchuk();

    return 0;
}

/*
 * Cleanup:
 *  1. stop the timer
 *  2. release GPIO pins (clear all outputs, set all as inputs and pull-none to minimize the power consumption)
 *  3. Unmap GPIO Physical address space from virtual address
 *  4. Free buffer
 *  5. Unregister device driver
 */
void gpio_driver_exit(void)
{
    printk(KERN_INFO "Removing gpio_driver module\n");

    /* Clear GPIO pins. */
    release_mem_region(BSC1_BASE_ADDR, 0x20);
    /* Set GPIO pins as inputs and disable pull-ups. */
    SetInternalPullUpDown(GPIO_02, PULL_NONE);
    SetInternalPullUpDown(GPIO_03, PULL_NONE);

    /* Freeing the major number. */
    unregister_chrdev(major, "gpio_driver");
}

/* File open function. */
static int gpio_driver_open(struct inode *inode, struct file *filp)
{
    /* Initialize driver variables here. */

    /* Reset the device here. */

    /* Success. */
    try_module_get(THIS_MODULE);
    return 0;
}

/* File close function. */
static int gpio_driver_release(struct inode *inode, struct file *filp)
{
    /* Success. */
    module_put(THIS_MODULE);
    return 0;
}

/*
 * File read function
 *  Parameters:https://www.facebook.com/
 *   filp  - a type file structure;
 *   buf   - a buffer, from which the user space function (fread) will read;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           value as the usual counter in the user space function (fread);
 *   f_pos - a position of where to start reading the file;
 *  Operation:
 *   The gpio_driver_read function transfers data from the driver buffer (gpio_driver_buffer)
 *   to user space with the function copy_to_user.
 */

int read_registers(u8 *reg_values, int n)
{

    char read_request = 0x00;

    int flag = send_data(&read_request, 1);

   // repeat:
        if(flag < 0)
        {
            printk(KERN_ALERT "Failed to send request message");
       //     init_nuunchuk();
     //       goto repeat;
        }

        return receive_data(reg_values, n);
}


static ssize_t gpio_driver_read(struct file *filp, char *buf, size_t len, loff_t *f_pos)
{
    u8 reg_values[6];
    reg_values[5] = 0xFF;
    char mushroom_x;
    char mushroom_y;
    char accelerometer_x;
    char accelerometer_y;
    char accelerometer_z;
    char c_button;
    char z_button;
    int i;

        if(read_registers(reg_values, 6) < 0)
        {
            printk(KERN_ALERT "Unable to read registers");
            init_nuunchuk();
        }

        mushroom_x = reg_values[0];
        mushroom_y = reg_values[1];
        accelerometer_x = reg_values[2];
        accelerometer_y = reg_values[3];
        accelerometer_z = reg_values[4];

        z_button = reg_values[5] & 0x01;
        z_button ^= 0x01;

        c_button = reg_values[5] & 0x02;
        c_button >>= 1;
        c_button ^= 0x01;

        buffer[0] = mushroom_x;
        buffer[1] = mushroom_y;
        buffer[2] = accelerometer_x;
        buffer[3] = accelerometer_y;
        buffer[4] = accelerometer_z;
        buffer[5] = z_button;
        buffer[6] = c_button;

        if(copy_to_user(buf, buffer, buff_len) != 0)
        {
            printk("Unable to copy to user");
            return -1;
        }   

}

/*
 * File write function
 *  Parameters:
 *   filp  - a type file structure;
 *   buf   - a buffer in which the user space function (fwrite) will write;
 *   len - a counter with the number of bytes to transfer, which has the same
 *           values as the usual counter in the user space function (fwrite);
 *   f_pos - a position of where to start writing in the file;
 *  Operation:
 *   The function copy_from_user transfers the data from user space to kernel space.
 */

static ssize_t gpio_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{



}
