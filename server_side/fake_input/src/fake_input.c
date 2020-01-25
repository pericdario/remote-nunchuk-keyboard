#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>

#include <asm/irq.h>
#include <asm/io.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

static int gpio_driver_open(struct inode *, struct file *);
static int gpio_driver_release(struct inode *, struct file *);
static ssize_t gpio_driver_write(struct file *, const char *buf, size_t , loff_t *);

static struct input_dev *button_dev;
const char name[] = "fake_keyboard";

#define BUF_LEN 80
char* gpio_driver_buffer;

int gpio_driver_major;

//timer
static struct hrtimer blink_timer;
static ktime_t kt;

//static int BUTTON_IRQ = -1;

/*static irqreturn_t button_interrupt(int irq, void *dummy)
{
        //input_report_key(button_dev, BTN_A, inb(BUTTON_PORT) & 1);
		input_event(button_dev, EV_KEY, BTN_A, 0);
        input_sync(button_dev);
        return IRQ_HANDLED;
}*/

struct file_operations gpio_driver_fops =
{
    open    :   gpio_driver_open,
    release :   gpio_driver_release,
    write   :   gpio_driver_write
};

static char flag_player1 = 0;
static char flag_player2 = 0;

static char input[4];

static char flag_cancel = 0;

void funkPlayer1Clear(char inputParam)
{
	if(inputParam == 1)
	{
		input_event(button_dev, EV_KEY, KEY_W, 0);
		input_sync(button_dev);
	}
	if(inputParam == 2)
	{
		input_event(button_dev, EV_KEY, KEY_A, 0);
		input_sync(button_dev);
	}
	if(inputParam == 3)
	{
		input_event(button_dev, EV_KEY, KEY_S, 0);
		input_sync(button_dev);
	}
	if(inputParam == 4)
	{
		input_event(button_dev, EV_KEY, KEY_D, 0);
		input_sync(button_dev);
	}
	if(inputParam == 5)
	{
		input_event(button_dev, EV_KEY, KEY_Q, 0);
		input_sync(button_dev);
	}
	if(inputParam == 6)
	{
		input_event(button_dev, EV_KEY, KEY_E, 0);
		input_sync(button_dev);
	}
}

void funkPlayer2Clear(char inputParam)
{
	if(inputParam == 1)
	{
		input_event(button_dev, EV_KEY, KEY_I, 0);
		input_sync(button_dev);
	}
	if(inputParam == 2)
	{
		input_event(button_dev, EV_KEY, KEY_J, 0);
		input_sync(button_dev);
	}
	if(inputParam == 3)
	{
		input_event(button_dev, EV_KEY, KEY_K, 0);
		input_sync(button_dev);
	}
	if(inputParam == 4)
	{
		input_event(button_dev, EV_KEY, KEY_L, 0);
		input_sync(button_dev);
	}
	if(inputParam == 5)
	{
		input_event(button_dev, EV_KEY, KEY_O, 0);
		input_sync(button_dev);
	}
	if(inputParam == 6)
	{
		input_event(button_dev, EV_KEY, KEY_P, 0);
		input_sync(button_dev);
	}
}

void funkPlayer1(char inputParam)
{
	if(inputParam == 1)
	{
		input_event(button_dev, EV_KEY, KEY_W, 1);
		input_sync(button_dev);
	}
	if(inputParam == 2)
	{
		input_event(button_dev, EV_KEY, KEY_A, 1);
		input_sync(button_dev);
	}
	if(inputParam == 3)
	{
		input_event(button_dev, EV_KEY, KEY_S, 1);
		input_sync(button_dev);
	}
	if(inputParam == 4)
	{
		input_event(button_dev, EV_KEY, KEY_D, 1);
		input_sync(button_dev);
	}
	if(inputParam == 5)
	{
		input_event(button_dev, EV_KEY, KEY_Q, 1);
		input_sync(button_dev);
	}
	if(inputParam == 6)
	{
		input_event(button_dev, EV_KEY, KEY_E, 1);
		input_sync(button_dev);
	}
}

void funkPlayer2(char inputParam)
{
	if(inputParam == 1)
	{
		input_event(button_dev, EV_KEY, KEY_I, 1);
		//input_event(button_dev, EV_KEY, KEY_I, 0);
		input_sync(button_dev);
	}
	if(inputParam == 2)
	{
		input_event(button_dev, EV_KEY, KEY_J, 1);
		//input_event(button_dev, EV_KEY, KEY_J, 0);
		input_sync(button_dev);
	}
	if(inputParam == 3)
	{
		input_event(button_dev, EV_KEY, KEY_K, 1);
		//input_event(button_dev, EV_KEY, KEY_K, 0);
		input_sync(button_dev);
	}
	if(inputParam == 4)
	{
		input_event(button_dev, EV_KEY, KEY_L, 1);
		//input_event(button_dev, EV_KEY, KEY_L, 0);
		input_sync(button_dev);
	}
	if(inputParam == 5)
	{
		input_event(button_dev, EV_KEY, KEY_O, 1);
		input_sync(button_dev);
	}
	if(inputParam == 6)
	{
		input_event(button_dev, EV_KEY, KEY_P, 1);
		input_sync(button_dev);
	}
}

/*static enum hrtimer_restart blink_timer_callback(struct hrtimer *param) //funkcija koju tajmer poziva svaku sekundu
{

	if(flag_player1)
	{
		funkPlayer1(input[0]);
	}
	if(flag_player2)
	{
		funkPlayer2(input[1]);
	}
	if(flag_backspace)
	{
		input_event(button_dev, EV_KEY, KEY_BACKSPACE, 1);
		input_event(button_dev, EV_KEY, KEY_BACKSPACE, 0);
		input_sync(button_dev);
		flag_backspace = 0;
	}

	hrtimer_forward(&blink_timer, ktime_get(), kt); //produzavanje tajmera

	return HRTIMER_RESTART;
}*/

static int __init button_init(void)
{
        int error;

        int result = -1;

        result = register_chrdev(0, "gpio_driver", &gpio_driver_fops);

        gpio_driver_major = result;

        printk("Major is %d", gpio_driver_major);

        /*if (request_irq(BUTTON_IRQ, button_interrupt, 0, "button", NULL)) {
                printk(KERN_ERR "button.c: Can't allocate irq %d\n", BUTTON_IRQ);
                return -EBUSY;
        }*/

        button_dev = input_allocate_device();
        if (!button_dev) {
                printk(KERN_ERR "button.c: Not enough memory\n");
                error = -ENOMEM;
                //goto err_free_irq;
        }
        //printk("allocation done");

        button_dev->name = name;

        button_dev->id.bustype = 0x3;

        button_dev->evbit[0] = BIT_MASK(EV_KEY);
        //button_dev->keybit[BIT_WORD(KEY_A)] = BIT_MASK(KEY_A);
        //button_dev->keybit[BIT_WORD(KEY_S)] = BIT_MASK(KEY_S);

        set_bit(KEY_W, button_dev->keybit);
        set_bit(KEY_A, button_dev->keybit);
        set_bit(KEY_S, button_dev->keybit);
        set_bit(KEY_D, button_dev->keybit);

        set_bit(KEY_Q, button_dev->keybit);
        set_bit(KEY_E, button_dev->keybit);

        set_bit(KEY_I, button_dev->keybit);
        set_bit(KEY_J, button_dev->keybit);
        set_bit(KEY_K, button_dev->keybit);
        set_bit(KEY_L, button_dev->keybit);

        set_bit(KEY_O, button_dev->keybit);
        set_bit(KEY_P, button_dev->keybit);

        set_bit(KEY_BACKSPACE, button_dev->keybit);

        //printk("BIT_MASK done");

        error = input_register_device(button_dev);
        if (error) {
                printk(KERN_ERR "button.c: Failed to register device\n");
                goto err_free_dev;
        }

        //printk("Register device done");

        //input_event(button_dev, EV_KEY, BTN_A, 0);
        //input_sync(button_dev);

		//input_event(button_dev, EV_KEY, KEY_S, 1);
		//input_sync(button_dev);
		//input_event(button_dev, EV_KEY, KEY_S, 0);

        /*hrtimer_init(&blink_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        kt = ktime_set(0, 100);
		blink_timer.function = &blink_timer_callback;
		hrtimer_start(&blink_timer, kt, HRTIMER_MODE_REL);*/

        //input_report_key(button_dev, BTN_A, inb(BUTTON_PORT) & 1);
        //printk("Timer started");

        gpio_driver_buffer = kmalloc(BUF_LEN, GFP_KERNEL);
        memset(gpio_driver_buffer, 0, BUF_LEN);

        return 0;

err_free_dev:
        input_free_device(button_dev);
        printk("Freeing device?");
/*err_free_irq:
        free_irq(BUTTON_IRQ, button_interrupt);
        return error;*/
}

static void __exit button_exit(void)
{
        input_unregister_device(button_dev);

        kfree(gpio_driver_buffer);

        unregister_chrdev(gpio_driver_major, "gpio_driver");

       // hrtimer_cancel(&blink_timer);
        //free_irq(BUTTON_IRQ, button_interrupt);
}

static int gpio_driver_open(struct inode *inode, struct file *filp)
{
    /* Initialize driver variables here. */

    /* Reset the device here. */

    /* Success. */
    return 0;
}

/* File close function. */
static int gpio_driver_release(struct inode *inode, struct file *filp)
{
    /* Success. */
    return 0;
}

static ssize_t gpio_driver_write(struct file *filp, const char *buf, size_t len, loff_t *f_pos)
{
   
    memset(gpio_driver_buffer, 0, BUF_LEN);

    
    if (copy_from_user(gpio_driver_buffer, buf, len) != 0)
    {
        return -EFAULT;
    }
    else
    {
        if(gpio_driver_buffer[2] == 'W')
        {
        	funkPlayer1(1);
        }
        else
        {
        	funkPlayer1Clear(1);
        }
        if(gpio_driver_buffer[0] == 'A')
        {
            funkPlayer1(2);
        }
        else
        {
        	funkPlayer1Clear(2);
        }
        if(gpio_driver_buffer[3] == 'S')
        {
            funkPlayer1(3);
        }
        else
        {
        	funkPlayer1Clear(3);
        }
        if(gpio_driver_buffer[1] == 'D')
        {
            funkPlayer1(4);
        }
        else
        {
        	funkPlayer1Clear(4);
        }
        //*******************************
        if(gpio_driver_buffer[8] == 'Q')
        {
        	funkPlayer1(5);
        }
        else
        {
        	funkPlayer1Clear(5);
        }
        if(gpio_driver_buffer[9] == 'E')
        {
        	funkPlayer1(6);
        }
        else
        {
        	funkPlayer1Clear(6);
        }
        //-------------------------------
        if(gpio_driver_buffer[4] == 'J')
        {
			funkPlayer2(2);
        }
		else
		{
			funkPlayer2Clear(2);
		}
        if(gpio_driver_buffer[5] == 'L')
        {
			funkPlayer2(4);
        }
		else
		{
			funkPlayer2Clear(4);
		}
        if(gpio_driver_buffer[6] == 'I')
        {
            funkPlayer2(1);
        }
		else
		{
			funkPlayer2Clear(1);
		}
        if(gpio_driver_buffer[7] == 'K')
        {
            funkPlayer2(3);
        }
		else
		{
			funkPlayer2Clear(3);
		}
		//******************************
		if(gpio_driver_buffer[10] == 'O')
		{
			funkPlayer2(5);
		}
		else
		{
			funkPlayer2Clear(5);
		}
		if(gpio_driver_buffer[11] == 'P')
		{
			funkPlayer2(6);
		}
		else
		{
			funkPlayer2Clear(6);
		}
    }
}

module_init(button_init);
module_exit(button_exit);
