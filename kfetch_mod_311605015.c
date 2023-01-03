
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include<linux/utsname.h>
#include <linux/cpumask.h>  /* for num_online_cpus */
#include<linux/mm.h>
#include<linux/string.h>
#include <linux/jiffies.h> /* for jiffies */
#include <linux/cpu.h> //for cpu_data ,cpuinfo_x86

/*
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "kfetch_mod_311605015"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80              /* Max length of the message from the device */

static int thread_count;

void get_thread_count(void)
{
    struct task_struct *task;

    for_each_process(task)
        thread_count += task->signal->nr_threads;
}
/*
 * Global variables are declared as static, so are global within the file.
 */

static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?
                                 * Used to prevent multiple access to device */
   
static char final_output[500];
static char hostname[40];//"chang311605015-virtual-machine\n"
static char kernel[40]; //Kernel:     5.19.12-os-311605015\n
static char CPU_name[40];//="CPU:     8th Gen Intel(R) Core(TM) i7-8750H\n"
static char CPU_core[40];//1
static char Mem[40];//"Mem:     4 Gb/6 Gb\n"
static char process_num[40];//5
static char uptime_ptr[40];
static char *msg_Ptr;


static char* checkmessage(int *arr,int *i){
    int checktemp;
    for (int j=*i;j<6;j++){
        if (arr[j]!=0){
            checktemp=j;
            break;
        }
        if (j==5){
            checktemp=6;//ot of index that checktemp means 6
        }

    }
    *i=checktemp+1; //plus 1 mean next time from that number to seek
    if (checktemp==0)
        return kernel;
    if (checktemp==1)
        return CPU_core;
    if (checktemp==2)
        return CPU_name;
    if (checktemp==3)
        return Mem;
    if (checktemp==4)
        return uptime_ptr;
    if (checktemp==5)
        return process_num;

    else 
        return "";
        
}
char bird0[]="                        ";
char bird1[]="      .-.               ";
char bird2[]="     (.. |              ";  
char bird3[]="     <>  |              ";   
char bird4[]="    / --- \\             ";     
char bird5[]="   ( |   | |            ";
char bird6[]=" |\\\\_)___/\\)/\\          ";
char bird7[]="<__)------(__/          "; 

static struct class *cls;

static struct file_operations chardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
    pr_info("this is  test\n");
    if (Major < 0) {
        pr_alert("Registering char device failed with %d\n", Major);
        return Major;
    }

    pr_info("I was assigned major number %d.\n", Major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);
    

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    device_destroy(cls, MKDEV(Major, 0));
    class_destroy(cls);

    /*
     * Unregister the device
     */
    unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
    

    if (Device_Open)
        return -EBUSY;
    msg_Ptr=final_output;
	pr_info("final_output%s\n",final_output);
    Device_Open++;
    
	
	
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;          /* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
                           char __user *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_read = 0;
	
    /*
     * If we're at the end of the message,
     * return 0 signifying end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /*
     * Actually put the data into the buffer
     */
    while (length && *msg_Ptr) {

        /*
         * The buffer is in the user data segment, not the kernel
         * segment so "*" assignment won't work.  We have to use
         * put_user which copies data from the kernel data segment to
         * the user data segment.
         */
        put_user(*(msg_Ptr++), buffer++);

        length--;
        bytes_read++;
    }
	
    /*
     * Most read functions return the number of bytes put into the buffer
     */
    return bytes_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *filp,
                            const char __user *buff,
                            size_t len,
                            loff_t * off)
{

    
    char *string[10];
    struct  sysinfo si;
    unsigned long uptime ;
    unsigned long free_ram,total_ram;
    unsigned int cpu = 0;
    struct cpuinfo_x86 *c;
    int mask_info;
    string[0]=bird2;
    string[1]=bird3;
    string[2]=bird4;
    string[3]=bird5;
    string[4]=bird6;
    string[5]=bird7;
    c = &cpu_data(cpu);//about cpu name
    si_meminfo(&si);
    free_ram = si.freeram * PAGE_SIZE;
    free_ram=free_ram/1000000;
    total_ram=si.totalram*PAGE_SIZE;
    total_ram=total_ram/1000000;
    uptime= jiffies_to_msecs(jiffies) / 1000;//for boot time
    get_thread_count();
    
    sprintf(CPU_name,"%s\n", c->x86_model_id);
    sprintf(process_num,"Procs:   %d\n", thread_count);
    thread_count=0; //init thread count ,otherwise thread count will increase 
    sprintf(uptime_ptr,"Uptime:  %lu min\n", uptime/60);
    sprintf(Mem,"Mem:     %lu MB/%lu MB\n",free_ram ,total_ram);
    sprintf(kernel,"Kernel:  %s\n",utsname()->release);
    
    sprintf(CPU_core,"CPUS     %d/%d\n", num_online_cpus(),num_active_cpus());


    sprintf(hostname,"%s\n",utsname()->nodename);
    char dash[strlen(hostname)];
    memset(dash,'-',(strlen(hostname)));
    dash[strlen(hostname)]='\0';
    sprintf(final_output,"%s%s%s%s\n",bird0,hostname,bird1,dash);
    

    

    if (copy_from_user(&mask_info, buff, len)) {
        pr_alert("Failed to copy data from user");
        return 0;
    }
	pr_info("mask_info:%d\n",mask_info);    
	int count=0;
    int flag=1;
    int temp;
    int bitarr[6];//store the information that which index will have return  cpu name or process or some thing ex: 1 0 0 1 0 0
    for (int i =0;i<6;i++){
        flag=1<<count;
        temp=flag&mask_info; //check if this index is  1
        if(temp!=0){
            bitarr[i]=1;
        }
        else {
            bitarr[i]=0;
        }
        
        count++;
    }
   
    int checkvalue=0;
    int *check;
    check=&checkvalue;
    for(int i=0;i<6;i++){
        sprintf(final_output+strlen(final_output),string[i]);
        sprintf(final_output+strlen(final_output),checkmessage(bitarr,check));//accroding to bitarr index return what we need information
        
        if (*check==7)
            sprintf(final_output+strlen(final_output),"\n");


    }
	
    
	msg_Ptr=final_output;
	
	return len;
}

MODULE_LICENSE("GPL");
