/*
 *
 * Copyright (c) 2009, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 *   K. Y. Srinivasan <kys@microsoft.com>
 *
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/hyperv.h>
#include <linux/uio.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
<<<<<<< HEAD

#include "hyperv_vmbus.h"

void hv_begin_read(struct hv_ring_buffer_info *rbi)
{
	rbi->ring_buffer->interrupt_mask = 1;
	virt_mb();
}

u32 hv_end_read(struct hv_ring_buffer_info *rbi)
{

	rbi->ring_buffer->interrupt_mask = 0;
	virt_mb();

	/*
	 * Now check to see if the ring buffer is still empty.
	 * If it is not, we raced and we need to process new
	 * incoming messages.
	 */
	return hv_get_bytes_to_read(rbi);
}
=======
#include <linux/prefetch.h>

#include "hyperv_vmbus.h"

#define VMBUS_PKT_TRAILER	8
>>>>>>> temp

/*
 * When we write to the ring buffer, check if the host needs to
 * be signaled. Here is the details of this protocol:
 *
 *	1. The host guarantees that while it is draining the
 *	   ring buffer, it will set the interrupt_mask to
 *	   indicate it does not need to be interrupted when
 *	   new data is placed.
 *
 *	2. The host guarantees that it will completely drain
 *	   the ring buffer before exiting the read loop. Further,
 *	   once the ring buffer is empty, it will clear the
 *	   interrupt_mask and re-check to see if new data has
 *	   arrived.
 *
 * KYS: Oct. 30, 2016:
 * It looks like Windows hosts have logic to deal with DOS attacks that
 * can be triggered if it receives interrupts when it is not expecting
 * the interrupt. The host expects interrupts only when the ring
 * transitions from empty to non-empty (or full to non full on the guest
 * to host ring).
 * So, base the signaling decision solely on the ring state until the
 * host logic is fixed.
 */

<<<<<<< HEAD
static void hv_signal_on_write(u32 old_write, struct vmbus_channel *channel,
			       bool kick_q)
=======
static void hv_signal_on_write(u32 old_write, struct vmbus_channel *channel)
>>>>>>> temp
{
	struct hv_ring_buffer_info *rbi = &channel->outbound;

	virt_mb();
	if (READ_ONCE(rbi->ring_buffer->interrupt_mask))
		return;

	/* check interrupt_mask before read_index */
	virt_rmb();
	/*
	 * This is the only case we need to signal when the
	 * ring transitions from being empty to non-empty.
	 */
	if (old_write == READ_ONCE(rbi->ring_buffer->read_index))
		vmbus_setevent(channel);
<<<<<<< HEAD

	return;
=======
>>>>>>> temp
}

/* Get the next write location for the specified ring buffer. */
static inline u32
hv_get_next_write_location(struct hv_ring_buffer_info *ring_info)
{
	u32 next = ring_info->ring_buffer->write_index;

	return next;
}

/* Set the next write location for the specified ring buffer. */
static inline void
hv_set_next_write_location(struct hv_ring_buffer_info *ring_info,
		     u32 next_write_location)
{
	ring_info->ring_buffer->write_index = next_write_location;
}

<<<<<<< HEAD
/* Get the next read location for the specified ring buffer. */
static inline u32
hv_get_next_read_location(struct hv_ring_buffer_info *ring_info)
{
	u32 next = ring_info->ring_buffer->read_index;

	return next;
}

/*
 * Get the next read location + offset for the specified ring buffer.
 * This allows the caller to skip.
 */
static inline u32
hv_get_next_readlocation_withoffset(struct hv_ring_buffer_info *ring_info,
				 u32 offset)
{
	u32 next = ring_info->ring_buffer->read_index;

	next += offset;
	next %= ring_info->ring_datasize;

	return next;
}

=======
>>>>>>> temp
/* Set the next read location for the specified ring buffer. */
static inline void
hv_set_next_read_location(struct hv_ring_buffer_info *ring_info,
		    u32 next_read_location)
{
	ring_info->ring_buffer->read_index = next_read_location;
	ring_info->priv_read_index = next_read_location;
}

/* Get the size of the ring buffer. */
static inline u32
hv_get_ring_buffersize(const struct hv_ring_buffer_info *ring_info)
{
	return ring_info->ring_datasize;
}

/* Get the read and write indices as u64 of the specified ring buffer. */
static inline u64
hv_get_ring_bufferindices(struct hv_ring_buffer_info *ring_info)
{
	return (u64)ring_info->ring_buffer->write_index << 32;
}

/*
<<<<<<< HEAD
 * Helper routine to copy to source from ring buffer.
 * Assume there is enough room. Handles wrap-around in src case only!!
 */
static u32 hv_copyfrom_ringbuffer(
	struct hv_ring_buffer_info	*ring_info,
	void				*dest,
	u32				destlen,
	u32				start_read_offset)
{
	void *ring_buffer = hv_get_ring_buffer(ring_info);
	u32 ring_buffer_size = hv_get_ring_buffersize(ring_info);

	memcpy(dest, ring_buffer + start_read_offset, destlen);

	start_read_offset += destlen;
	start_read_offset %= ring_buffer_size;

	return start_read_offset;
}


/*
=======
>>>>>>> temp
 * Helper routine to copy from source to ring buffer.
 * Assume there is enough room. Handles wrap-around in dest case only!!
 */
static u32 hv_copyto_ringbuffer(
	struct hv_ring_buffer_info	*ring_info,
	u32				start_write_offset,
	const void			*src,
	u32				srclen)
{
	void *ring_buffer = hv_get_ring_buffer(ring_info);
	u32 ring_buffer_size = hv_get_ring_buffersize(ring_info);

	memcpy(ring_buffer + start_write_offset, src, srclen);

	start_write_offset += srclen;
	if (start_write_offset >= ring_buffer_size)
		start_write_offset -= ring_buffer_size;

	return start_write_offset;
}

/* Get various debug metrics for the specified ring buffer. */
<<<<<<< HEAD
void hv_ringbuffer_get_debuginfo(struct hv_ring_buffer_info *ring_info,
			    struct hv_ring_buffer_debug_info *debug_info)
=======
void hv_ringbuffer_get_debuginfo(const struct hv_ring_buffer_info *ring_info,
				 struct hv_ring_buffer_debug_info *debug_info)
>>>>>>> temp
{
	u32 bytes_avail_towrite;
	u32 bytes_avail_toread;

	if (ring_info->ring_buffer) {
		hv_get_ringbuffer_availbytes(ring_info,
					&bytes_avail_toread,
					&bytes_avail_towrite);

		debug_info->bytes_avail_toread = bytes_avail_toread;
		debug_info->bytes_avail_towrite = bytes_avail_towrite;
		debug_info->current_read_index =
			ring_info->ring_buffer->read_index;
		debug_info->current_write_index =
			ring_info->ring_buffer->write_index;
		debug_info->current_interrupt_mask =
			ring_info->ring_buffer->interrupt_mask;
	}
}
EXPORT_SYMBOL_GPL(hv_ringbuffer_get_debuginfo);

/* Initialize the ring buffer. */
int hv_ringbuffer_init(struct hv_ring_buffer_info *ring_info,
		       struct page *pages, u32 page_cnt)
{
	int i;
	struct page **pages_wraparound;

	BUILD_BUG_ON((sizeof(struct hv_ring_buffer) != PAGE_SIZE));

	memset(ring_info, 0, sizeof(struct hv_ring_buffer_info));

	/*
	 * First page holds struct hv_ring_buffer, do wraparound mapping for
	 * the rest.
	 */
	pages_wraparound = kzalloc(sizeof(struct page *) * (page_cnt * 2 - 1),
				   GFP_KERNEL);
	if (!pages_wraparound)
		return -ENOMEM;

	pages_wraparound[0] = pages;
	for (i = 0; i < 2 * (page_cnt - 1); i++)
		pages_wraparound[i + 1] = &pages[i % (page_cnt - 1) + 1];

	ring_info->ring_buffer = (struct hv_ring_buffer *)
		vmap(pages_wraparound, page_cnt * 2 - 1, VM_MAP, PAGE_KERNEL);

	kfree(pages_wraparound);


	if (!ring_info->ring_buffer)
		return -ENOMEM;

	ring_info->ring_buffer->read_index =
		ring_info->ring_buffer->write_index = 0;

	/* Set the feature bit for enabling flow control. */
	ring_info->ring_buffer->feature_bits.value = 1;

	ring_info->ring_size = page_cnt << PAGE_SHIFT;
	ring_info->ring_datasize = ring_info->ring_size -
		sizeof(struct hv_ring_buffer);

	spin_lock_init(&ring_info->ring_lock);

	return 0;
}

/* Cleanup the ring buffer. */
void hv_ringbuffer_cleanup(struct hv_ring_buffer_info *ring_info)
{
	vunmap(ring_info->ring_buffer);
}

/* Write to the ring buffer. */
int hv_ringbuffer_write(struct vmbus_channel *channel,
<<<<<<< HEAD
		    struct kvec *kv_list, u32 kv_count, bool lock,
		    bool kick_q)
=======
			const struct kvec *kv_list, u32 kv_count)
>>>>>>> temp
{
	int i;
	u32 bytes_avail_towrite;
<<<<<<< HEAD
	u32 totalbytes_towrite = 0;

	u32 next_write_location;
	u32 old_write;
	u64 prev_indices = 0;
	unsigned long flags = 0;
	struct hv_ring_buffer_info *outring_info = &channel->outbound;
=======
	u32 totalbytes_towrite = sizeof(u64);
	u32 next_write_location;
	u32 old_write;
	u64 prev_indices;
	unsigned long flags;
	struct hv_ring_buffer_info *outring_info = &channel->outbound;

	if (channel->rescind)
		return -ENODEV;
>>>>>>> temp

	for (i = 0; i < kv_count; i++)
		totalbytes_towrite += kv_list[i].iov_len;

<<<<<<< HEAD
	totalbytes_towrite += sizeof(u64);

	if (lock)
		spin_lock_irqsave(&outring_info->ring_lock, flags);
=======
	spin_lock_irqsave(&outring_info->ring_lock, flags);
>>>>>>> temp

	bytes_avail_towrite = hv_get_bytes_to_write(outring_info);

	/*
	 * If there is only room for the packet, assume it is full.
	 * Otherwise, the next time around, we think the ring buffer
	 * is empty since the read index == write index.
	 */
	if (bytes_avail_towrite <= totalbytes_towrite) {
		if (lock)
			spin_unlock_irqrestore(&outring_info->ring_lock, flags);
		return -EAGAIN;
	}

	/* Write to the ring buffer */
	next_write_location = hv_get_next_write_location(outring_info);

	old_write = next_write_location;

	for (i = 0; i < kv_count; i++) {
		next_write_location = hv_copyto_ringbuffer(outring_info,
						     next_write_location,
						     kv_list[i].iov_base,
						     kv_list[i].iov_len);
	}

	/* Set previous packet start */
	prev_indices = hv_get_ring_bufferindices(outring_info);

	next_write_location = hv_copyto_ringbuffer(outring_info,
					     next_write_location,
					     &prev_indices,
					     sizeof(u64));

	/* Issue a full memory barrier before updating the write index */
	virt_mb();

	/* Now, update the write location */
	hv_set_next_write_location(outring_info, next_write_location);


<<<<<<< HEAD
	if (lock)
		spin_unlock_irqrestore(&outring_info->ring_lock, flags);
=======
	spin_unlock_irqrestore(&outring_info->ring_lock, flags);

	hv_signal_on_write(old_write, channel);

	if (channel->rescind)
		return -ENODEV;

	return 0;
}

int hv_ringbuffer_read(struct vmbus_channel *channel,
		       void *buffer, u32 buflen, u32 *buffer_actual_len,
		       u64 *requestid, bool raw)
{
	struct vmpacket_descriptor *desc;
	u32 packetlen, offset;

	if (unlikely(buflen == 0))
		return -EINVAL;

	*buffer_actual_len = 0;
	*requestid = 0;

	/* Make sure there is something to read */
	desc = hv_pkt_iter_first(channel);
	if (desc == NULL) {
		/*
		 * No error is set when there is even no header, drivers are
		 * supposed to analyze buffer_actual_len.
		 */
		return 0;
	}

	offset = raw ? 0 : (desc->offset8 << 3);
	packetlen = (desc->len8 << 3) - offset;
	*buffer_actual_len = packetlen;
	*requestid = desc->trans_id;

	if (unlikely(packetlen > buflen))
		return -ENOBUFS;

	/* since ring is double mapped, only one copy is necessary */
	memcpy(buffer, (const char *)desc + offset, packetlen);

	/* Advance ring index to next packet descriptor */
	__hv_pkt_iter_next(channel, desc);

	/* Notify host of update */
	hv_pkt_iter_close(channel);
>>>>>>> temp

	hv_signal_on_write(old_write, channel, kick_q);
	return 0;
}

<<<<<<< HEAD
int hv_ringbuffer_read(struct vmbus_channel *channel,
		       void *buffer, u32 buflen, u32 *buffer_actual_len,
		       u64 *requestid, bool raw)
{
	u32 bytes_avail_toread;
	u32 next_read_location = 0;
	u64 prev_indices = 0;
	struct vmpacket_descriptor desc;
	u32 offset;
	u32 packetlen;
	int ret = 0;
	struct hv_ring_buffer_info *inring_info = &channel->inbound;
=======
/*
 * Determine number of bytes available in ring buffer after
 * the current iterator (priv_read_index) location.
 *
 * This is similar to hv_get_bytes_to_read but with private
 * read index instead.
 */
static u32 hv_pkt_iter_avail(const struct hv_ring_buffer_info *rbi)
{
	u32 priv_read_loc = rbi->priv_read_index;
	u32 write_loc = READ_ONCE(rbi->ring_buffer->write_index);

	if (write_loc >= priv_read_loc)
		return write_loc - priv_read_loc;
	else
		return (rbi->ring_datasize - priv_read_loc) + write_loc;
}

/*
 * Get first vmbus packet from ring buffer after read_index
 *
 * If ring buffer is empty, returns NULL and no other action needed.
 */
struct vmpacket_descriptor *hv_pkt_iter_first(struct vmbus_channel *channel)
{
	struct hv_ring_buffer_info *rbi = &channel->inbound;
	struct vmpacket_descriptor *desc;

	if (hv_pkt_iter_avail(rbi) < sizeof(struct vmpacket_descriptor))
		return NULL;

	desc = hv_get_ring_buffer(rbi) + rbi->priv_read_index;
	if (desc)
		prefetch((char *)desc + (desc->len8 << 3));

	return desc;
}
EXPORT_SYMBOL_GPL(hv_pkt_iter_first);

/*
 * Get next vmbus packet from ring buffer.
 *
 * Advances the current location (priv_read_index) and checks for more
 * data. If the end of the ring buffer is reached, then return NULL.
 */
struct vmpacket_descriptor *
__hv_pkt_iter_next(struct vmbus_channel *channel,
		   const struct vmpacket_descriptor *desc)
{
	struct hv_ring_buffer_info *rbi = &channel->inbound;
	u32 packetlen = desc->len8 << 3;
	u32 dsize = rbi->ring_datasize;
>>>>>>> temp

	/* bump offset to next potential packet */
	rbi->priv_read_index += packetlen + VMBUS_PKT_TRAILER;
	if (rbi->priv_read_index >= dsize)
		rbi->priv_read_index -= dsize;

<<<<<<< HEAD

	*buffer_actual_len = 0;
	*requestid = 0;

	bytes_avail_toread = hv_get_bytes_to_read(inring_info);
	/* Make sure there is something to read */
	if (bytes_avail_toread < sizeof(desc)) {
		/*
		 * No error is set when there is even no header, drivers are
		 * supposed to analyze buffer_actual_len.
		 */
		return ret;
	}

	init_cached_read_index(channel);
	next_read_location = hv_get_next_read_location(inring_info);
	next_read_location = hv_copyfrom_ringbuffer(inring_info, &desc,
						    sizeof(desc),
						    next_read_location);

	offset = raw ? 0 : (desc.offset8 << 3);
	packetlen = (desc.len8 << 3) - offset;
	*buffer_actual_len = packetlen;
	*requestid = desc.trans_id;

	if (bytes_avail_toread < packetlen + offset)
		return -EAGAIN;

	if (packetlen > buflen)
		return -ENOBUFS;
=======
	/* more data? */
	return hv_pkt_iter_first(channel);
}
EXPORT_SYMBOL_GPL(__hv_pkt_iter_next);

/* How many bytes were read in this iterator cycle */
static u32 hv_pkt_iter_bytes_read(const struct hv_ring_buffer_info *rbi,
					u32 start_read_index)
{
	if (rbi->priv_read_index >= start_read_index)
		return rbi->priv_read_index - start_read_index;
	else
		return rbi->ring_datasize - start_read_index +
			rbi->priv_read_index;
}

/*
 * Update host ring buffer after iterating over packets.
 */
void hv_pkt_iter_close(struct vmbus_channel *channel)
{
	struct hv_ring_buffer_info *rbi = &channel->inbound;
	u32 curr_write_sz, pending_sz, bytes_read, start_read_index;

	/*
	 * Make sure all reads are done before we update the read index since
	 * the writer may start writing to the read area once the read index
	 * is updated.
	 */
	virt_rmb();
	start_read_index = rbi->ring_buffer->read_index;
	rbi->ring_buffer->read_index = rbi->priv_read_index;
>>>>>>> temp

	if (!rbi->ring_buffer->feature_bits.feat_pending_send_sz)
		return;

<<<<<<< HEAD
	next_read_location = hv_copyfrom_ringbuffer(inring_info,
						buffer,
						packetlen,
						next_read_location);
=======
	/*
	 * Issue a full memory barrier before making the signaling decision.
	 * Here is the reason for having this barrier:
	 * If the reading of the pend_sz (in this function)
	 * were to be reordered and read before we commit the new read
	 * index (in the calling function)  we could
	 * have a problem. If the host were to set the pending_sz after we
	 * have sampled pending_sz and go to sleep before we commit the
	 * read index, we could miss sending the interrupt. Issue a full
	 * memory barrier to address this.
	 */
	virt_mb();
>>>>>>> temp

	pending_sz = READ_ONCE(rbi->ring_buffer->pending_send_sz);
	if (!pending_sz)
		return;

	/*
<<<<<<< HEAD
	 * Make sure all reads are done before we update the read index since
	 * the writer may start writing to the read area once the read index
	 * is updated.
	 */
	virt_mb();
=======
	 * Ensure the read of write_index in hv_get_bytes_to_write()
	 * happens after the read of pending_send_sz.
	 */
	virt_rmb();
	curr_write_sz = hv_get_bytes_to_write(rbi);
	bytes_read = hv_pkt_iter_bytes_read(rbi, start_read_index);
>>>>>>> temp

	/*
	 * If there was space before we began iteration,
	 * then host was not blocked.
	 */

<<<<<<< HEAD
	hv_signal_on_read(channel);

	return ret;
=======
	if (curr_write_sz - bytes_read > pending_sz)
		return;

	/* If pending write will not fit, don't give false hope. */
	if (curr_write_sz <= pending_sz)
		return;

	vmbus_setevent(channel);
>>>>>>> temp
}
EXPORT_SYMBOL_GPL(hv_pkt_iter_close);
