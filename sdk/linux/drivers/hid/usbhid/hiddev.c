/*
 *  Copyright (c) 2001 Paul Stewart
 *  Copyright (c) 2001 Vojtech Pavlik
 *
 *  HID char devices, giving access to raw HID device events.
 *
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to Paul Stewart <stewart@wetlogic.net>
 */

#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/input.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/hiddev.h>
#include <linux/compat.h>
#include "usbhid.h"

#ifdef CONFIG_USB_DYNAMIC_MINORS
#define HIDDEV_MINOR_BASE	0
#define HIDDEV_MINORS		256
#else
#define HIDDEV_MINOR_BASE	96
#define HIDDEV_MINORS		16
#endif
#define HIDDEV_BUFFER_SIZE	4096 // 64

struct hiddev {
	int exist;
	int open;
	wait_queue_head_t wait;
	struct hid_device *hid;
	struct list_head list;
	spinlock_t list_lock;
};

struct hiddev_list {
	struct hiddev_usage_ref buffer[HIDDEV_BUFFER_SIZE];
	int head;
	int tail;
	unsigned flags;
	struct fasync_struct *fasync;
	struct hiddev *hiddev;
	struct list_head node;
};

static struct hiddev *hiddev_table[HIDDEV_MINORS];

/*
 * Find a report, given the report's type and ID.  The ID can be specified
 * indirectly by REPORT_ID_FIRST (which returns the first report of the given
 * type) or by (REPORT_ID_NEXT | old_id), which returns the next report of the
 * given type which follows old_id.
 */
static struct hid_report *
hiddev_lookup_report(struct hid_device *hid, struct hiddev_report_info *rinfo)
{
	unsigned int flags = rinfo->report_id & ~HID_REPORT_ID_MASK;
	unsigned int rid = rinfo->report_id & HID_REPORT_ID_MASK;
	struct hid_report_enum *report_enum;
	struct hid_report *report;
	struct list_head *list;

	if (rinfo->report_type < HID_REPORT_TYPE_MIN ||
	    rinfo->report_type > HID_REPORT_TYPE_MAX)
		return NULL;

	report_enum = hid->report_enum +
		(rinfo->report_type - HID_REPORT_TYPE_MIN);

	switch (flags) {
	case 0: /* Nothing to do -- report_id is already set correctly */
		break;

	case HID_REPORT_ID_FIRST:
		if (list_empty(&report_enum->report_list))
			return NULL;

		list = report_enum->report_list.next;
		report = list_entry(list, struct hid_report, list);
		rinfo->report_id = report->id;
		break;

	case HID_REPORT_ID_NEXT:
		report = report_enum->report_id_hash[rid];
		if (!report)
			return NULL;

		list = report->list.next;
		if (list == &report_enum->report_list)
			return NULL;

		report = list_entry(list, struct hid_report, list);
		rinfo->report_id = report->id;
		break;

	default:
		return NULL;
	}

	return report_enum->report_id_hash[rinfo->report_id];
}

/*
 * Perform an exhaustive search of the report table for a usage, given its
 * type and usage id.
 */
static struct hid_field *
hiddev_lookup_usage(struct hid_device *hid, struct hiddev_usage_ref *uref)
{
	int i, j;
	struct hid_report *report;
	struct hid_report_enum *report_enum;
	struct hid_field *field;

	if (uref->report_type < HID_REPORT_TYPE_MIN ||
	    uref->report_type > HID_REPORT_TYPE_MAX)
		return NULL;

	report_enum = hid->report_enum +
		(uref->report_type - HID_REPORT_TYPE_MIN);

	list_for_each_entry(report, &report_enum->report_list, list) {
		for (i = 0; i < report->maxfield; i++) {
			field = report->field[i];
			for (j = 0; j < field->maxusage; j++) {
				if (field->usage[j].hid == uref->usage_code) {
					uref->report_id = report->id;
					uref->field_index = i;
					uref->usage_index = j;
					return field;
				}
			}
		}
	}

	return NULL;
}

static void hiddev_send_event(struct hid_device *hid,
			      struct hiddev_usage_ref *uref)
{
	struct hiddev *hiddev = hid->hiddev;
	struct hiddev_list *list;
	unsigned long flags;

	spin_lock_irqsave(&hiddev->list_lock, flags);
	list_for_each_entry(list, &hiddev->list, node) {
		if (uref->field_index != HID_FIELD_INDEX_NONE ||
		    (list->flags & HIDDEV_FLAG_REPORT) != 0) {
			list->buffer[list->head] = *uref;
			list->head = (list->head + 1) &
				(HIDDEV_BUFFER_SIZE - 1);
			//printk("[%s %d]\n",__func__,__LINE__);
			kill_fasync(&list->fasync, SIGIO, POLL_IN);
		}
	}
	spin_unlock_irqrestore(&hiddev->list_lock, flags);

	wake_up_interruptible(&hiddev->wait);
}

/*
 * This is where hid.c calls into hiddev to pass an event that occurred over
 * the interrupt pipe
 */
void hiddev_hid_event(struct hid_device *hid, struct hid_field *field,
		      struct hid_usage *usage, __s32 value)
{
	unsigned type = field->report_type;
	struct hiddev_usage_ref uref;

	uref.report_type =
	  (type == HID_INPUT_REPORT) ? HID_REPORT_TYPE_INPUT :
	  ((type == HID_OUTPUT_REPORT) ? HID_REPORT_TYPE_OUTPUT :
	   ((type == HID_FEATURE_REPORT) ? HID_REPORT_TYPE_FEATURE : 0));
	uref.report_id = field->report->id;
	uref.field_index = field->index;
	uref.usage_index = (usage - field->usage);
	uref.usage_code = usage->hid;
	uref.value = value; 
	hiddev_send_event(hid, &uref);
}
EXPORT_SYMBOL_GPL(hiddev_hid_event);

void hiddev_report_event(struct hid_device *hid, struct hid_report *report)
{
	unsigned type = report->type;
	struct hiddev_usage_ref uref;

	memset(&uref, 0, sizeof(uref));
	uref.report_type =
	  (type == HID_INPUT_REPORT) ? HID_REPORT_TYPE_INPUT :
	  ((type == HID_OUTPUT_REPORT) ? HID_REPORT_TYPE_OUTPUT :
	   ((type == HID_FEATURE_REPORT) ? HID_REPORT_TYPE_FEATURE : 0));
	uref.report_id = report->id;
	uref.field_index = HID_FIELD_INDEX_NONE;  
	hiddev_send_event(hid, &uref);
}

/*
 * fasync file op
 */
static int hiddev_fasync(int fd, struct file *file, int on)
{
	int retval = 0;
	//printk("[%s %d bluse]on :%d\n",__func__,__LINE__,on);
	struct hiddev_list *list = file->private_data;

	if ( list != NULL )
		retval = fasync_helper(fd, file, on, &list->fasync);

	//printk("[%s %d bluse]on :%d\n",__func__,__LINE__,on);

	return retval < 0 ? retval : 0;
}


/*
 * release file op
 */
static int hiddev_release(struct inode * inode, struct file * file)
{
	struct hiddev_list *list = file->private_data;
	unsigned long flags;

	hiddev_fasync(-1, file, 0);

	spin_lock_irqsave(&list->hiddev->list_lock, flags);
	list_del(&list->node);
	spin_unlock_irqrestore(&list->hiddev->list_lock, flags);

	if (!--list->hiddev->open) {
		if (list->hiddev->exist)
			usbhid_close(list->hiddev->hid);
		else
			kfree(list->hiddev);
	}

	kfree(list);

	return 0;
}

/*
 * open file op
 */
static int hiddev_open(struct inode *inode, struct file *file)
{
	struct hiddev_list *list;
	unsigned long flags;

	int i = iminor(inode) - HIDDEV_MINOR_BASE;

	if (i >= HIDDEV_MINORS || !hiddev_table[i])
		return -ENODEV;

	if (!(list = kzalloc(sizeof(struct hiddev_list), GFP_KERNEL)))
		return -ENOMEM;

	list->hiddev = hiddev_table[i];

	spin_lock_irqsave(&list->hiddev->list_lock, flags);
	list_add_tail(&list->node, &hiddev_table[i]->list);
	spin_unlock_irqrestore(&list->hiddev->list_lock, flags);

	file->private_data = list;
	if (!list->hiddev->open++)
		if (list->hiddev->exist)
			usbhid_open(hiddev_table[i]->hid);

	return 0;
}

/*
 * "write" file op
 */
static ssize_t hiddev_write(struct file * file, const char __user * buffer, size_t count, loff_t *ppos)
{
	return -EINVAL;
}

/*
 * "read" file op
 */
static ssize_t hiddev_read(struct file * file, char __user * buffer, size_t count, loff_t *ppos)
{
	DECLARE_WAITQUEUE(wait, current);
	struct hiddev_list *list = file->private_data;
	int event_size;
	int retval = 0;

	event_size = ((list->flags & HIDDEV_FLAG_UREF) != 0) ?
		sizeof(struct hiddev_usage_ref) : sizeof(struct hiddev_event);

	if (count < event_size)
		return 0;
//printk("--%02x:%02x:%02x--\n",HIDIOCGREPORTINFO,HIDIOCGFIELDINFO,HIDIOCGUCODE,HIDIOCGUSAGE);
	while (retval == 0) {
		if (list->head == list->tail) {
			add_wait_queue(&list->hiddev->wait, &wait);
			set_current_state(TASK_INTERRUPTIBLE);

			while (list->head == list->tail) {
				if (file->f_flags & O_NONBLOCK) {
					retval = -EAGAIN;
					break;
				}
				if (signal_pending(current)) {
					retval = -ERESTARTSYS;
					break;
				}
				if (!list->hiddev->exist) {
					retval = -EIO;
					break;
				}

				schedule();
				set_current_state(TASK_INTERRUPTIBLE);
			}

			set_current_state(TASK_RUNNING);
			remove_wait_queue(&list->hiddev->wait, &wait);
		}

		if (retval)
			return retval;


		while (list->head != list->tail &&
		       retval + event_size <= count) {
			if ((list->flags & HIDDEV_FLAG_UREF) == 0) {
				if (list->buffer[list->tail].field_index !=
				    HID_FIELD_INDEX_NONE) {
					struct hiddev_event event;
					event.hid = list->buffer[list->tail].usage_code;
					event.value = list->buffer[list->tail].value;
					//printk("[%s %d bluse] usage_code:0x%x value:0x%x\n",__func__,__LINE__,event.hid,event.value);
					if (copy_to_user(buffer + retval, &event, sizeof(struct hiddev_event))) {
						printk("[%s %d bluse] fault\n",__func__,__LINE__);
						return -EFAULT;
					}
					retval += sizeof(struct hiddev_event);
				}
			} else {
				if (list->buffer[list->tail].field_index != HID_FIELD_INDEX_NONE ||
				    (list->flags & HIDDEV_FLAG_REPORT) != 0) {
				    printk("%s ----\n",__func__);
					if (copy_to_user(buffer + retval, list->buffer + list->tail, sizeof(struct hiddev_usage_ref)))
						return -EFAULT;
					retval += sizeof(struct hiddev_usage_ref);
				}
			}
			list->tail = (list->tail + 1) & (HIDDEV_BUFFER_SIZE - 1);
		}

	}

	return retval;
}

/*
 * "poll" file op
 * No kernel lock - fine
 */
static unsigned int hiddev_poll(struct file *file, poll_table *wait)
{
	struct hiddev_list *list = file->private_data;
	poll_wait(file, &list->hiddev->wait, wait);
	if (list->head != list->tail){
#ifdef AM_HID_DEVICE 
		if(list->hiddev->hid->DataQueue->front == list->hiddev->hid->DataQueue->rear){
			//printk("head:%d tail:%d\n",list->head,list->tail);
			list->head=list->tail=0;//bluse change for usb hid 
		}
#else
		// Ziv: this is a serious bug !!!, don't reset queue when poll
		// 
		//list->head=list->tail=0;
#endif
		return POLLIN | POLLRDNORM;
	}
	if (!list->hiddev->exist)
		return POLLERR | POLLHUP;
	return 0;
}

/*
 * "ioctl" file op
 */
static noinline int hiddev_ioctl_usage(struct hiddev *hiddev, unsigned int cmd, void __user *user_arg)
{
	struct hid_device *hid = hiddev->hid;
	struct hiddev_report_info rinfo;
	struct hiddev_usage_ref_multi *uref_multi = NULL;
	struct hiddev_usage_ref *uref;
	struct hid_report *report;
	struct hid_field *field;
	int i;

	uref_multi = kmalloc(sizeof(struct hiddev_usage_ref_multi), GFP_KERNEL);
	if (!uref_multi)
		return -ENOMEM;
	lock_kernel();
	uref = &uref_multi->uref;
	if (cmd == HIDIOCGUSAGES || cmd == HIDIOCSUSAGES) {
		if (copy_from_user(uref_multi, user_arg,
				   sizeof(*uref_multi)))
			goto fault;
	} else {
		if (copy_from_user(uref, user_arg, sizeof(*uref)))
			goto fault;
	}

	switch (cmd) {
	case HIDIOCGUCODE:
		rinfo.report_type = uref->report_type;
		rinfo.report_id = uref->report_id;
		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			goto inval;

		if (uref->field_index >= report->maxfield)
			goto inval;

		field = report->field[uref->field_index];
		if (uref->usage_index >= field->maxusage)
			goto inval;

		uref->usage_code = field->usage[uref->usage_index].hid;

		if (copy_to_user(user_arg, uref, sizeof(*uref)))
			goto fault;

		kfree(uref_multi);
		return 0;

	default:
		if (cmd != HIDIOCGUSAGE &&
		    cmd != HIDIOCGUSAGES &&
		    uref->report_type == HID_REPORT_TYPE_INPUT)
			goto inval;

		if (uref->report_id == HID_REPORT_ID_UNKNOWN) {
			field = hiddev_lookup_usage(hid, uref);
			if (field == NULL)
				goto inval;
		} else {
			rinfo.report_type = uref->report_type;
			rinfo.report_id = uref->report_id;
			if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
				goto inval;

			if (uref->field_index >= report->maxfield)
				goto inval;

			field = report->field[uref->field_index];

			if (cmd == HIDIOCGCOLLECTIONINDEX) {
				if (uref->usage_index >= field->maxusage)
					goto inval;
			} else if (uref->usage_index >= field->report_count)
				goto inval;

			else if ((cmd == HIDIOCGUSAGES || cmd == HIDIOCSUSAGES) &&
				 (uref_multi->num_values > HID_MAX_MULTI_USAGES ||
				  uref->usage_index + uref_multi->num_values > field->report_count))
				goto inval;
			}

		switch (cmd) {
		case HIDIOCGUSAGE:
#ifdef AM_HID_DEVICE 
			uref->value=hid->DataQueue->HidData[hid->DataQueue->front][uref->usage_index];

			if(uref->usage_index==63){
				//printk("%s %d front:%d rear:%d\n",__func__,__LINE__,hid->DataQueue->front,hid->DataQueue->rear);
				hid->DataQueue->front=(hid->DataQueue->front+1)%16;
			}
#else
			uref->value = field->value[uref->usage_index];
#endif
			if (copy_to_user(user_arg, uref, sizeof(*uref)))
				goto fault;
			goto goodreturn;

		case HIDIOCSUSAGE:
			field->value[uref->usage_index] = uref->value;
			goto goodreturn;

		case HIDIOCGCOLLECTIONINDEX:
			i = field->usage[uref->usage_index].collection_index;
			unlock_kernel();
			kfree(uref_multi);
			return i;
		case HIDIOCGUSAGES:
			for (i = 0; i < uref_multi->num_values; i++)
				uref_multi->values[i] =
				    field->value[uref->usage_index + i];
			if (copy_to_user(user_arg, uref_multi,
					 sizeof(*uref_multi)))
				goto fault;
			goto goodreturn;
		case HIDIOCSUSAGES:
			for (i = 0; i < uref_multi->num_values; i++)
				field->value[uref->usage_index + i] =
				    uref_multi->values[i];
			goto goodreturn;
		}

goodreturn:
		unlock_kernel();
		kfree(uref_multi);
		return 0;
fault:
		unlock_kernel();
		kfree(uref_multi);
		return -EFAULT;
inval:
		unlock_kernel();
		kfree(uref_multi);
		return -EINVAL;
	}
}

static noinline int hiddev_ioctl_string(struct hiddev *hiddev, unsigned int cmd, void __user *user_arg)
{
	struct hid_device *hid = hiddev->hid;
	struct usb_device *dev = hid_to_usb_dev(hid);
	int idx, len;
	char *buf;

	if (get_user(idx, (int __user *)user_arg))
		return -EFAULT;

	if ((buf = kmalloc(HID_STRING_SIZE, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if ((len = usb_string(dev, idx, buf, HID_STRING_SIZE-1)) < 0) {
		kfree(buf);
		return -EINVAL;
	}

	if (copy_to_user(user_arg+sizeof(int), buf, len+1)) {
		kfree(buf);
		return -EFAULT;
	}

	kfree(buf);

	return len;
}

static long hiddev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hiddev_list *list = file->private_data;
	struct hiddev *hiddev = list->hiddev;
	struct hid_device *hid = hiddev->hid;
	struct usb_device *dev = hid_to_usb_dev(hid);
	struct hiddev_collection_info cinfo;
	struct hiddev_report_info rinfo;
	struct hiddev_field_info finfo;
	struct hiddev_devinfo dinfo;
	struct hid_report *report;
	struct hid_field *field;
	struct usbhid_device *usbhid = hid->driver_data;
	void __user *user_arg = (void __user *)arg;
	int i;
	/* Called without BKL by compat methods so no BKL taken */

	/* FIXME: Who or what stop this racing with a disconnect ?? */
	if (!hiddev->exist)
		return -EIO;

	switch (cmd) {

	case HIDIOCGVERSION:
		return put_user(HID_VERSION, (int __user *)arg);

	case HIDIOCAPPLICATION:
		if (arg < 0 || arg >= hid->maxapplication)
			return -EINVAL;

		for (i = 0; i < hid->maxcollection; i++)
			if (hid->collection[i].type ==
			    HID_COLLECTION_APPLICATION && arg-- == 0)
				break;

		if (i == hid->maxcollection)
			return -EINVAL;

		return hid->collection[i].usage;

	case HIDIOCGDEVINFO:
		dinfo.bustype = BUS_USB;
		dinfo.busnum = dev->bus->busnum;
		dinfo.devnum = dev->devnum;
		dinfo.ifnum = usbhid->ifnum;
		dinfo.vendor = le16_to_cpu(dev->descriptor.idVendor);
		dinfo.product = le16_to_cpu(dev->descriptor.idProduct);
		dinfo.version = le16_to_cpu(dev->descriptor.bcdDevice);
		dinfo.num_applications = hid->maxapplication;
		if (copy_to_user(user_arg, &dinfo, sizeof(dinfo)))
			return -EFAULT;

		return 0;

	case HIDIOCGFLAG:
		if (put_user(list->flags, (int __user *)arg))
			return -EFAULT;

		return 0;

	case HIDIOCSFLAG:
		{
			int newflags;
			if (get_user(newflags, (int __user *)arg))
				return -EFAULT;

			if ((newflags & ~HIDDEV_FLAGS) != 0 ||
			    ((newflags & HIDDEV_FLAG_REPORT) != 0 &&
			     (newflags & HIDDEV_FLAG_UREF) == 0))
				return -EINVAL;

			list->flags = newflags;

			return 0;
		}

	case HIDIOCGSTRING:
		return hiddev_ioctl_string(hiddev, cmd, user_arg);

	case HIDIOCINITREPORT:
		usbhid_init_reports(hid);

		return 0;

	case HIDIOCGREPORT:
		if (copy_from_user(&rinfo, user_arg, sizeof(rinfo)))
			return -EFAULT;

		if (rinfo.report_type == HID_REPORT_TYPE_OUTPUT)
			return -EINVAL;

		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			return -EINVAL;

		usbhid_submit_report(hid, report, USB_DIR_IN);
		usbhid_wait_io(hid);

		return 0;

	case HIDIOCSREPORT:
		if (copy_from_user(&rinfo, user_arg, sizeof(rinfo)))
			return -EFAULT;

		if (rinfo.report_type == HID_REPORT_TYPE_INPUT)
			return -EINVAL;

		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			return -EINVAL;

		usbhid_submit_report(hid, report, USB_DIR_OUT);
		usbhid_wait_io(hid);

		return 0;

	case HIDIOCGREPORTINFO:
		if (copy_from_user(&rinfo, user_arg, sizeof(rinfo)))
			return -EFAULT;

		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			return -EINVAL;

		rinfo.num_fields = report->maxfield;

		if (copy_to_user(user_arg, &rinfo, sizeof(rinfo)))
			return -EFAULT;

		return 0;

	case HIDIOCGFIELDINFO:
		if (copy_from_user(&finfo, user_arg, sizeof(finfo)))
			return -EFAULT;
		rinfo.report_type = finfo.report_type;
		rinfo.report_id = finfo.report_id;
		if ((report = hiddev_lookup_report(hid, &rinfo)) == NULL)
			return -EINVAL;

		if (finfo.field_index >= report->maxfield)
			return -EINVAL;
		
		field = report->field[finfo.field_index];
#if 0  //bluse
		printk("[%s %d] report_count:%d value:\n",__func__,__LINE__,field->report_count);
		int n;
		for (n = 0; n < field->report_count; n++) {
			printk(" %02x", field->value[n]);
		}
		printk("\n");
#endif
		memset(&finfo, 0, sizeof(finfo));
		finfo.report_type = rinfo.report_type;
		finfo.report_id = rinfo.report_id;
		finfo.field_index = field->report_count - 1;
		finfo.maxusage = field->maxusage;
		finfo.flags = field->flags;
		finfo.physical = field->physical;
		finfo.logical = field->logical;
		finfo.application = field->application;
		finfo.logical_minimum = field->logical_minimum;
		finfo.logical_maximum = field->logical_maximum;
		finfo.physical_minimum = field->physical_minimum;
		finfo.physical_maximum = field->physical_maximum;
		finfo.unit_exponent = field->unit_exponent;
		finfo.unit = field->unit;

		if (copy_to_user(user_arg, &finfo, sizeof(finfo)))
			return -EFAULT;

		return 0;

	case HIDIOCGUCODE:
		/* fall through */
	case HIDIOCGUSAGE:
	case HIDIOCSUSAGE:
	case HIDIOCGUSAGES:
	case HIDIOCSUSAGES:
	case HIDIOCGCOLLECTIONINDEX:
		return hiddev_ioctl_usage(hiddev, cmd, user_arg);
	case HIDIOCGCOLLECTIONINFO:
		if (copy_from_user(&cinfo, user_arg, sizeof(cinfo)))
			return -EFAULT;

		if (cinfo.index >= hid->maxcollection)
			return -EINVAL;

		cinfo.type = hid->collection[cinfo.index].type;
		cinfo.usage = hid->collection[cinfo.index].usage;
		cinfo.level = hid->collection[cinfo.index].level;

		if (copy_to_user(user_arg, &cinfo, sizeof(cinfo)))
			return -EFAULT;
		return 0;

	default:

		if (_IOC_TYPE(cmd) != 'H' || _IOC_DIR(cmd) != _IOC_READ)
			return -EINVAL;

		if (_IOC_NR(cmd) == _IOC_NR(HIDIOCGNAME(0))) {
			int len;
			if (!hid->name)
				return 0;
			len = strlen(hid->name) + 1;
			if (len > _IOC_SIZE(cmd))
				 len = _IOC_SIZE(cmd);
			return copy_to_user(user_arg, hid->name, len) ?
				-EFAULT : len;
		}

		if (_IOC_NR(cmd) == _IOC_NR(HIDIOCGPHYS(0))) {
			int len;
			if (!hid->phys)
				return 0;
			len = strlen(hid->phys) + 1;
			if (len > _IOC_SIZE(cmd))
				len = _IOC_SIZE(cmd);
			return copy_to_user(user_arg, hid->phys, len) ?
				-EFAULT : len;
		}
	}
	return -EINVAL;
}

#ifdef CONFIG_COMPAT
static long hiddev_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return hiddev_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static const struct file_operations hiddev_fops = {
	.owner =	THIS_MODULE,
	.read =		hiddev_read,
	.write =	hiddev_write,
	.poll =		hiddev_poll,
	.open =		hiddev_open,
	.release =	hiddev_release,
	.unlocked_ioctl =	hiddev_ioctl,
	.fasync =	hiddev_fasync,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= hiddev_compat_ioctl,
#endif
};

static struct usb_class_driver hiddev_class = {
	.name =		"hiddev%d",
	.fops =		&hiddev_fops,
	.minor_base =	HIDDEV_MINOR_BASE,
};

/*
 * This is where hid.c calls us to connect a hid device to the hiddev driver
 */
int hiddev_connect(struct hid_device *hid)
{
	struct hiddev *hiddev;
	struct usbhid_device *usbhid = hid->driver_data;
	int i;
	int retval;

	for (i = 0; i < hid->maxcollection; i++)
		if (hid->collection[i].type ==
		    HID_COLLECTION_APPLICATION &&
		    !IS_INPUT_APPLICATION(hid->collection[i].usage))
			break;

	if (i == hid->maxcollection && (hid->quirks & HID_QUIRK_HIDDEV) == 0)
		return -1;

	if (!(hiddev = kzalloc(sizeof(struct hiddev), GFP_KERNEL)))
		return -1;

	retval = usb_register_dev(usbhid->intf, &hiddev_class);
	if (retval) {
		err_hid("Not able to get a minor for this device.");
		kfree(hiddev);
		return -1;
	}

	init_waitqueue_head(&hiddev->wait);
	INIT_LIST_HEAD(&hiddev->list);
	spin_lock_init(&hiddev->list_lock);
	hiddev->hid = hid;
	hiddev->exist = 1;

	hid->minor = usbhid->intf->minor;
	hid->hiddev = hiddev;

	hiddev_table[usbhid->intf->minor - HIDDEV_MINOR_BASE] = hiddev;

	return 0;
}

/*
 * This is where hid.c calls us to disconnect a hiddev device from the
 * corresponding hid device (usually because the usb device has disconnected)
 */
static struct usb_class_driver hiddev_class;
void hiddev_disconnect(struct hid_device *hid)
{
	struct hiddev *hiddev = hid->hiddev;
	struct usbhid_device *usbhid = hid->driver_data;

	hiddev->exist = 0;

	hiddev_table[hiddev->hid->minor - HIDDEV_MINOR_BASE] = NULL;
	usb_deregister_dev(usbhid->intf, &hiddev_class);

	if (hiddev->open) {
		usbhid_close(hiddev->hid);
		wake_up_interruptible(&hiddev->wait);
	} else {
		kfree(hiddev);
	}
}

/* Currently this driver is a USB driver.  It's not a conventional one in
 * the sense that it doesn't probe at the USB level.  Instead it waits to
 * be connected by HID through the hiddev_connect / hiddev_disconnect
 * routines.  The reason to register as a USB device is to gain part of the
 * minor number space from the USB major.
 *
 * In theory, should the HID code be generalized to more than one physical
 * medium (say, IEEE 1384), this driver will probably need to register its
 * own major number, and in doing so, no longer need to register with USB.
 * At that point the probe routine and hiddev_driver struct below will no
 * longer be useful.
 */


/* We never attach in this manner, and rely on HID to connect us.  This
 * is why there is no disconnect routine defined in the usb_driver either.
 */
static int hiddev_usbd_probe(struct usb_interface *intf,
			     const struct usb_device_id *hiddev_info)
{
	return -ENODEV;
}


static /* const */ struct usb_driver hiddev_driver = {
	.name =		"hiddev",
	.probe =	hiddev_usbd_probe,
};

int __init hiddev_init(void)
{
	return usb_register(&hiddev_driver);
}

void hiddev_exit(void)
{
	usb_deregister(&hiddev_driver);
}
