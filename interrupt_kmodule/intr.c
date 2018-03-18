/*
 * Sample driver for making GPIO to interrupt
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.
 *
 * AUTHOR : Titusrathinaraj Stalin
 * EMAIL  : x0213399@ti.com
 *	
 */



/* Include linux kernel header files */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <asm/gpio.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Titus S");
MODULE_DESCRIPTION("GPIO interrupt sample driver");

/* OMAPL138_LCDK : S3 switch */

#define GPIO_INT 7

/* GPIO No 141 -> GPIO8[12] : OMAPL138_LCDK : J15[16] */

//#define GPIO_INT 165 //input key for SDI OMAPL138
//#define GPIO_INT1 166 //output led1 for SDI OMAPL138
//#define GPIO_INT2 167 //output led2 for SDI OMAPL138

#define gpio_int "gpio_int"

/* local functions */
static int gpio_irq_open (struct inode *inode, struct file *filp);
static int gpio_irq_release (struct inode *inode, struct file *filp);
static int gpio_irq_fasync(int fd, struct file *filp, int on);

static int major;
static struct class *class_gpio_int;
static struct device *dev_gpio_int;


static struct fasync_struct *gpio_irq_async_queue;

/* declare file operation structure */
static struct file_operations gpio_irq_fops = {
	.owner = THIS_MODULE,
	.open  = gpio_irq_open,
	.release = gpio_irq_release,
	.fasync	= gpio_irq_fasync,
};

/*Interrupt Handler */
static irq_handler_t gpio_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	printk("\n GPIO interrupt raised; IRQ No : %d\n",irq);
	kill_fasync(&gpio_irq_async_queue, SIGIO, POLL_IN);
	return IRQ_HANDLED;
}

/***************************************************************************
 * Function             - gpio_request_irq
 * Functionality        - Registers Handlers to the IRQ's of GPIO's by request_irq
 * Input Params - 
 * Return Value - None
 * Note                 - None
 ****************************************************************************/
void gpio_request_irq(int irq_num)
{
	int status = 0;

//	irq_set_irq_type(irq_num,IRQ_TYPE_EDGE_BOTH);

	status = request_irq(irq_num, (irq_handler_t)gpio_interrupt,0,"gpio_int", NULL);

/* Need to use 'IRQ_TYPE_EDGE_FALLING' since the switch S3 is pulled high and it should raise interrupt when we pressed switch (ie falling edge) */

	irq_set_irq_type(irq_num,IRQ_TYPE_EDGE_BOTH);

	printk(KERN_WARNING "gpio_request_irq %d status %d\n",
			   irq_num, status);

	if (status == -EINVAL) {
		printk("<1> EINVAL \n");
		} 
	else if (status == -ENOMEM) {
		printk("<1> ENOMEM ");
		} 		
		
	if (status < 0) {
		printk("<1> gpio_request_irq : Failed to Register IRQ %d  \n",
		       irq_num);
		printk("<1> gpio_request_irq : return status is %d  \n",
		       status);
	}

}

/***************************************************************************
 * Function             - gpio_unrequest_irq
 * Functionality        - Free Handlers o the IRQ's of GPIO's
 * Input Params - 
 * Return Value - None
 * Note                 - None
 ****************************************************************************/
void gpio_unrequest_irq(int irq_num)
{
	free_irq(irq_num, NULL);
//	gpio_free(GPIO_INT1);
//	gpio_free(GPIO_INT2);
	printk("<1> gpio_unrequest_irq :  Freeing IRQ %d  \n", irq_num);

}
/****************************************************************************/


static int gpio_irq_fasync(int fd, struct file *filp, int on)
{
	int temp;
	temp = fasync_helper(fd, filp, on, &gpio_irq_async_queue);
	if (fd != -1)
		kill_fasync(&gpio_irq_async_queue, SIGIO, POLL_IN);
	return(temp);
}

void gpio_init(void)
{
	unsigned int status,tmp;

	status = gpio_request(GPIO_INT, "gpio_ctrl");
	printk(KERN_ALERT "gpio_ctrl : GPIO NO: %d Status %d\n",GPIO_INT,status);
//	status = gpio_request(GPIO_INT1, "gpio_ctrl");
//	printk(KERN_ALERT "gpio_ctrl : GPIO NO: %d Status %d\n",GPIO_INT1,status);
//	status = gpio_request(GPIO_INT2, "gpio_ctrl");
//	printk(KERN_ALERT "gpio_ctrl : GPIO NO: %d Status %d\n",GPIO_INT2,status);

//	gpio_direction_output(GPIO_INT1, 0);
//	gpio_direction_output(GPIO_INT2, 0);
	gpio_direction_input(GPIO_INT);


	tmp = gpio_to_irq(GPIO_INT);
	printk(KERN_ALERT "IRQ is %d\n",tmp);
	gpio_request_irq(tmp);
}

/******************************************************************************
 * gpio_irq_open - do nothing 
 *****************************************************************************/
static int gpio_irq_open (struct inode *inode, struct file *filp)
{
    return 0;
}    

/******************************************************************************
 * gpio_irq_release - do nothing 
 *****************************************************************************/
static int gpio_irq_release (struct inode *inode, struct file *filp)
{
	if (filp->f_flags & FASYNC) {
		gpio_irq_fasync (-1, filp, 0);
	}
	return 0;
} 

/*****************************************************************************
 * initialise user gpio module
 ****************************************************************************/
static int gpio_irq_init(void)
{
	static int status;

	void *ptr_err;
	if ((major = register_chrdev(0, gpio_int, &gpio_irq_fops)) < 0)
		return major;

	class_gpio_int = class_create(THIS_MODULE, gpio_int);
	if (IS_ERR(ptr_err = class_gpio_int))
		goto err2;

	dev_gpio_int = device_create(class_gpio_int, NULL, MKDEV(major, 0), NULL, gpio_int);
	if (IS_ERR(ptr_err = dev_gpio_int))
		goto err;

    	gpio_init();

	printk(KERN_ALERT "GPIO IRQ device is inserted sucessfully\n");

    	return 0;

err:
	class_destroy(class_gpio_int);
err2:
	unregister_chrdev(major, gpio_int);
	return PTR_ERR(ptr_err);

}

/*****************************************************************************
 * cleanup user gpio module
 ****************************************************************************/
static void gpio_irq_exit(void)
{

//	gpio_direction_output(GPIO_INT1,0);
//	gpio_direction_output(GPIO_INT2,0);

mdelay(500);
//printk("Im done...\n");
//	gpio_direction_output(GPIO_INT1,1);
//	gpio_direction_output(GPIO_INT2,1);

//mdelay(1000);
//printk("Im down...\n");

	gpio_unrequest_irq(gpio_to_irq(GPIO_INT));

	device_destroy(class_gpio_int, MKDEV(major, 0));
	class_destroy(class_gpio_int);
	printk(KERN_ALERT "GPIO IRQ device is ejected sucessfully\n");
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);
