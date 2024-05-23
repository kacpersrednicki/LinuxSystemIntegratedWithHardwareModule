#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>

MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kacper Srednicki");
MODULE_DESCRIPTION("Simple kernel module for SYKOM project");
MODULE_VERSION("0.01");

#define SYKT_GPIO_ADDR_SPACE (0x00100000)
#define SYKT_GPIO_SIZE (0x8000)
#define SYKT_EXIT (0x3333)
#define SYKT_EXIT_CODE (0x7F)

#define A_REG (SYKT_GPIO_ADDR_SPACE + 0x238)
#define S_REG (SYKT_GPIO_ADDR_SPACE + 0x250)
#define W_REG (SYKT_GPIO_ADDR_SPACE + 0x248)

#define BUF_SIZE 6
#define MEMO_SIZE 4

#define MY_DIRECTORY ("proj4srekac")
#define A_FILENAME ("rejA")
#define S_FILENAME ("rejS")
#define W_FILENAME ("rejW")

void __iomem *baseptr;
void __iomem *baseptrA;
void __iomem *baseptrS;
void __iomem *baseptrW;

static struct proc_dir_entry *directory;
static struct proc_dir_entry *file_A;
static struct proc_dir_entry *file_S;
static struct proc_dir_entry *file_W;

static ssize_t read_A(struct file *file, char *ubuf, size_t count, loff_t *offs) {
        int not_copied, to_read, length;
        char buf[BUF_SIZE];

        to_read = readl(baseptrA);

        if(*offs > 0) {
                return 0;
        }

        snprintf(buf, BUF_SIZE, "%o\n", to_read);
        length = strlen(buf);
        not_copied = copy_to_user(ubuf, buf, length);

        if(not_copied != 0) {
                printk(KERN_ERR "Error copying to user.\n");
                return -EFAULT;
        }

        *offs = *offs + count;
        return count;
}

static ssize_t read_S(struct file *file, char *ubuf, size_t count, loff_t *offs) {
        int not_copied, to_read, length;
        char buf[BUF_SIZE];

        to_read = readl(baseptrS);

        if(*offs > 0) {
                return 0;
        }

        snprintf(buf, BUF_SIZE, "%o\n", to_read);
        length = strlen(buf);
        not_copied = copy_to_user(ubuf, buf, length);

        if(not_copied != 0) {
                printk(KERN_ERR "Error copying to user.\n");
                return -EFAULT;
        }

        *offs = *offs + count;
        return count;
}

static ssize_t read_W(struct file *file, char *ubuf, size_t count, loff_t *offs) {
        int not_copied, to_read, length;
        char buf[BUF_SIZE];

        to_read = readl(baseptrW);

        if(*offs > 0) {
                return 0;
        }

        snprintf(buf, BUF_SIZE, "%o\n", to_read);
        length = strlen(buf);
        not_copied = copy_to_user(ubuf, buf, length);

        if(not_copied != 0) {
                printk(KERN_ERR "Error copying to user.\n");
                return -EFAULT;
        }

        *offs = *offs + count;
        return count;
}

static ssize_t write_A(struct file *file, const char *ubuf, size_t count, loff_t *offs) {
        int to_write;
        char buf[BUF_SIZE] = {0};

        if(count > sizeof(buf)) {
                printk(KERN_ERR "Too long input.\n");
                return -EINVAL;
        }

        if((copy_from_user(buf, ubuf, count)) != 0) {
                printk(KERN_ERR "Error copying from user.\n");
                return -EFAULT;
        }

	// printk(KERN_INFO "Received: %s\n", buf);

        for (int i = 0; i < count - 1; i++) {
                if (buf[i] < '0' || buf[i] > '7') {
                printk(KERN_ERR "Wrong input, not octal.\n");
                return -EINVAL;
                }
        }

        if(sscanf(buf, "%o", &to_write) != 1) {
                printk(KERN_ERR "Wrong input format.\n");
                return -EINVAL;
        }

        buf[sizeof(buf) - 1] = '\0';
        int max_octal = 01750;

        if(to_write <= 0 || to_write > max_octal){
                printk(KERN_ERR "Octal value out of range.\n");
                return -EINVAL;
        }

        writel(to_write, baseptrA);
        return count;
}

static struct file_operations proc_fops_A = {
        .read = read_A,
        .write = write_A
};

static struct file_operations proc_fops_S = {
        .read = read_S
};

static struct file_operations proc_fops_W = {
        .read = read_W
};

int my_init_module(void) {
        printk(KERN_INFO "Init my module.\n");
        baseptr = ioremap(SYKT_GPIO_ADDR_SPACE, SYKT_GPIO_SIZE);
        baseptrA = ioremap(A_REG, MEMO_SIZE);
        baseptrS = ioremap(S_REG, MEMO_SIZE);
        baseptrW = ioremap(W_REG, MEMO_SIZE);

        if(!baseptr || !baseptrA || !baseptrS || !baseptrW) {
                printk(KERN_ERR "Error mapping GPIO memory.\n");
                return -ENOMEM;
        }

        directory = proc_mkdir(MY_DIRECTORY, NULL);
        file_A = proc_create(A_FILENAME, 0666, directory, &proc_fops_A);
        file_S = proc_create(S_FILENAME, 0444, directory, &proc_fops_S);
        file_W = proc_create(W_FILENAME, 0444, directory, &proc_fops_W);

        if(!directory) {
                printk(KERN_ERR "Error creating proc directory.\n");
                iounmap(baseptr);
                iounmap(baseptrA);
                iounmap(baseptrS);
                iounmap(baseptrW);
                return -ENOMEM;
        }

        if(!file_A || !file_S || !file_W) {
                printk(KERN_ERR "Error creating proc entries.\n");
                remove_proc_entry(A_FILENAME, directory);
                remove_proc_entry(S_FILENAME, directory);
                remove_proc_entry(W_FILENAME, directory);
                remove_proc_entry(MY_DIRECTORY, NULL);
                iounmap(baseptr);
                iounmap(baseptrA);
                iounmap(baseptrS);
                iounmap(baseptrW);
                return -ENOMEM;
        }

        return 0;
}


void my_cleanup_module(void) {
        printk(KERN_INFO "Cleanup my module.\n");
        writel(SYKT_EXIT | ((SYKT_EXIT_CODE)<<16), baseptr);

        remove_proc_entry(A_FILENAME, directory);
        remove_proc_entry(S_FILENAME, directory);
        remove_proc_entry(W_FILENAME, directory);
        remove_proc_entry(MY_DIRECTORY, NULL);

        iounmap(baseptr);
}

module_init(my_init_module)
module_exit(my_cleanup_module)

