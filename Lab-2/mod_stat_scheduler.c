#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/workqueue.h>

unsigned int delay_ms = 2000;
// default delay if delay is not defined on module loadup

void kmod_work_handler(struct work_struct *w);
struct workqueue_struct *wq = 0;
DECLARE_DELAYED_WORK(kmod_work, kmod_work_handler);
// setup workqueue

module_param(delay_ms, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
// module parameter for delay
MODULE_PARM_DESC(delay_ms, "Delay between statistics output");
// parameter description

void kmod_work_handler(struct work_struct *w) {
    struct latency_record *this_record = current->latency_record;
    // get latency record of current task

    int recLength = this_record->count;
    printk(KERN_INFO "record count: %u", this_record->count);
    for (int i = 0; i < recLength; i++) {
        // print count, time, max and element 0 of backtrace for each record
        printk(KERN_INFO
               "record nr. %d,\t time: %lu,\t\t\t max: %lu,\t\t\t backtrace[0]: %pX \n",
               i + 1, this_record[i].time, this_record[i].max,
               (void *)this_record[i].backtrace[0]);
    }
    printk(KERN_INFO "\n");
    queue_delayed_work(wq, &kmod_work, msecs_to_jiffies(delay_ms));
    // queue another statistics output in x ms defines in delay_ms
}

int __init init_module(void) {
    pr_info("start mod-stat\n");
    pr_info("delay = %u\n", delay_ms);
    // startup info when module is loaded

    wq = create_singlethread_workqueue("modworkqueue");
    // create a new workqueue
    queue_delayed_work(wq, &kmod_work, msecs_to_jiffies(delay_ms));
    // call work after defined delay

    return 0;
}

void __exit cleanup_module(void) {
    cancel_delayed_work_sync(&kmod_work);
    // cancel ongoing work, if not called the kernel could crash on
    // destroy_workqueue()
    destroy_workqueue(wq);
    // destroy the workqueue
    pr_info("end mod-stat\n");
    // info on module unload
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim and Tom");
MODULE_DESCRIPTION("BS-Lab-2 kernel module");
// license an description