
/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
//#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm/param.h> /* include HZ */
#include <linux/string.h> /* string operations */
#include <linux/timer.h> /* timer gizmos */
#include <linux/list.h> /* include list data struct */
#include <linux/interrupt.h>
#include <linux/random.h>


MODULE_LICENSE("MIT");
MODULE_AUTHOR("Naved Ansari");    
MODULE_DESCRIPTION("Fake OpenBCI board device");  
MODULE_VERSION("1.0");  

/* Declaration of memory.c functions */
static int board_open(struct inode *inode, struct file *filp);
static int board_release(struct inode *inode, struct file *filp);
static ssize_t board_read(struct file *filp,char *buf, size_t count, loff_t *f_pos);
static ssize_t board_write(struct file *filp,const char *buf, size_t count, loff_t *f_pos);
static void board_exit(void);
static int board_init(void);
static void timer_handler(unsigned long data);

volatile int packetno=0; //to count no. of packets
volatile int channel[9];

/* Structure that declares the usual file */
/* access functions */
struct file_operations board_fops = {
	read: board_read,
	write: board_write,
	open: board_open,
	release: board_release
};

/* Declaration of the init and exit functions */
module_init(board_init);
module_exit(board_exit);

static unsigned capacity = 256; //buffer capacity
module_param(capacity, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int board_major = 61;

/* Buffer to store data */
static char *board_buffer;
/* length of the current message */
static int board_len;

/* timers, not curently used */
static struct timer_list the_timer[1];
int timePer = 500;

char command = 's'; //command that is sent to boards


/* the init function is called once, when the module is inserted */
static int board_init(void) 
{
	int result, ret; 
	/* Registering device */
	result = register_chrdev(board_major, "board", &board_fops);
	if (result < 0)
	{
		printk(KERN_ALERT "board: cannot obtain major number %d\n", board_major);
		return result;
	}
	/* Allocating board for the buffer */
	board_buffer = kmalloc(capacity, GFP_KERNEL);
	/* Check if all right */
	if (!board_buffer)
	{ 
		printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
		goto fail; 
	} 

	memset(board_buffer, 0, capacity); //init buffer to zero
	board_len = 0;
	printk(KERN_EMERG "Inserting board module\n"); 

	//set up timers, not currently used
	setup_timer(&the_timer[0], timer_handler, 0);
	ret = mod_timer(&the_timer[0], jiffies + msecs_to_jiffies(timePer/2));


	return 0;
	fail: 
	board_exit(); 
	return result;
}

/*exit function is called when module is rmmod */
static void board_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(board_major, "board");
	/* Freeing buffer memory */
	if (board_buffer)
		kfree(board_buffer);
	/* Freeing timer list */
	//if (the_timer)
	//	kfree(the_timer);
	/* removing timer stuff, currently not used*/
	if (timer_pending(&the_timer[0]))
		del_timer(&the_timer[0]);
	printk(KERN_ALERT "Removing board module\n");

}
/*i still dont know what this does, apart printgint pid */
static int board_open(struct inode *inode, struct file *filp)
{
	/*printk(KERN_INFO "open: process id %d, command %s\n",
		current->pid, current->comm);*/
	/* Success */
	return 0;
}
/*don't know what this does either */
static int board_release(struct inode *inode, struct file *filp)
{
	/*printk(KERN_INFO "release called: process id %d, command %s\n",
		current->pid, current->comm);*/
	/* Success */
	return 0;
}

/*whenever someone writes to the board, this is called */
static ssize_t board_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	int i;
	if (copy_from_user(board_buffer, buf, 1)) //copy 1 character from the user
	{
		return -EFAULT;
	}
	printk(KERN_EMERG "\nboard_buffer[0] = %c", board_buffer[0]);
    
    /*see what that character is and set the command*/
     switch (board_buffer[0])
    {
    	case 'b':
	    	command = 'b'; //stream
	    	break;
    	case 'v':
	    	command = 'v'; //reset
	    	break;
    	case 's':
	    	command = 's'; //stop
	    	break;
	    case '1':
	    	channel[1] = -1;
	    	break;
	    case '2':
	    	channel[2] = -1;
	    	break;
	    case '3':
	    	channel[3] = -1;
	    	break;
	    case '4':
	    	channel[4] = -1;
	    	break;
	    case '5':
	    	channel[5] = -1;
	    	break;
	    case '6':
	    	channel[6] = -1;
	    	break;
	    case '7':
	    	channel[7] = -1;
	    	break;
	    case '8':
	    	channel[8] = -1;
	    	break;
	    case '!':
	    	channel[1] = 2;
	    	break;
	    case '@':
	    	channel[2] = 2;
	    	break;
	    case '#':
	    	channel[3] = 2;
	    	break;
	    case '$':
	    	channel[4] = 2;
	    	break;
	    case '%':
	    	channel[5] = 2;
	    	break;
	    case '^':
	    	channel[6] = 2;
	    	break;
	    case '&':
	    	channel[7] = 2;
	    	break;
	    case '*':
	    	channel[8] = 2;
	    	break;
	    case 'd':
	    	 for(i=0; i<9; i++) channel[i] = 2;
	    	 break;
	    case 'D':
	    	 command = 'D';
    	default:
    		command = 'u'; //
    }

    return count;
}

/*this function is called, whenever something reads from the device */
static ssize_t board_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{ 
	char tbuf[33]; //the 33 byte packet that the board will send
	char rand; //random byte
	int i; //counter variable
	//printk(KERN_EMERG "\nIn board_read, command = %c", command); //just for debugging
	
	if(command == 'b')
	{
		//sprintf(tbuf,"\nI will stream data now");
		packetno++; //increment no. of packets
		//start streaming data, make the packet and then put it on tbuf
		//make fake accel data here. 
		/*float accelArray[3] = {0};
		accelArray[0] = (randomnum(time(NULL)) * 0.1 * (randomnum(time(NULL)) > 0.5 ? -1 : 1)); //x
		accelArray[1] = (randomnum(time(NULL)+2) * 0.1 * (randomnum(time(NULL)+3) > 0.5 ? -1 : 1)); //y
		accelArray[2] = (randomnum(time(NULL)) * 0.4 * (randomnum(time(NULL)) > 0.5 ? -1 : 1)); //z */
	
		//printk(KERN_EMERG "\nIn kernel, starting stream, packet %d", packetno); 
		tbuf[0] = 0xA0; //init the 1st byte
		for(i=1; i<32; i++)
		{
			//tbuf[i] = 0x23;
			get_random_bytes(&rand, sizeof(rand)); //init the bytes randomly
			tbuf[i] = rand;
		}
		tbuf[32]= 0xC0; //init the last byte
		//printk(KERN_EMERG "\nchannel 2 value = %d", channel[2]);
		/*turn off channels here
		setting to 56 instead of 0 because of some bug*/
		if(channel[1] == -1)
			{
				printk(KERN_EMERG "\nChannel 1 is OFF");
				tbuf[2] = 0x56;
				tbuf[3] = 0x56;
				tbuf[4] = 0x56;
			}
		if(channel[2] == -1)
			{
				printk(KERN_EMERG "\nChannel 2 is OFF");
				tbuf[5] = 0x56;
				tbuf[6] = 0x56;
				tbuf[7] = 0x56;
			}
		if(channel[3] == -1)
			{
				printk(KERN_EMERG "\nChannel 3 is OFF");
				tbuf[8] = 0x56;
				tbuf[9] = 0x56;
				tbuf[10] = 0x56;
			}
		if(channel[4] == -1)
			{
				printk(KERN_EMERG "\nChannel 4 is OFF");
				tbuf[11] = 0x56;
				tbuf[12] = 0x56;
				tbuf[13] = 0x56;
			}
		if(channel[5] == -1)
			{
				printk(KERN_EMERG "\nChannel 5 is OFF");
				tbuf[14] = 0x56;
				tbuf[15] = 0x56;
				tbuf[16] = 0x56;
			}
		if(channel[6] == -1)
			{
				printk(KERN_EMERG "\nChannel 6 is OFF");
				tbuf[17] = 0x56;
				tbuf[18] = 0x56;
				tbuf[19] = 0x56;
			}
		if(channel[7] == -1)
			{
				printk(KERN_EMERG "\nChannel 7 is OFF");
				tbuf[20] = 0x56;
				tbuf[21] = 0x56;
				tbuf[22] = 0x56;
			}
		if(channel[8] == -1)
			{
				printk(KERN_EMERG "\nChannel 8 is OFF");
				tbuf[23] = 0x56;
				tbuf[24] = 0x56;
				tbuf[25] = 0x56;
			}

		for(i=0; i<33; i++)
		{
			printk(KERN_EMERG "\nBYTE = %x",tbuf[i]);
		}


	}
	else if(command == 'v')
	{
		//sprintf(bigbuff, "OpenBCI V3 8-16 channel\nDS1299 Device ID: 0x3E\nLIS3DH Device ID: 0x33\nFirmware: v2.0.0\n$$$");
		sprintf(tbuf,"\nOpenBCI-RESETINFO\nDEVID\n$$$"); 
		//handle reset
	}
	else if (command == 's')
	{
		sprintf(tbuf,"\n Streaming stopped");
		//handle stop
	}
	else if (command == 'D')
	{
		//send defualt settings info
		sprintf(tbuf, "123456$$$"); //send 6 ASCII characters followed by some dough!
	}
	else
	{
		sprintf(tbuf, "\nUnknown command");
		//handle unkown condition
	}

	strcpy(board_buffer,tbuf); //put whatever we need into the board buffer

	//do not go over then end
	if (count > 256 - *f_pos)
		count = 256 - *f_pos;

	if (copy_to_user(buf,board_buffer, sizeof(tbuf))) //copy the buffer to user space
	{
		printk(KERN_EMERG "Couldn't copy to user\n"); 
		return -EFAULT;	
	}
	
	// Changing reading position as best suits 
	*f_pos += count; 
	return count; 
}

/*function that sets up the timer */
static void timer_handler(unsigned long data) 
{
	//call somefuntion to do something when timer expires
	mod_timer(&the_timer[0],jiffies + msecs_to_jiffies(timePer/2)); //reset the timer

}	






