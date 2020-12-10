/****************************************************************************
 * Driver for Solarflare network controllers and boards
 * Copyright 2005-2006 Fen Systems Ltd.
 * Copyright 2005-2013 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */

#include <linux/pci.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/ipv6.h>
#include <linux/slab.h>
#include <net/ipv6.h>
#include <linux/if_ether.h>
#include <linux/highmem.h>
#include <linux/cache.h>
#include "net_driver.h"
#include "efx.h"
#include "io.h"
#include "nic.h"
#include "tx.h"
#include "workarounds.h"
#include "ef10_regs.h"

#ifdef EFX_USE_PIO

#define EFX_PIOBUF_SIZE_DEF ALIGN(256, L1_CACHE_BYTES)
unsigned int efx_piobuf_size __read_mostly = EFX_PIOBUF_SIZE_DEF;

#endif /* EFX_USE_PIO */

static inline u8 *efx_tx_get_copy_buffer(struct efx_tx_queue *tx_queue,
					 struct efx_tx_buffer *buffer)
{
	unsigned int index = efx_tx_queue_get_insert_index(tx_queue);
	struct efx_buffer *page_buf =
		&tx_queue->cb_page[index >> (PAGE_SHIFT - EFX_TX_CB_ORDER)];
	unsigned int offset =
		((index << EFX_TX_CB_ORDER) + NET_IP_ALIGN) & (PAGE_SIZE - 1);

	if (unlikely(!page_buf->addr) &&
	    efx_nic_alloc_buffer(tx_queue->efx, page_buf, PAGE_SIZE,
				 GFP_ATOMIC))
		return NULL;
	buffer->dma_addr = page_buf->dma_addr + offset;
	buffer->unmap_len = 0;
	return (u8 *)page_buf->addr + offset;
}

u8 *efx_tx_get_copy_buffer_limited(struct efx_tx_queue *tx_queue,
				   struct efx_tx_buffer *buffer, size_t len)
{
	if (len > EFX_TX_CB_SIZE)
		return NULL;
	return efx_tx_get_copy_buffer(tx_queue, buffer);
}

static void efx_dequeue_buffer(struct efx_tx_queue *tx_queue,
			       struct efx_tx_buffer *buffer,
			       unsigned int *pkts_compl,
			       unsigned int *bytes_compl)
{
	if (buffer->unmap_len) {
		struct device *dma_dev = &tx_queue->efx->pci_dev->dev;
		dma_addr_t unmap_addr = buffer->dma_addr - buffer->dma_offset;
		if (buffer->flags & EFX_TX_BUF_MAP_SINGLE)
			dma_unmap_single(dma_dev, unmap_addr, buffer->unmap_len,
					 DMA_TO_DEVICE);
		else
			dma_unmap_page(dma_dev, unmap_addr, buffer->unmap_len,
				       DMA_TO_DEVICE);
		buffer->unmap_len = 0;
	}

	if (buffer->flags & EFX_TX_BUF_SKB) {
		EFX_WARN_ON_PARANOID(!pkts_compl || !bytes_compl);
		(*pkts_compl)++;
		(*bytes_compl) += buffer->skb->len;
		dev_consume_skb_any((struct sk_buff *)buffer->skb);
		netif_vdbg(tx_queue->efx, tx_done, tx_queue->efx->net_dev,
			   "TX queue %d transmission id %x complete\n",
			   tx_queue->queue, tx_queue->read_count);
	}

	buffer->len = 0;
	buffer->flags = 0;
}

unsigned int efx_tx_max_skb_descs(struct efx_nic *efx)
{
	/* Header and payload descriptor for each output segment, plus
	 * one for every input fragment boundary within a segment
	 */
	unsigned int max_descs = EFX_TSO_MAX_SEGS * 2 + MAX_SKB_FRAGS;

	/* Possibly one more per segment for option descriptors */
	if (efx_nic_rev(efx) >= EFX_REV_HUNT_A0)
		max_descs += EFX_TSO_MAX_SEGS;

	/* Possibly more for PCIe page boundaries within input fragments */
	if (PAGE_SIZE > EFX_PAGE_SIZE)
		max_descs += max_t(unsigned int, MAX_SKB_FRAGS,
				   DIV_ROUND_UP(GSO_MAX_SIZE, EFX_PAGE_SIZE));

	return max_descs;
}

static void efx_tx_maybe_stop_queue(struct efx_tx_queue *txq1)
{
	/* We need to consider both queues that the net core sees as one */
	struct efx_tx_queue *txq2 = efx_tx_queue_partner(txq1);
	struct efx_nic *efx = txq1->efx;
	unsigned int fill_level;

	fill_level = max(txq1->insert_count - txq1->old_read_count,
			 txq2->insert_count - txq2->old_read_count);
	if (likely(fill_level < efx->txq_stop_thresh))
		return;

	/* We used the stale old_read_count above, which gives us a
	 * pessimistic estimate of the fill level (which may even
	 * validly be >= efx->txq_entries).  Now try again using
	 * read_count (more likely to be a cache miss).
	 *
	 * If we read read_count and then conditionally stop the
	 * queue, it is possible for the completion path to race with
	 * us and complete all outstanding descriptors in the middle,
	 * after which there will be no more completions to wake it.
	 * Therefore we stop the queue first, then read read_count
	 * (with a memory barrier to ensure the ordering), then
	 * restart the queue if the fill level turns out to be low
	 * enough.
	 */
	netif_tx_stop_queue(txq1->core_txq);
	smp_mb();
	txq1->old_read_count = READ_ONCE(txq1->read_count);
	txq2->old_read_count = READ_ONCE(txq2->read_count);

	fill_level = max(txq1->insert_count - txq1->old_read_count,
			 txq2->insert_count - txq2->old_read_count);
	EFX_WARN_ON_ONCE_PARANOID(fill_level >= efx->txq_entries);
	if (likely(fill_level < efx->txq_stop_thresh)) {
		smp_mb();
		if (likely(!efx->loopback_selftest))
			netif_tx_start_queue(txq1->core_txq);
	}
}

static int efx_enqueue_skb_copy(struct efx_tx_queue *tx_queue,
				struct sk_buff *skb)
{
	unsigned int copy_len = skb->len;
	struct efx_tx_buffer *buffer;
	u8 *copy_buffer;
	int rc;

	EFX_WARN_ON_ONCE_PARANOID(copy_len > EFX_TX_CB_SIZE);

	buffer = efx_tx_queue_get_insert_buffer(tx_queue);

	copy_buffer = efx_tx_get_copy_buffer(tx_queue, buffer);
	if (unlikely(!copy_buffer))
		return -ENOMEM;

	rc = skb_copy_bits(skb, 0, copy_buffer, copy_len);
	EFX_WARN_ON_PARANOID(rc);
	buffer->len = copy_len;

	buffer->skb = skb;
	buffer->flags = EFX_TX_BUF_SKB;

	++tx_queue->insert_count;
	return rc;
}

#ifdef EFX_USE_PIO

struct efx_short_copy_buffer {
	int used;
	u8 buf[L1_CACHE_BYTES];
};

/* Copy to PIO, respecting that writes to PIO buffers must be dword aligned.
 * Advances piobuf pointer. Leaves additional data in the copy buffer.
 */
static void efx_memcpy_toio_aligned(struct efx_nic *efx, u8 __iomem **piobuf,
				    u8 *data, int len,
				    struct efx_short_copy_buffer *copy_buf)
{
	int block_len = len & ~(sizeof(copy_buf->buf) - 1);

	__iowrite64_copy(*piobuf, data, block_len >> 3);
	*piobuf += block_len;
	len -= block_len;

	if (len) {
		data += block_len;
		BUG_ON(copy_buf->used);
		BUG_ON(len > sizeof(copy_buf->buf));
		memcpy(copy_buf->buf, data, len);
		copy_buf->used = len;
	}
}

/* Copy to PIO, respecting dword alignment, popping data from copy buffer first.
 * Advances piobuf pointer. Leaves additional data in the copy buffer.
 */
static void efx_memcpy_toio_aligned_cb(struct efx_nic *efx, u8 __iomem **piobuf,
				       u8 *data, int len,
				       struct efx_short_copy_buffer *copy_buf)
{
	if (copy_buf->used) {
		/* if the copy buffer is partially full, fill it up and write */
		int copy_to_buf =
			min_t(int, sizeof(copy_buf->buf) - copy_buf->used, len);

		memcpy(copy_buf->buf + copy_buf->used, data, copy_to_buf);
		copy_buf->used += copy_to_buf;

		/* if we didn't fill it up then we're done for now */
		if (copy_buf->used < sizeof(copy_buf->buf))
			return;

		__iowrite64_copy(*piobuf, copy_buf->buf,
				 sizeof(copy_buf->buf) >> 3);
		*piobuf += sizeof(copy_buf->buf);
		data += copy_to_buf;
		len -= copy_to_buf;
		copy_buf->used = 0;
	}

	efx_memcpy_toio_aligned(efx, piobuf, data, len, copy_buf);
}

static void efx_flush_copy_buffer(struct efx_nic *efx, u8 __iomem *piobuf,
				  struct efx_short_copy_buffer *copy_buf)
{
	/* if there's anything in it, write the whole buffer, including junk */
	if (copy_buf->used)
		__iowrite64_copy(piobuf, copy_buf->buf,
				 sizeof(copy_buf->buf) >> 3);
}

/* Traverse skb structure and copy fragments in to PIO buffer.
 * Advances piobuf pointer.
 */
static void efx_skb_copy_bits_to_pio(struct efx_nic *efx, struct sk_buff *skb,
				     u8 __iomem **piobuf,
				     struct efx_short_copy_buffer *copy_buf)
{
	int i;

	efx_memcpy_toio_aligned(efx, piobuf, skb->data, skb_headlen(skb),
				copy_buf);

	for (i = 0; i < skb_shinfo(skb)->nr_frags; ++i) {
		skb_frag_t *f = &skb_shinfo(skb)->frags[i];
		u8 *vaddr;

		vaddr = kmap_atomic(skb_frag_page(f));

		efx_memcpy_toio_aligned_cb(efx, piobuf, vaddr + f->page_offset,
					   skb_frag_size(f), copy_buf);
		kunmap_atomic(vaddr);
	}

	EFX_WARN_ON_ONCE_PARANOID(skb_shinfo(skb)->frag_list);
}

static int efx_enqueue_skb_pio(struct efx_tx_queue *tx_queue,
			       struct sk_buff *skb)
{
	struct efx_tx_buffer *buffer =
		efx_tx_queue_get_insert_buffer(tx_queue);
	u8 __iomem *piobuf = tx_queue->piobuf;

	/* Copy to PIO buffer. Ensure the writes are padded to the end
	 * of a cache line, as this is required for write-combining to be
	 * effective on at least x86.
	 */

	if (skb_shinfo(skb)->nr_frags) {
		/* The size of the copy buffer will ensure all writes
		 * are the size of a cache line.
		 */
		struct efx_short_copy_buffer copy_buf;

		copy_buf.used = 0;

		efx_skb_copy_bits_to_pio(tx_queue->efx, skb,
					 &piobuf, &copy_buf);
		efx_flush_copy_buffer(tx_queue->efx, piobuf, &copy_buf);
	} else {
		/* Pad the write to the size of a cache line.
		 * We can do this because we know the skb_shared_info struct is
		 * after the source, and the destination buffer is big enough.
		 */
		BUILD_BUG_ON(L1_CACHE_BYTES >
			     SKB_DATA_ALIGN(sizeof(struct skb_shared_info)));
		__iowrite64_copy(tx_queue->piobuf, skb->data,
				 ALIGN(skb->len, L1_CACHE_BYTES) >> 3);
	}

	buffer->skb = skb;
	buffer->flags = EFX_TX_BUF_SKB | EFX_TX_BUF_OPTION;

	EFX_POPULATE_QWORD_5(buffer->option,
			     ESF_DZ_TX_DESC_IS_OPT, 1,
			     ESF_DZ_TX_OPTION_TYPE, ESE_DZ_TX_OPTION_DESC_PIO,
			     ESF_DZ_TX_PIO_CONT, 0,
			     ESF_DZ_TX_PIO_BYTE_CNT, skb->len,
			     ESF_DZ_TX_PIO_BUF_ADDR,
			     tx_queue->piobuf_offset);
	++tx_queue->insert_count;
	return 0;
}
#endif /* EFX_USE_PIO */

static struct efx_tx_buffer *efx_tx_map_chunk(struct efx_tx_queue *tx_queue,
					      dma_addr_t dma_addr,
					      size_t len)
{
	const struct efx_nic_type *nic_type = tx_queue->efx->type;
	struct efx_tx_buffer *buffer;
	unsigned int dma_len;

	/* Map the fragment taking account of NIC-dependent DMA limits. */
	do {
		buffer = efx_tx_queue_get_insert_buffer(tx_queue);
		dma_len = nic_type->tx_limit_len(tx_queue, dma_addr, len);

		buffer->len = dma_len;
		buffer->dma_addr = dma_addr;
		buffer->flags = EFX_TX_BUF_CONT;
		len -= dma_len;
		dma_addr += dma_len;
		++tx_queue->insert_count;
	} while (len);

	return buffer;
}

/* Map all data from an SKB for DMA and create descriptors on the queue.
 */
static int efx_tx_map_data(struct efx_tx_queue *tx_queue, struct sk_buff *skb,
			   unsigned int segment_count)
{
	struct efx_nic *efx = tx_queue->efx;
	struct device *dma_dev = &efx->pci_dev->dev;
	unsigned int frag_index, nr_frags;
	dma_addr_t dma_addr, unmap_addr;
	unsigned short dma_flags;
	size_t len, unmap_len;

	nr_frags = skb_shinfo(skb)->nr_frags;
	frag_index = 0;

	/* Map header data. */
	len = skb_headlen(skb);
	dma_addr = dma_map_single(dma_dev, skb->data, len, DMA_TO_DEVICE);
	dma_flags = EFX_TX_BUF_MAP_SINGLE;
	unmap_len = len;
	unmap_addr = dma_addr;

	if (unlikely(dma_mapping_error(dma_dev, dma_addr)))
		return -EIO;

	if (segment_count) {
		/* For TSO we need to put the header in to a separate
		 * descriptor. Map this separately if necessary.
		 */
		size_t header_len = skb_transport_header(skb) - skb->data +
				(tcp_hdr(skb)->doff << 2u);

		if (header_len != len) {
			tx_queue->tso_long_headers++;
			efx_tx_map_chunk(tx_queue, dma_addr, header_len);
			len -= header_len;
			dma_addr += header_len;
		}
	}

	/* Add descriptors for each fragment. */
	do {
		struct efx_tx_buffer *buffer;
		skb_frag_t *fragment;

		buffer = efx_tx_map_chunk(tx_queue, dma_addr, len);

		/* The final descriptor for a fragment is responsible for
		 * unmapping the whole fragment.
		 */
		buffer->flags = EFX_TX_BUF_CONT | dma_flags;
		buffer->unmap_len = unmap_len;
		buffer->dma_offset = buffer->dma_addr - unmap_addr;

		if (frag_index >= nr_frags) {
			/* Store SKB details with the final buffer for
			 * the completion.
			 */
			buffer->skb = skb;
			buffer->flags = EFX_TX_BUF_SKB | dma_flags;
			return 0;
		}

		/* Move on to the next fragment. */
		fragment = &skb_shinfo(skb)->frags[frag_index++];
		len = skb_frag_size(fragment);
		dma_addr = skb_frag_dma_map(dma_dev, fragment,
				0, len, DMA_TO_DEVICE);
		dma_flags = 0;
		unmap_len = len;
		unmap_addr = dma_addr;

		if (unlikely(dma_mapping_error(dma_dev, dma_addr)))
			return -EIO;
	} while (1);
}

/* Remove buffers put into a tx_queue.  None of the buffers must have
 * an skb attached.
 */
static void efx_enqueue_unwind(struct efx_tx_queue *tx_queue)
{
	struct efx_tx_buffer *buffer;
	unsigned int bytes_compl = 0;
	unsigned int pkts_compl = 0;

	/* Work backwards until we hit the original insert pointer value */
	while (tx_queue->insert_count != tx_queue->write_count) {
		--tx_queue->insert_count;
		buffer = __efx_tx_queue_get_insert_buffer(tx_queue);
		efx_dequeue_buffer(tx_queue, buffer, &pkts_compl, &bytes_compl);
	}
}

/*
 * Fallback to software TSO.
 *
 * This is used if we are unable to send a GSO packet through hardware TSO.
 * This should only ever happen due to per-queue restrictions - unsupported
 * packets should first be filtered by the feature flags.
 *
 * Returns 0 on success, error code otherwise.
 */
static int efx_tx_tso_fallback(struct efx_tx_queue *tx_queue,
			       struct sk_buff *skb)
{
	struct sk_buff *segments, *next;

	segments = skb_gso_segment(skb, 0);
	if (IS_ERR(segments))
		return PTR_ERR(segments);

	dev_kfree_skb_any(skb);
	skb = segments;

	while (skb) {
		next = skb->next;
		skb->next = NULL;

		if (next)
			skb->xmit_more = true;
		efx_enqueue_skb(tx_queue, skb);
		skb = next;
	}

	return 0;
}

/*
 * Add a socket buffer to a TX queue
 *
 * This maps all fragments of a socket buffer for DMA and adds them to
 * the TX queue.  The queue's insert pointer will be incremented by
 * the number of fragments in the socket buffer.
 *
 * If any DMA mapping fails, any mapped fragments will be unmapped,
 * the queue's insert pointer will be restored to its original value.
 *
 * This function is split out from efx_hard_start_xmit to allow the
 * loopback test to direct packets via specific TX queues.
 *
 * Returns NETDEV_TX_OK.
 * You must hold netif_tx_lock() to call this function.
 */
netdev_tx_t efx_enqueue_skb(struct efx_tx_queue *tx_queue, struct sk_buff *skb)
{
	bool data_mapped = false;
	unsigned int segments;
	unsigned int skb_len;
	int rc;

	skb_len = skb->len;
	segments = skb_is_gso(skb) ? skb_shinfo(skb)->gso_segs : 0;
	if (segments == 1)
		segments = 0; /* Don't use TSO for a single segment. */

	/* Handle TSO first - it's *possible* (although unlikely) that we might
	 * be passed a packet to segment that's smaller than the copybreak/PIO
	 * size limit.
	 */
	if (segments) {
		EFX_WARN_ON_ONCE_PARANOID(!tx_queue->handle_tso);
		rc = tx_queue->handle_tso(tx_queue, skb, &data_mapped);
		if (rc == -EINVAL) {
			rc = efx_tx_tso_fallback(tx_queue, skb);
			tx_queue->tso_fallbacks++;
			if (rc == 0)
				return 0;
		}
		if (rc)
			goto err;
#ifdef EFX_USE_PIO
	} else if (skb_len <= efx_piobuf_size && !skb->xmit_more &&
		   efx_nic_may_tx_pio(tx_queue)) {
		/* Use PIO for short packets with an empty queue. */
		if (efx_enqueue_skb_pio(tx_queue, skb))
			goto err;
		tx_queue->pio_packets++;
		data_mapped = true;
#endif
	} else if (skb->data_len && skb_len <= EFX_TX_CB_SIZE) {
		/* Pad short packets or coalesce short fragmented packets. */
		if (efx_enqueue_skb_copy(tx_queue, skb))
			goto err;
		tx_queue->cb_packets++;
		data_mapped = true;
	}

	/* Map for DMA and create descriptors if we haven't done so already. */
	if (!data_mapped && (efx_tx_map_data(tx_queue, skb, segments)))
		goto err;

	/* Update BQL */
	netdev_tx_sent_queue(tx_queue->core_txq, skb_len);

	/* Pass off to hardware */
	if (!skb->xmit_more || netif_xmit_stopped(tx_queue->core_txq)) {
		struct efx_tx_queue *txq2 = efx_tx_queue_partner(tx_queue);

		/* There could be packets left on the partner queue if those
		 * SKBs had skb->xmit_more set. If we do not push those they
		 * could be left for a long time and cause a netdev watchdog.
		 */
		if (txq2->xmit_more_available)
			efx_nic_push_buffers(txq2);

		efx_nic_push_buffers(tx_queue);
	} else {
		tx_queue->xmit_more_available = skb->xmit_more;
	}

	if (segments) {
		tx_queue->tso_bursts++;
		tx_queue->tso_packets += segments;
		tx_queue->tx_packets  += segments;
	} else {
		tx_queue->tx_packets++;
	}

	efx_tx_maybe_stop_queue(tx_queue);

	return NETDEV_TX_OK;


err:
	efx_enqueue_unwind(tx_queue);
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

/* Remove packets from the TX queue
 *
 * This removes packets from the TX queue, up to and including the
 * specified index.
 */
static void efx_dequeue_buffers(struct efx_tx_queue *tx_queue,
				unsigned int index,
				unsigned int *pkts_compl,
				unsigned int *bytes_compl)
{
	struct efx_nic *efx = tx_queue->efx;
	unsigned int stop_index, read_ptr;

	stop_index = (index + 1) & tx_queue->ptr_mask;
	read_ptr = tx_queue->read_count & tx_queue->ptr_mask;

	while (read_ptr != stop_index) {
		struct efx_tx_buffer *buffer = &tx_queue->buffer[read_ptr];

		if (!(buffer->flags & EFX_TX_BUF_OPTION) &&
		    unlikely(buffer->len == 0)) {
			netif_err(efx, tx_err, efx->net_dev,
				  "TX queue %d spurious TX completion id %x\n",
				  tx_queue->queue, read_ptr);
			efx_schedule_reset(efx, RESET_TYPE_TX_SKIP);
			return;
		}

		efx_dequeue_buffer(tx_queue, buffer, pkts_compl, bytes_compl);

		++tx_queue->read_count;
		read_ptr = tx_queue->read_count & tx_queue->ptr_mask;
	}
}

/* Initiate a packet transmission.  We use one channel per CPU
 * (sharing when we have more CPUs than channels).  On Falcon, the TX
 * completion events will be directed back to the CPU that transmitted
 * the packet, which should be cache-efficient.
 *
 * Context: non-blocking.
 * Note that returning anything other than NETDEV_TX_OK will cause the
 * OS to free the skb.
 */
netdev_tx_t efx_hard_start_xmit(struct sk_buff *skb,
				struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct efx_tx_queue *tx_queue;
	unsigned index, type;

	EFX_WARN_ON_PARANOID(!netif_device_present(net_dev));

	/* PTP "event" packet */
	if (unlikely(efx_xmit_with_hwtstamp(skb)) &&
	    unlikely(efx_ptp_is_ptp_tx(efx, skb))) {
		return efx_ptp_tx(efx, skb);
	}

	index = skb_get_queue_mapping(skb);
	type = skb->ip_summed == CHECKSUM_PARTIAL ? EFX_TXQ_TYPE_OFFLOAD : 0;
	if (index >= efx->n_tx_channels) {
		index -= efx->n_tx_channels;
		type |= EFX_TXQ_TYPE_HIGHPRI;
	}
	tx_queue = efx_get_tx_queue(efx, index, type);

	return efx_enqueue_skb(tx_queue, skb);
}

void efx_init_tx_queue_core_txq(struct efx_tx_queue *tx_queue)
{
	struct efx_nic *efx = tx_queue->efx;

	/* Must be inverse of queue lookup in efx_hard_start_xmit() */
	tx_queue->core_txq =
		netdev_get_tx_queue(efx->net_dev,
				    tx_queue->queue / EFX_TXQ_TYPES +
				    ((tx_queue->queue & EFX_TXQ_TYPE_HIGHPRI) ?
				     efx->n_tx_channels : 0));
}

<<<<<<< HEAD
int efx_setup_tc(struct net_device *net_dev, u32 handle, __be16 proto,
		 struct tc_to_netdev *ntc)
=======
int efx_setup_tc(struct net_device *net_dev, enum tc_setup_type type,
		 void *type_data)
>>>>>>> temp
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct tc_mqprio_qopt *mqprio = type_data;
	struct efx_channel *channel;
	struct efx_tx_queue *tx_queue;
	unsigned tc, num_tc;
	int rc;

<<<<<<< HEAD
	if (handle != TC_H_ROOT || ntc->type != TC_SETUP_MQPRIO)
		return -EINVAL;

	num_tc = ntc->tc;

	if (efx_nic_rev(efx) < EFX_REV_FALCON_B0 || num_tc > EFX_MAX_TX_TC)
=======
	if (type != TC_SETUP_QDISC_MQPRIO)
		return -EOPNOTSUPP;

	num_tc = mqprio->num_tc;

	if (num_tc > EFX_MAX_TX_TC)
>>>>>>> temp
		return -EINVAL;

	mqprio->hw = TC_MQPRIO_HW_OFFLOAD_TCS;

	if (num_tc == net_dev->num_tc)
		return 0;

	for (tc = 0; tc < num_tc; tc++) {
		net_dev->tc_to_txq[tc].offset = tc * efx->n_tx_channels;
		net_dev->tc_to_txq[tc].count = efx->n_tx_channels;
	}

	if (num_tc > net_dev->num_tc) {
		/* Initialise high-priority queues as necessary */
		efx_for_each_channel(channel, efx) {
			efx_for_each_possible_channel_tx_queue(tx_queue,
							       channel) {
				if (!(tx_queue->queue & EFX_TXQ_TYPE_HIGHPRI))
					continue;
				if (!tx_queue->buffer) {
					rc = efx_probe_tx_queue(tx_queue);
					if (rc)
						return rc;
				}
				if (!tx_queue->initialised)
					efx_init_tx_queue(tx_queue);
				efx_init_tx_queue_core_txq(tx_queue);
			}
		}
	} else {
		/* Reduce number of classes before number of queues */
		net_dev->num_tc = num_tc;
	}

	rc = netif_set_real_num_tx_queues(net_dev,
					  max_t(int, num_tc, 1) *
					  efx->n_tx_channels);
	if (rc)
		return rc;

	/* Do not destroy high-priority queues when they become
	 * unused.  We would have to flush them first, and it is
	 * fairly difficult to flush a subset of TX queues.  Leave
	 * it to efx_fini_channels().
	 */

	net_dev->num_tc = num_tc;
	return 0;
}

void efx_xmit_done(struct efx_tx_queue *tx_queue, unsigned int index)
{
	unsigned fill_level;
	struct efx_nic *efx = tx_queue->efx;
	struct efx_tx_queue *txq2;
	unsigned int pkts_compl = 0, bytes_compl = 0;

	EFX_WARN_ON_ONCE_PARANOID(index > tx_queue->ptr_mask);

	efx_dequeue_buffers(tx_queue, index, &pkts_compl, &bytes_compl);
	tx_queue->pkts_compl += pkts_compl;
	tx_queue->bytes_compl += bytes_compl;

	if (pkts_compl > 1)
		++tx_queue->merge_events;

	/* See if we need to restart the netif queue.  This memory
	 * barrier ensures that we write read_count (inside
	 * efx_dequeue_buffers()) before reading the queue status.
	 */
	smp_mb();
	if (unlikely(netif_tx_queue_stopped(tx_queue->core_txq)) &&
	    likely(efx->port_enabled) &&
	    likely(netif_device_present(efx->net_dev))) {
		txq2 = efx_tx_queue_partner(tx_queue);
		fill_level = max(tx_queue->insert_count - tx_queue->read_count,
				 txq2->insert_count - txq2->read_count);
		if (fill_level <= efx->txq_wake_thresh)
			netif_tx_wake_queue(tx_queue->core_txq);
	}

	/* Check whether the hardware queue is now empty */
	if ((int)(tx_queue->read_count - tx_queue->old_write_count) >= 0) {
		tx_queue->old_write_count = READ_ONCE(tx_queue->write_count);
		if (tx_queue->read_count == tx_queue->old_write_count) {
			smp_mb();
			tx_queue->empty_read_count =
				tx_queue->read_count | EFX_EMPTY_COUNT_VALID;
		}
	}
}

static unsigned int efx_tx_cb_page_count(struct efx_tx_queue *tx_queue)
{
	return DIV_ROUND_UP(tx_queue->ptr_mask + 1, PAGE_SIZE >> EFX_TX_CB_ORDER);
}

int efx_probe_tx_queue(struct efx_tx_queue *tx_queue)
{
	struct efx_nic *efx = tx_queue->efx;
	unsigned int entries;
	int rc;

	/* Create the smallest power-of-two aligned ring */
	entries = max(roundup_pow_of_two(efx->txq_entries), EFX_MIN_DMAQ_SIZE);
	EFX_WARN_ON_PARANOID(entries > EFX_MAX_DMAQ_SIZE);
	tx_queue->ptr_mask = entries - 1;

	netif_dbg(efx, probe, efx->net_dev,
		  "creating TX queue %d size %#x mask %#x\n",
		  tx_queue->queue, efx->txq_entries, tx_queue->ptr_mask);

	/* Allocate software ring */
	tx_queue->buffer = kcalloc(entries, sizeof(*tx_queue->buffer),
				   GFP_KERNEL);
	if (!tx_queue->buffer)
		return -ENOMEM;

	tx_queue->cb_page = kcalloc(efx_tx_cb_page_count(tx_queue),
				    sizeof(tx_queue->cb_page[0]), GFP_KERNEL);
	if (!tx_queue->cb_page) {
		rc = -ENOMEM;
		goto fail1;
	}

	/* Allocate hardware ring */
	rc = efx_nic_probe_tx(tx_queue);
	if (rc)
		goto fail2;

	return 0;

fail2:
	kfree(tx_queue->cb_page);
	tx_queue->cb_page = NULL;
fail1:
	kfree(tx_queue->buffer);
	tx_queue->buffer = NULL;
	return rc;
}

void efx_init_tx_queue(struct efx_tx_queue *tx_queue)
{
	struct efx_nic *efx = tx_queue->efx;

	netif_dbg(efx, drv, efx->net_dev,
		  "initialising TX queue %d\n", tx_queue->queue);

	tx_queue->insert_count = 0;
	tx_queue->write_count = 0;
	tx_queue->packet_write_count = 0;
	tx_queue->old_write_count = 0;
	tx_queue->read_count = 0;
	tx_queue->old_read_count = 0;
	tx_queue->empty_read_count = 0 | EFX_EMPTY_COUNT_VALID;
	tx_queue->xmit_more_available = false;

	/* Set up default function pointers. These may get replaced by
	 * efx_nic_init_tx() based off NIC/queue capabilities.
	 */
	tx_queue->handle_tso = efx_enqueue_skb_tso;

	/* Set up TX descriptor ring */
	efx_nic_init_tx(tx_queue);

	tx_queue->initialised = true;
}

void efx_fini_tx_queue(struct efx_tx_queue *tx_queue)
{
	struct efx_tx_buffer *buffer;

	netif_dbg(tx_queue->efx, drv, tx_queue->efx->net_dev,
		  "shutting down TX queue %d\n", tx_queue->queue);

	if (!tx_queue->buffer)
		return;

	/* Free any buffers left in the ring */
	while (tx_queue->read_count != tx_queue->write_count) {
		unsigned int pkts_compl = 0, bytes_compl = 0;
		buffer = &tx_queue->buffer[tx_queue->read_count & tx_queue->ptr_mask];
		efx_dequeue_buffer(tx_queue, buffer, &pkts_compl, &bytes_compl);

		++tx_queue->read_count;
	}
	tx_queue->xmit_more_available = false;
	netdev_tx_reset_queue(tx_queue->core_txq);
}

void efx_remove_tx_queue(struct efx_tx_queue *tx_queue)
{
	int i;

	if (!tx_queue->buffer)
		return;

	netif_dbg(tx_queue->efx, drv, tx_queue->efx->net_dev,
		  "destroying TX queue %d\n", tx_queue->queue);
	efx_nic_remove_tx(tx_queue);

	if (tx_queue->cb_page) {
		for (i = 0; i < efx_tx_cb_page_count(tx_queue); i++)
			efx_nic_free_buffer(tx_queue->efx,
					    &tx_queue->cb_page[i]);
		kfree(tx_queue->cb_page);
		tx_queue->cb_page = NULL;
	}

	kfree(tx_queue->buffer);
	tx_queue->buffer = NULL;
}
<<<<<<< HEAD


/* Efx TCP segmentation acceleration.
 *
 * Why?  Because by doing it here in the driver we can go significantly
 * faster than the GSO.
 *
 * Requires TX checksum offload support.
 */

#define PTR_DIFF(p1, p2)  ((u8 *)(p1) - (u8 *)(p2))

/**
 * struct tso_state - TSO state for an SKB
 * @out_len: Remaining length in current segment
 * @seqnum: Current sequence number
 * @ipv4_id: Current IPv4 ID, host endian
 * @packet_space: Remaining space in current packet
 * @dma_addr: DMA address of current position
 * @in_len: Remaining length in current SKB fragment
 * @unmap_len: Length of SKB fragment
 * @unmap_addr: DMA address of SKB fragment
 * @dma_flags: TX buffer flags for DMA mapping - %EFX_TX_BUF_MAP_SINGLE or 0
 * @protocol: Network protocol (after any VLAN header)
 * @ip_off: Offset of IP header
 * @tcp_off: Offset of TCP header
 * @header_len: Number of bytes of header
 * @ip_base_len: IPv4 tot_len or IPv6 payload_len, before TCP payload
 * @header_dma_addr: Header DMA address, when using option descriptors
 * @header_unmap_len: Header DMA mapped length, or 0 if not using option
 *	descriptors
 *
 * The state used during segmentation.  It is put into this data structure
 * just to make it easy to pass into inline functions.
 */
struct tso_state {
	/* Output position */
	unsigned out_len;
	unsigned seqnum;
	u16 ipv4_id;
	unsigned packet_space;

	/* Input position */
	dma_addr_t dma_addr;
	unsigned in_len;
	unsigned unmap_len;
	dma_addr_t unmap_addr;
	unsigned short dma_flags;

	__be16 protocol;
	unsigned int ip_off;
	unsigned int tcp_off;
	unsigned header_len;
	unsigned int ip_base_len;
	dma_addr_t header_dma_addr;
	unsigned int header_unmap_len;
};


/*
 * Verify that our various assumptions about sk_buffs and the conditions
 * under which TSO will be attempted hold true.  Return the protocol number.
 */
static __be16 efx_tso_check_protocol(struct sk_buff *skb)
{
	__be16 protocol = skb->protocol;

	EFX_BUG_ON_PARANOID(((struct ethhdr *)skb->data)->h_proto !=
			    protocol);
	if (protocol == htons(ETH_P_8021Q)) {
		struct vlan_ethhdr *veh = (struct vlan_ethhdr *)skb->data;
		protocol = veh->h_vlan_encapsulated_proto;
	}

	if (protocol == htons(ETH_P_IP)) {
		EFX_BUG_ON_PARANOID(ip_hdr(skb)->protocol != IPPROTO_TCP);
	} else {
		EFX_BUG_ON_PARANOID(protocol != htons(ETH_P_IPV6));
		EFX_BUG_ON_PARANOID(ipv6_hdr(skb)->nexthdr != NEXTHDR_TCP);
	}
	EFX_BUG_ON_PARANOID((PTR_DIFF(tcp_hdr(skb), skb->data)
			     + (tcp_hdr(skb)->doff << 2u)) >
			    skb_headlen(skb));

	return protocol;
}

static u8 *efx_tsoh_get_buffer(struct efx_tx_queue *tx_queue,
			       struct efx_tx_buffer *buffer, unsigned int len)
{
	u8 *result;

	EFX_BUG_ON_PARANOID(buffer->len);
	EFX_BUG_ON_PARANOID(buffer->flags);
	EFX_BUG_ON_PARANOID(buffer->unmap_len);

	if (likely(len <= TSOH_STD_SIZE - NET_IP_ALIGN)) {
		unsigned index =
			(tx_queue->insert_count & tx_queue->ptr_mask) / 2;
		struct efx_buffer *page_buf =
			&tx_queue->tsoh_page[index / TSOH_PER_PAGE];
		unsigned offset =
			TSOH_STD_SIZE * (index % TSOH_PER_PAGE) + NET_IP_ALIGN;

		if (unlikely(!page_buf->addr) &&
		    efx_nic_alloc_buffer(tx_queue->efx, page_buf, PAGE_SIZE,
					 GFP_ATOMIC))
			return NULL;

		result = (u8 *)page_buf->addr + offset;
		buffer->dma_addr = page_buf->dma_addr + offset;
		buffer->flags = EFX_TX_BUF_CONT;
	} else {
		tx_queue->tso_long_headers++;

		buffer->heap_buf = kmalloc(NET_IP_ALIGN + len, GFP_ATOMIC);
		if (unlikely(!buffer->heap_buf))
			return NULL;
		result = (u8 *)buffer->heap_buf + NET_IP_ALIGN;
		buffer->flags = EFX_TX_BUF_CONT | EFX_TX_BUF_HEAP;
	}

	buffer->len = len;

	return result;
}

/**
 * efx_tx_queue_insert - push descriptors onto the TX queue
 * @tx_queue:		Efx TX queue
 * @dma_addr:		DMA address of fragment
 * @len:		Length of fragment
 * @final_buffer:	The final buffer inserted into the queue
 *
 * Push descriptors onto the TX queue.
 */
static void efx_tx_queue_insert(struct efx_tx_queue *tx_queue,
				dma_addr_t dma_addr, unsigned len,
				struct efx_tx_buffer **final_buffer)
{
	struct efx_tx_buffer *buffer;
	struct efx_nic *efx = tx_queue->efx;
	unsigned dma_len;

	EFX_BUG_ON_PARANOID(len <= 0);

	while (1) {
		buffer = efx_tx_queue_get_insert_buffer(tx_queue);
		++tx_queue->insert_count;

		EFX_BUG_ON_PARANOID(tx_queue->insert_count -
				    tx_queue->read_count >=
				    efx->txq_entries);

		buffer->dma_addr = dma_addr;

		dma_len = efx_max_tx_len(efx, dma_addr);

		/* If there is enough space to send then do so */
		if (dma_len >= len)
			break;

		buffer->len = dma_len;
		buffer->flags = EFX_TX_BUF_CONT;
		dma_addr += dma_len;
		len -= dma_len;
	}

	EFX_BUG_ON_PARANOID(!len);
	buffer->len = len;
	*final_buffer = buffer;
}


/*
 * Put a TSO header into the TX queue.
 *
 * This is special-cased because we know that it is small enough to fit in
 * a single fragment, and we know it doesn't cross a page boundary.  It
 * also allows us to not worry about end-of-packet etc.
 */
static int efx_tso_put_header(struct efx_tx_queue *tx_queue,
			      struct efx_tx_buffer *buffer, u8 *header)
{
	if (unlikely(buffer->flags & EFX_TX_BUF_HEAP)) {
		buffer->dma_addr = dma_map_single(&tx_queue->efx->pci_dev->dev,
						  header, buffer->len,
						  DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(&tx_queue->efx->pci_dev->dev,
					       buffer->dma_addr))) {
			kfree(buffer->heap_buf);
			buffer->len = 0;
			buffer->flags = 0;
			return -ENOMEM;
		}
		buffer->unmap_len = buffer->len;
		buffer->dma_offset = 0;
		buffer->flags |= EFX_TX_BUF_MAP_SINGLE;
	}

	++tx_queue->insert_count;
	return 0;
}


/* Remove buffers put into a tx_queue.  None of the buffers must have
 * an skb attached.
 */
static void efx_enqueue_unwind(struct efx_tx_queue *tx_queue,
			       unsigned int insert_count)
{
	struct efx_tx_buffer *buffer;

	/* Work backwards until we hit the original insert pointer value */
	while (tx_queue->insert_count != insert_count) {
		--tx_queue->insert_count;
		buffer = __efx_tx_queue_get_insert_buffer(tx_queue);
		efx_dequeue_buffer(tx_queue, buffer, NULL, NULL);
	}
}


/* Parse the SKB header and initialise state. */
static int tso_start(struct tso_state *st, struct efx_nic *efx,
		     struct efx_tx_queue *tx_queue,
		     const struct sk_buff *skb)
{
	struct device *dma_dev = &efx->pci_dev->dev;
	unsigned int header_len, in_len;
	bool use_opt_desc = false;
	dma_addr_t dma_addr;

	if (tx_queue->tso_version == 1)
		use_opt_desc = true;

	st->ip_off = skb_network_header(skb) - skb->data;
	st->tcp_off = skb_transport_header(skb) - skb->data;
	header_len = st->tcp_off + (tcp_hdr(skb)->doff << 2u);
	in_len = skb_headlen(skb) - header_len;
	st->header_len = header_len;
	st->in_len = in_len;
	if (st->protocol == htons(ETH_P_IP)) {
		st->ip_base_len = st->header_len - st->ip_off;
		st->ipv4_id = ntohs(ip_hdr(skb)->id);
	} else {
		st->ip_base_len = st->header_len - st->tcp_off;
		st->ipv4_id = 0;
	}
	st->seqnum = ntohl(tcp_hdr(skb)->seq);

	EFX_BUG_ON_PARANOID(tcp_hdr(skb)->urg);
	EFX_BUG_ON_PARANOID(tcp_hdr(skb)->syn);
	EFX_BUG_ON_PARANOID(tcp_hdr(skb)->rst);

	st->out_len = skb->len - header_len;

	if (!use_opt_desc) {
		st->header_unmap_len = 0;

		if (likely(in_len == 0)) {
			st->dma_flags = 0;
			st->unmap_len = 0;
			return 0;
		}

		dma_addr = dma_map_single(dma_dev, skb->data + header_len,
					  in_len, DMA_TO_DEVICE);
		st->dma_flags = EFX_TX_BUF_MAP_SINGLE;
		st->dma_addr = dma_addr;
		st->unmap_addr = dma_addr;
		st->unmap_len = in_len;
	} else {
		dma_addr = dma_map_single(dma_dev, skb->data,
					  skb_headlen(skb), DMA_TO_DEVICE);
		st->header_dma_addr = dma_addr;
		st->header_unmap_len = skb_headlen(skb);
		st->dma_flags = 0;
		st->dma_addr = dma_addr + header_len;
		st->unmap_len = 0;
	}

	return unlikely(dma_mapping_error(dma_dev, dma_addr)) ? -ENOMEM : 0;
}

static int tso_get_fragment(struct tso_state *st, struct efx_nic *efx,
			    skb_frag_t *frag)
{
	st->unmap_addr = skb_frag_dma_map(&efx->pci_dev->dev, frag, 0,
					  skb_frag_size(frag), DMA_TO_DEVICE);
	if (likely(!dma_mapping_error(&efx->pci_dev->dev, st->unmap_addr))) {
		st->dma_flags = 0;
		st->unmap_len = skb_frag_size(frag);
		st->in_len = skb_frag_size(frag);
		st->dma_addr = st->unmap_addr;
		return 0;
	}
	return -ENOMEM;
}


/**
 * tso_fill_packet_with_fragment - form descriptors for the current fragment
 * @tx_queue:		Efx TX queue
 * @skb:		Socket buffer
 * @st:			TSO state
 *
 * Form descriptors for the current fragment, until we reach the end
 * of fragment or end-of-packet.
 */
static void tso_fill_packet_with_fragment(struct efx_tx_queue *tx_queue,
					  const struct sk_buff *skb,
					  struct tso_state *st)
{
	struct efx_tx_buffer *buffer;
	int n;

	if (st->in_len == 0)
		return;
	if (st->packet_space == 0)
		return;

	EFX_BUG_ON_PARANOID(st->in_len <= 0);
	EFX_BUG_ON_PARANOID(st->packet_space <= 0);

	n = min(st->in_len, st->packet_space);

	st->packet_space -= n;
	st->out_len -= n;
	st->in_len -= n;

	efx_tx_queue_insert(tx_queue, st->dma_addr, n, &buffer);

	if (st->out_len == 0) {
		/* Transfer ownership of the skb */
		buffer->skb = skb;
		buffer->flags = EFX_TX_BUF_SKB;
	} else if (st->packet_space != 0) {
		buffer->flags = EFX_TX_BUF_CONT;
	}

	if (st->in_len == 0) {
		/* Transfer ownership of the DMA mapping */
		buffer->unmap_len = st->unmap_len;
		buffer->dma_offset = buffer->unmap_len - buffer->len;
		buffer->flags |= st->dma_flags;
		st->unmap_len = 0;
	}

	st->dma_addr += n;
}


/**
 * tso_start_new_packet - generate a new header and prepare for the new packet
 * @tx_queue:		Efx TX queue
 * @skb:		Socket buffer
 * @st:			TSO state
 *
 * Generate a new header and prepare for the new packet.  Return 0 on
 * success, or -%ENOMEM if failed to alloc header.
 */
static int tso_start_new_packet(struct efx_tx_queue *tx_queue,
				const struct sk_buff *skb,
				struct tso_state *st)
{
	struct efx_tx_buffer *buffer =
		efx_tx_queue_get_insert_buffer(tx_queue);
	bool is_last = st->out_len <= skb_shinfo(skb)->gso_size;
	u8 tcp_flags_clear;

	if (!is_last) {
		st->packet_space = skb_shinfo(skb)->gso_size;
		tcp_flags_clear = 0x09; /* mask out FIN and PSH */
	} else {
		st->packet_space = st->out_len;
		tcp_flags_clear = 0x00;
	}

	if (!st->header_unmap_len) {
		/* Allocate and insert a DMA-mapped header buffer. */
		struct tcphdr *tsoh_th;
		unsigned ip_length;
		u8 *header;
		int rc;

		header = efx_tsoh_get_buffer(tx_queue, buffer, st->header_len);
		if (!header)
			return -ENOMEM;

		tsoh_th = (struct tcphdr *)(header + st->tcp_off);

		/* Copy and update the headers. */
		memcpy(header, skb->data, st->header_len);

		tsoh_th->seq = htonl(st->seqnum);
		((u8 *)tsoh_th)[13] &= ~tcp_flags_clear;

		ip_length = st->ip_base_len + st->packet_space;

		if (st->protocol == htons(ETH_P_IP)) {
			struct iphdr *tsoh_iph =
				(struct iphdr *)(header + st->ip_off);

			tsoh_iph->tot_len = htons(ip_length);
			tsoh_iph->id = htons(st->ipv4_id);
		} else {
			struct ipv6hdr *tsoh_iph =
				(struct ipv6hdr *)(header + st->ip_off);

			tsoh_iph->payload_len = htons(ip_length);
		}

		rc = efx_tso_put_header(tx_queue, buffer, header);
		if (unlikely(rc))
			return rc;
	} else {
		/* Send the original headers with a TSO option descriptor
		 * in front
		 */
		u8 tcp_flags = ((u8 *)tcp_hdr(skb))[13] & ~tcp_flags_clear;

		buffer->flags = EFX_TX_BUF_OPTION;
		buffer->len = 0;
		buffer->unmap_len = 0;
		EFX_POPULATE_QWORD_5(buffer->option,
				     ESF_DZ_TX_DESC_IS_OPT, 1,
				     ESF_DZ_TX_OPTION_TYPE,
				     ESE_DZ_TX_OPTION_DESC_TSO,
				     ESF_DZ_TX_TSO_TCP_FLAGS, tcp_flags,
				     ESF_DZ_TX_TSO_IP_ID, st->ipv4_id,
				     ESF_DZ_TX_TSO_TCP_SEQNO, st->seqnum);
		++tx_queue->insert_count;

		/* We mapped the headers in tso_start().  Unmap them
		 * when the last segment is completed.
		 */
		buffer = efx_tx_queue_get_insert_buffer(tx_queue);
		buffer->dma_addr = st->header_dma_addr;
		buffer->len = st->header_len;
		if (is_last) {
			buffer->flags = EFX_TX_BUF_CONT | EFX_TX_BUF_MAP_SINGLE;
			buffer->unmap_len = st->header_unmap_len;
			buffer->dma_offset = 0;
			/* Ensure we only unmap them once in case of a
			 * later DMA mapping error and rollback
			 */
			st->header_unmap_len = 0;
		} else {
			buffer->flags = EFX_TX_BUF_CONT;
			buffer->unmap_len = 0;
		}
		++tx_queue->insert_count;
	}

	st->seqnum += skb_shinfo(skb)->gso_size;

	/* Linux leaves suitable gaps in the IP ID space for us to fill. */
	++st->ipv4_id;

	++tx_queue->tso_packets;

	++tx_queue->tx_packets;

	return 0;
}


/**
 * efx_enqueue_skb_tso - segment and transmit a TSO socket buffer
 * @tx_queue:		Efx TX queue
 * @skb:		Socket buffer
 *
 * Context: You must hold netif_tx_lock() to call this function.
 *
 * Add socket buffer @skb to @tx_queue, doing TSO or return != 0 if
 * @skb was not enqueued.  In all cases @skb is consumed.  Return
 * %NETDEV_TX_OK.
 */
static int efx_enqueue_skb_tso(struct efx_tx_queue *tx_queue,
			       struct sk_buff *skb)
{
	struct efx_nic *efx = tx_queue->efx;
	unsigned int old_insert_count = tx_queue->insert_count;
	int frag_i, rc;
	struct tso_state state;

	/* Find the packet protocol and sanity-check it */
	state.protocol = efx_tso_check_protocol(skb);

	rc = tso_start(&state, efx, tx_queue, skb);
	if (rc)
		goto mem_err;

	if (likely(state.in_len == 0)) {
		/* Grab the first payload fragment. */
		EFX_BUG_ON_PARANOID(skb_shinfo(skb)->nr_frags < 1);
		frag_i = 0;
		rc = tso_get_fragment(&state, efx,
				      skb_shinfo(skb)->frags + frag_i);
		if (rc)
			goto mem_err;
	} else {
		/* Payload starts in the header area. */
		frag_i = -1;
	}

	if (tso_start_new_packet(tx_queue, skb, &state) < 0)
		goto mem_err;

	while (1) {
		tso_fill_packet_with_fragment(tx_queue, skb, &state);

		/* Move onto the next fragment? */
		if (state.in_len == 0) {
			if (++frag_i >= skb_shinfo(skb)->nr_frags)
				/* End of payload reached. */
				break;
			rc = tso_get_fragment(&state, efx,
					      skb_shinfo(skb)->frags + frag_i);
			if (rc)
				goto mem_err;
		}

		/* Start at new packet? */
		if (state.packet_space == 0 &&
		    tso_start_new_packet(tx_queue, skb, &state) < 0)
			goto mem_err;
	}

	netdev_tx_sent_queue(tx_queue->core_txq, skb->len);

	efx_tx_maybe_stop_queue(tx_queue);

	/* Pass off to hardware */
	if (!skb->xmit_more || netif_xmit_stopped(tx_queue->core_txq)) {
		struct efx_tx_queue *txq2 = efx_tx_queue_partner(tx_queue);

		/* There could be packets left on the partner queue if those
		 * SKBs had skb->xmit_more set. If we do not push those they
		 * could be left for a long time and cause a netdev watchdog.
		 */
		if (txq2->xmit_more_available)
			efx_nic_push_buffers(txq2);

		efx_nic_push_buffers(tx_queue);
	} else {
		tx_queue->xmit_more_available = skb->xmit_more;
	}

	tx_queue->tso_bursts++;
	return NETDEV_TX_OK;

 mem_err:
	netif_err(efx, tx_err, efx->net_dev,
		  "Out of memory for TSO headers, or DMA mapping error\n");
	dev_kfree_skb_any(skb);

	/* Free the DMA mapping we were in the process of writing out */
	if (state.unmap_len) {
		if (state.dma_flags & EFX_TX_BUF_MAP_SINGLE)
			dma_unmap_single(&efx->pci_dev->dev, state.unmap_addr,
					 state.unmap_len, DMA_TO_DEVICE);
		else
			dma_unmap_page(&efx->pci_dev->dev, state.unmap_addr,
				       state.unmap_len, DMA_TO_DEVICE);
	}

	/* Free the header DMA mapping, if using option descriptors */
	if (state.header_unmap_len)
		dma_unmap_single(&efx->pci_dev->dev, state.header_dma_addr,
				 state.header_unmap_len, DMA_TO_DEVICE);

	efx_enqueue_unwind(tx_queue, old_insert_count);
	return NETDEV_TX_OK;
}
=======
>>>>>>> temp
