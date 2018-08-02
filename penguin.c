/*
File: penguin.c
Name: Sai Herng
Project: Penguin Printer
*/



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>


MODULE_LICENSE("GPL");  /* Kernel needs this license. */

#define ENTRY_NAME "penguin"
#define PERMS 0644
#define PARENT NULL

/* Function declarations */
ssize_t procfile_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos);

ssize_t procfile_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos);

static int run(void* data);

/* Global variables go here */
int printQueue[20];
struct mutex my_mutex;
int i = 0;
int TotalProcessed = 0;


static struct file_operations penguin_proc_ops = {
   .owner = THIS_MODULE,
   .read = procfile_read,
   .write = procfile_write,
};

struct task_struct *t; /* Printer thread task struct (use to start/stop)*/


/* The thread will run this function.  The thread will stop
 * when either penguin_stop(t) is called or else the function ends. */
static int run(void* data) {
   

   /* Print hello message until we are unloaded */
   while(!kthread_should_stop()) {

      printk("Printer checking slot %i\n", i);

      if (printQueue[i] == 1) {
      	ssleep(2);
      }
      	
      else if (printQueue[i] == 2){
      	ssleep(3);
      }

      else if (printQueue[i] == 3){
      	ssleep(3);
      }

      else if (printQueue[i] == 4){
      	ssleep(5);
  	  }

      else if (printQueue[i] == 5){
      	ssleep(8);
      }

      

      /* Sleep one second */
      ssleep(1);
      i++;
      TotalProcessed++;

      if (i > 19){
      	i = 0;
      }
    }

   return 0;
}


int penguin_proc_init(void) {

   proc_create_data(ENTRY_NAME, 0, NULL, &penguin_proc_ops, NULL);
   
   /* This message will print in /var/log/syslog or on the first tty. */
   printk("/proc/%s created\n", ENTRY_NAME);

   /* Start the printer -- move out of here later. */
   
   return 0;
}

ssize_t procfile_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

   static int finished = 0;
   int ret;
   char ret_buf[80];

   /* Are we done reading? If so, we return 0 to indicate end-of-file */
   if (finished) {
	finished=0;
	return 0;
   }

   finished = 1;

   /* This message will print in /var/log/syslog or on the first tty. */
   printk("/proc/%s read called.\n", ENTRY_NAME);



   ret=sprintf(ret_buf,"Printer checking slot %d\n",i);

   if(copy_to_user(buf, ret_buf, ret)) {
      ret = -EFAULT;  
   }

   return ret;
}

ssize_t procfile_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{

    char *page; /* don't touch */
    int my_data = 0;

    
    /* Allocating kernel memory, don't touch. */
    page = (char *) vmalloc(count);
    if (!page)
       return -ENOMEM;   

    /* Copy the data from the user space.  Data is placed in page. */ 
    if (copy_from_user(page, buf, count)) {
       vfree(page);
       return -EFAULT;
    }

   /* Now do something with the data, here we just print it */
    sscanf(page,"%d",&my_data);
    printk("User has sent the value of %d\n", my_data);


    if (my_data == 0){

    	mutex_unlock(&my_mutex);
    	t = kthread_run(run, NULL, "my_thread");
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    }

    else if (my_data == 1){
    	mutex_unlock(&my_mutex);
    	printQueue[i] = 1;
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    }
    	
    else if (my_data == 2){
    	
    	mutex_unlock(&my_mutex);
    	printQueue[i] = 2;
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    	
    }
    else if (my_data == 3){
    	
    	mutex_unlock(&my_mutex);
    	printQueue[i] = 3;
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    	
    }
    else if (my_data == 4){
    	
    	mutex_unlock(&my_mutex);
    	printQueue[i] = 4;
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    	
    }
    	
    else if (my_data == 5){
    	
    	mutex_unlock(&my_mutex);
    	printQueue[i] = 5;
    	if (mutex_lock_interruptible(&my_mutex)){
    		return -ERESTARTSYS;
    	}
    	
    	
    }

    /* Free the allocated memory, don't touch. */
    vfree(page); 
    return count;
}

void penguin_proc_exit(void)
{

   /* Will block here and wait until kthread stops */
   kthread_stop(t);

   remove_proc_entry(ENTRY_NAME, NULL);
   printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(penguin_proc_init);
module_exit(penguin_proc_exit);

