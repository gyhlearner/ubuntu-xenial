/*
 * Copyright (C) 2004 IBM Corporation
 * Authors:
 * Leendert van Doorn <leendert@watson.ibm.com>
 * Dave Safford <safford@watson.ibm.com>
 * Reiner Sailer <sailer@watson.ibm.com>
 * Kylene Hall <kjhall@us.ibm.com>
 *
 * Copyright (C) 2013 Obsidian Research Corp
 * Jason Gunthorpe <jgunthorpe@obsidianresearch.com>
 *
 * Device file system interface to the TPM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 */
#include <linux/slab.h>
<<<<<<< HEAD
#include <linux/uaccess.h>
#include "tpm.h"

struct file_priv {
	struct tpm_chip *chip;

	/* Data passed to and from the tpm via the read/write calls */
	size_t data_pending;
	struct mutex buffer_mutex;

	struct timer_list user_read_timer;      /* user needs to claim result */
	struct work_struct work;

	u8 data_buffer[TPM_BUFSIZE];
};

static void user_reader_timeout(unsigned long ptr)
{
	struct file_priv *priv = (struct file_priv *)ptr;

	schedule_work(&priv->work);
}

static void timeout_work(struct work_struct *work)
{
	struct file_priv *priv = container_of(work, struct file_priv, work);

	mutex_lock(&priv->buffer_mutex);
	priv->data_pending = 0;
	memset(priv->data_buffer, 0, sizeof(priv->data_buffer));
	mutex_unlock(&priv->buffer_mutex);
}
=======
#include "tpm-dev.h"
>>>>>>> temp

static int tpm_open(struct inode *inode, struct file *file)
{
	struct tpm_chip *chip;
	struct file_priv *priv;

	chip = container_of(inode->i_cdev, struct tpm_chip, cdev);

	/* It's assured that the chip will be opened just once,
	 * by the check of is_open variable, which is protected
	 * by driver_lock. */
	if (test_and_set_bit(0, &chip->is_open)) {
		dev_dbg(&chip->dev, "Another process owns this TPM\n");
		return -EBUSY;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL)
		goto out;

<<<<<<< HEAD
	priv->chip = chip;
	mutex_init(&priv->buffer_mutex);
	setup_timer(&priv->user_read_timer, user_reader_timeout,
			(unsigned long)priv);
	INIT_WORK(&priv->work, timeout_work);

	file->private_data = priv;
	return 0;
}

static ssize_t tpm_read(struct file *file, char __user *buf,
			size_t size, loff_t *off)
{
	struct file_priv *priv = file->private_data;
	ssize_t ret_size = 0;
	int rc;

	del_singleshot_timer_sync(&priv->user_read_timer);
	flush_work(&priv->work);
	mutex_lock(&priv->buffer_mutex);

	if (priv->data_pending) {
		ret_size = min_t(ssize_t, size, priv->data_pending);
		rc = copy_to_user(buf, priv->data_buffer, ret_size);
		memset(priv->data_buffer, 0, priv->data_pending);
		if (rc)
			ret_size = -EFAULT;

		priv->data_pending = 0;
	}

	mutex_unlock(&priv->buffer_mutex);
	return ret_size;
=======
	tpm_common_open(file, chip, priv);

	return 0;

 out:
	clear_bit(0, &chip->is_open);
	return -ENOMEM;
>>>>>>> temp
}

static ssize_t tpm_write(struct file *file, const char __user *buf,
			 size_t size, loff_t *off)
{
<<<<<<< HEAD
	struct file_priv *priv = file->private_data;
	size_t in_size = size;
	ssize_t out_size;

	if (in_size > TPM_BUFSIZE)
		return -E2BIG;

	mutex_lock(&priv->buffer_mutex);

	/* Cannot perform a write until the read has cleared either via
	 * tpm_read or a user_read_timer timeout. This also prevents split
	 * buffered writes from blocking here.
	 */
	if (priv->data_pending != 0) {
		mutex_unlock(&priv->buffer_mutex);
		return -EBUSY;
	}

	if (copy_from_user
	    (priv->data_buffer, (void __user *) buf, in_size)) {
		mutex_unlock(&priv->buffer_mutex);
		return -EFAULT;
	}

	/* atomic tpm command send and result receive. We only hold the ops
	 * lock during this period so that the tpm can be unregistered even if
	 * the char dev is held open.
	 */
	if (tpm_try_get_ops(priv->chip)) {
		mutex_unlock(&priv->buffer_mutex);
		return -EPIPE;
	}
	out_size = tpm_transmit(priv->chip, priv->data_buffer,
				sizeof(priv->data_buffer), 0);

	tpm_put_ops(priv->chip);
	if (out_size < 0) {
		mutex_unlock(&priv->buffer_mutex);
		return out_size;
	}

	priv->data_pending = out_size;
	mutex_unlock(&priv->buffer_mutex);

	/* Set a timeout by which the reader must come claim the result */
	mod_timer(&priv->user_read_timer, jiffies + (60 * HZ));

	return in_size;
=======
	return tpm_common_write(file, buf, size, off, NULL);
>>>>>>> temp
}

/*
 * Called on file close
 */
static int tpm_release(struct inode *inode, struct file *file)
{
	struct file_priv *priv = file->private_data;

<<<<<<< HEAD
	del_singleshot_timer_sync(&priv->user_read_timer);
	flush_work(&priv->work);
	file->private_data = NULL;
	priv->data_pending = 0;
=======
	tpm_common_release(file, priv);
>>>>>>> temp
	clear_bit(0, &priv->chip->is_open);
	kfree(priv);

	return 0;
}

const struct file_operations tpm_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.open = tpm_open,
	.read = tpm_common_read,
	.write = tpm_write,
	.release = tpm_release,
};
