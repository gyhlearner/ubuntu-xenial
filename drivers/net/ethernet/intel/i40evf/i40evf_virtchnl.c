/*******************************************************************************
 *
 * Intel Ethernet Controller XL710 Family Linux Virtual Function Driver
 * Copyright(c) 2013 - 2014 Intel Corporation.
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
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 ******************************************************************************/

#include "i40evf.h"
#include "i40e_prototype.h"
#include "i40evf_client.h"

/* busy wait delay in msec */
#define I40EVF_BUSY_WAIT_DELAY 10
#define I40EVF_BUSY_WAIT_COUNT 50

/**
 * i40evf_send_pf_msg
 * @adapter: adapter structure
 * @op: virtual channel opcode
 * @msg: pointer to message buffer
 * @len: message length
 *
 * Send message to PF and print status if failure.
 **/
static int i40evf_send_pf_msg(struct i40evf_adapter *adapter,
			      enum virtchnl_ops op, u8 *msg, u16 len)
{
	struct i40e_hw *hw = &adapter->hw;
	i40e_status err;

	if (adapter->flags & I40EVF_FLAG_PF_COMMS_FAILED)
		return 0; /* nothing to see here, move along */

	err = i40e_aq_send_msg_to_pf(hw, op, 0, msg, len, NULL);
	if (err)
		dev_dbg(&adapter->pdev->dev, "Unable to send opcode %d to PF, err %s, aq_err %s\n",
			op, i40evf_stat_str(hw, err),
			i40evf_aq_str(hw, hw->aq.asq_last_status));
	return err;
}

/**
 * i40evf_send_api_ver
 * @adapter: adapter structure
 *
 * Send API version admin queue message to the PF. The reply is not checked
 * in this function. Returns 0 if the message was successfully
 * sent, or one of the I40E_ADMIN_QUEUE_ERROR_ statuses if not.
 **/
int i40evf_send_api_ver(struct i40evf_adapter *adapter)
{
	struct virtchnl_version_info vvi;

	vvi.major = VIRTCHNL_VERSION_MAJOR;
	vvi.minor = VIRTCHNL_VERSION_MINOR;

	return i40evf_send_pf_msg(adapter, VIRTCHNL_OP_VERSION, (u8 *)&vvi,
				  sizeof(vvi));
}

/**
 * i40evf_verify_api_ver
 * @adapter: adapter structure
 *
 * Compare API versions with the PF. Must be called after admin queue is
 * initialized. Returns 0 if API versions match, -EIO if they do not,
 * I40E_ERR_ADMIN_QUEUE_NO_WORK if the admin queue is empty, and any errors
 * from the firmware are propagated.
 **/
int i40evf_verify_api_ver(struct i40evf_adapter *adapter)
{
	struct virtchnl_version_info *pf_vvi;
	struct i40e_hw *hw = &adapter->hw;
	struct i40e_arq_event_info event;
	enum virtchnl_ops op;
	i40e_status err;

	event.buf_len = I40EVF_MAX_AQ_BUF_SIZE;
	event.msg_buf = kzalloc(event.buf_len, GFP_KERNEL);
	if (!event.msg_buf) {
		err = -ENOMEM;
		goto out;
	}

	while (1) {
		err = i40evf_clean_arq_element(hw, &event, NULL);
		/* When the AQ is empty, i40evf_clean_arq_element will return
		 * nonzero and this loop will terminate.
		 */
		if (err)
			goto out_alloc;
		op =
		    (enum virtchnl_ops)le32_to_cpu(event.desc.cookie_high);
		if (op == VIRTCHNL_OP_VERSION)
			break;
	}


	err = (i40e_status)le32_to_cpu(event.desc.cookie_low);
	if (err)
		goto out_alloc;

	if (op != VIRTCHNL_OP_VERSION) {
		dev_info(&adapter->pdev->dev, "Invalid reply type %d from PF\n",
			op);
		err = -EIO;
		goto out_alloc;
	}

	pf_vvi = (struct virtchnl_version_info *)event.msg_buf;
	adapter->pf_version = *pf_vvi;

	if ((pf_vvi->major > VIRTCHNL_VERSION_MAJOR) ||
	    ((pf_vvi->major == VIRTCHNL_VERSION_MAJOR) &&
	     (pf_vvi->minor > VIRTCHNL_VERSION_MINOR)))
		err = -EIO;

out_alloc:
	kfree(event.msg_buf);
out:
	return err;
}

/**
 * i40evf_send_vf_config_msg
 * @adapter: adapter structure
 *
 * Send VF configuration request admin queue message to the PF. The reply
 * is not checked in this function. Returns 0 if the message was
 * successfully sent, or one of the I40E_ADMIN_QUEUE_ERROR_ statuses if not.
 **/
int i40evf_send_vf_config_msg(struct i40evf_adapter *adapter)
{
	u32 caps;

<<<<<<< HEAD
	adapter->current_op = I40E_VIRTCHNL_OP_GET_VF_RESOURCES;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_GET_CONFIG;
	caps = I40E_VIRTCHNL_VF_OFFLOAD_L2 |
	       I40E_VIRTCHNL_VF_OFFLOAD_RSS_AQ |
	       I40E_VIRTCHNL_VF_OFFLOAD_RSS_REG |
	       I40E_VIRTCHNL_VF_OFFLOAD_VLAN |
	       I40E_VIRTCHNL_VF_OFFLOAD_WB_ON_ITR |
	       I40E_VIRTCHNL_VF_OFFLOAD_RSS_PCTYPE_V2;

	adapter->current_op = I40E_VIRTCHNL_OP_GET_VF_RESOURCES;
=======
	caps = VIRTCHNL_VF_OFFLOAD_L2 |
	       VIRTCHNL_VF_OFFLOAD_RSS_PF |
	       VIRTCHNL_VF_OFFLOAD_RSS_AQ |
	       VIRTCHNL_VF_OFFLOAD_RSS_REG |
	       VIRTCHNL_VF_OFFLOAD_VLAN |
	       VIRTCHNL_VF_OFFLOAD_WB_ON_ITR |
	       VIRTCHNL_VF_OFFLOAD_RSS_PCTYPE_V2 |
	       VIRTCHNL_VF_OFFLOAD_ENCAP |
	       VIRTCHNL_VF_OFFLOAD_ENCAP_CSUM |
	       VIRTCHNL_VF_OFFLOAD_REQ_QUEUES;

	adapter->current_op = VIRTCHNL_OP_GET_VF_RESOURCES;
>>>>>>> temp
	adapter->aq_required &= ~I40EVF_FLAG_AQ_GET_CONFIG;
	if (PF_IS_V11(adapter))
		return i40evf_send_pf_msg(adapter,
					  VIRTCHNL_OP_GET_VF_RESOURCES,
					  (u8 *)&caps, sizeof(caps));
	else
		return i40evf_send_pf_msg(adapter,
					  VIRTCHNL_OP_GET_VF_RESOURCES,
					  NULL, 0);
}

/**
 * i40evf_get_vf_config
 * @hw: pointer to the hardware structure
 * @len: length of buffer
 *
 * Get VF configuration from PF and populate hw structure. Must be called after
 * admin queue is initialized. Busy waits until response is received from PF,
 * with maximum timeout. Response from PF is returned in the buffer for further
 * processing by the caller.
 **/
int i40evf_get_vf_config(struct i40evf_adapter *adapter)
{
	struct i40e_hw *hw = &adapter->hw;
	struct i40e_arq_event_info event;
	enum virtchnl_ops op;
	i40e_status err;
	u16 len;

	len =  sizeof(struct virtchnl_vf_resource) +
		I40E_MAX_VF_VSI * sizeof(struct virtchnl_vsi_resource);
	event.buf_len = len;
	event.msg_buf = kzalloc(event.buf_len, GFP_KERNEL);
	if (!event.msg_buf) {
		err = -ENOMEM;
		goto out;
	}

	while (1) {
		/* When the AQ is empty, i40evf_clean_arq_element will return
		 * nonzero and this loop will terminate.
		 */
		err = i40evf_clean_arq_element(hw, &event, NULL);
		if (err)
			goto out_alloc;
		op =
		    (enum virtchnl_ops)le32_to_cpu(event.desc.cookie_high);
		if (op == VIRTCHNL_OP_GET_VF_RESOURCES)
			break;
	}

	err = (i40e_status)le32_to_cpu(event.desc.cookie_low);
	memcpy(adapter->vf_res, event.msg_buf, min(event.msg_len, len));

	i40e_vf_parse_hw_config(hw, adapter->vf_res);
out_alloc:
	kfree(event.msg_buf);
out:
	return err;
}

/**
 * i40evf_configure_queues
 * @adapter: adapter structure
 *
 * Request that the PF set up our (previously allocated) queues.
 **/
void i40evf_configure_queues(struct i40evf_adapter *adapter)
{
	struct virtchnl_vsi_queue_config_info *vqci;
	struct virtchnl_queue_pair_info *vqpi;
	int pairs = adapter->num_active_queues;
	int i, len, max_frame = I40E_MAX_RXBUFFER;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot configure queues, command %d pending\n",
			adapter->current_op);
		return;
	}
<<<<<<< HEAD
	adapter->current_op = I40E_VIRTCHNL_OP_CONFIG_VSI_QUEUES;
	len = sizeof(struct i40e_virtchnl_vsi_queue_config_info) +
		       (sizeof(struct i40e_virtchnl_queue_pair_info) * pairs);
=======
	adapter->current_op = VIRTCHNL_OP_CONFIG_VSI_QUEUES;
	len = sizeof(struct virtchnl_vsi_queue_config_info) +
		       (sizeof(struct virtchnl_queue_pair_info) * pairs);
>>>>>>> temp
	vqci = kzalloc(len, GFP_KERNEL);
	if (!vqci)
		return;

	/* Limit maximum frame size when jumbo frames is not enabled */
	if (!(adapter->flags & I40EVF_FLAG_LEGACY_RX) &&
	    (adapter->netdev->mtu <= ETH_DATA_LEN))
		max_frame = I40E_RXBUFFER_1536 - NET_IP_ALIGN;

	vqci->vsi_id = adapter->vsi_res->vsi_id;
	vqci->num_queue_pairs = pairs;
	vqpi = vqci->qpair;
	/* Size check is not needed here - HW max is 16 queue pairs, and we
	 * can fit info for 31 of them into the AQ buffer before it overflows.
	 */
	for (i = 0; i < pairs; i++) {
		vqpi->txq.vsi_id = vqci->vsi_id;
		vqpi->txq.queue_id = i;
		vqpi->txq.ring_len = adapter->tx_rings[i].count;
		vqpi->txq.dma_ring_addr = adapter->tx_rings[i].dma;
<<<<<<< HEAD
		vqpi->txq.headwb_enabled = 1;
		vqpi->txq.dma_headwb_addr = vqpi->txq.dma_ring_addr +
		    (vqpi->txq.ring_len * sizeof(struct i40e_tx_desc));

=======
>>>>>>> temp
		vqpi->rxq.vsi_id = vqci->vsi_id;
		vqpi->rxq.queue_id = i;
		vqpi->rxq.ring_len = adapter->rx_rings[i].count;
		vqpi->rxq.dma_ring_addr = adapter->rx_rings[i].dma;
<<<<<<< HEAD
		vqpi->rxq.max_pkt_size = adapter->netdev->mtu
					+ ETH_HLEN + VLAN_HLEN + ETH_FCS_LEN;
		vqpi->rxq.databuffer_size = adapter->rx_rings[i].rx_buf_len;
		if (adapter->flags & I40EVF_FLAG_RX_PS_ENABLED) {
			vqpi->rxq.splithdr_enabled = true;
			vqpi->rxq.hdr_size = I40E_RX_HDR_SIZE;
		}
=======
		vqpi->rxq.max_pkt_size = max_frame;
		vqpi->rxq.databuffer_size =
			ALIGN(adapter->rx_rings[i].rx_buf_len,
			      BIT_ULL(I40E_RXQ_CTX_DBUFF_SHIFT));
>>>>>>> temp
		vqpi++;
	}

	adapter->aq_required &= ~I40EVF_FLAG_AQ_CONFIGURE_QUEUES;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_CONFIG_VSI_QUEUES,
			   (u8 *)vqci, len);
	kfree(vqci);
}

/**
 * i40evf_enable_queues
 * @adapter: adapter structure
 *
 * Request that the PF enable all of our queues.
 **/
void i40evf_enable_queues(struct i40evf_adapter *adapter)
{
	struct virtchnl_queue_select vqs;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot enable queues, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_ENABLE_QUEUES;
	vqs.vsi_id = adapter->vsi_res->vsi_id;
	vqs.tx_queues = BIT(adapter->num_active_queues) - 1;
	vqs.rx_queues = vqs.tx_queues;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_ENABLE_QUEUES;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_ENABLE_QUEUES,
			   (u8 *)&vqs, sizeof(vqs));
}

/**
 * i40evf_disable_queues
 * @adapter: adapter structure
 *
 * Request that the PF disable all of our queues.
 **/
void i40evf_disable_queues(struct i40evf_adapter *adapter)
{
	struct virtchnl_queue_select vqs;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot disable queues, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_DISABLE_QUEUES;
	vqs.vsi_id = adapter->vsi_res->vsi_id;
	vqs.tx_queues = BIT(adapter->num_active_queues) - 1;
	vqs.rx_queues = vqs.tx_queues;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_DISABLE_QUEUES;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_DISABLE_QUEUES,
			   (u8 *)&vqs, sizeof(vqs));
}

/**
 * i40evf_map_queues
 * @adapter: adapter structure
 *
 * Request that the PF map queues to interrupt vectors. Misc causes, including
 * admin queue, are always mapped to vector 0.
 **/
void i40evf_map_queues(struct i40evf_adapter *adapter)
{
	struct virtchnl_irq_map_info *vimi;
	int v_idx, q_vectors, len;
	struct i40e_q_vector *q_vector;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot map queues to vectors, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_CONFIG_IRQ_MAP;

	q_vectors = adapter->num_msix_vectors - NONQ_VECS;

	len = sizeof(struct virtchnl_irq_map_info) +
	      (adapter->num_msix_vectors *
<<<<<<< HEAD
		sizeof(struct i40e_virtchnl_vector_map));
=======
		sizeof(struct virtchnl_vector_map));
>>>>>>> temp
	vimi = kzalloc(len, GFP_KERNEL);
	if (!vimi)
		return;

	vimi->num_vectors = adapter->num_msix_vectors;
	/* Queue vectors first */
	for (v_idx = 0; v_idx < q_vectors; v_idx++) {
		q_vector = adapter->q_vectors + v_idx;
		vimi->vecmap[v_idx].vsi_id = adapter->vsi_res->vsi_id;
		vimi->vecmap[v_idx].vector_id = v_idx + NONQ_VECS;
		vimi->vecmap[v_idx].txq_map = q_vector->ring_mask;
		vimi->vecmap[v_idx].rxq_map = q_vector->ring_mask;
	}
	/* Misc vector last - this is only for AdminQ messages */
	vimi->vecmap[v_idx].vsi_id = adapter->vsi_res->vsi_id;
	vimi->vecmap[v_idx].vector_id = 0;
	vimi->vecmap[v_idx].txq_map = 0;
	vimi->vecmap[v_idx].rxq_map = 0;

	adapter->aq_required &= ~I40EVF_FLAG_AQ_MAP_VECTORS;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_CONFIG_IRQ_MAP,
			   (u8 *)vimi, len);
	kfree(vimi);
}

/**
 * i40evf_request_queues
 * @adapter: adapter structure
 * @num: number of requested queues
 *
 * We get a default number of queues from the PF.  This enables us to request a
 * different number.  Returns 0 on success, negative on failure
 **/
int i40evf_request_queues(struct i40evf_adapter *adapter, int num)
{
	struct virtchnl_vf_res_request vfres;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot request queues, command %d pending\n",
			adapter->current_op);
		return -EBUSY;
	}

	vfres.num_queue_pairs = num;

	adapter->current_op = VIRTCHNL_OP_REQUEST_QUEUES;
	adapter->flags |= I40EVF_FLAG_REINIT_ITR_NEEDED;
	return i40evf_send_pf_msg(adapter, VIRTCHNL_OP_REQUEST_QUEUES,
				  (u8 *)&vfres, sizeof(vfres));
}

/**
 * i40evf_add_ether_addrs
 * @adapter: adapter structure
 * @addrs: the MAC address filters to add (contiguous)
 * @count: number of filters
 *
 * Request that the PF add one or more addresses to our filters.
 **/
void i40evf_add_ether_addrs(struct i40evf_adapter *adapter)
{
	struct virtchnl_ether_addr_list *veal;
	int len, i = 0, count = 0;
	struct i40evf_mac_filter *f;
	bool more = false;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot add filters, command %d pending\n",
			adapter->current_op);
		return;
	}
	list_for_each_entry(f, &adapter->mac_filter_list, list) {
		if (f->add)
			count++;
	}
	if (!count) {
		adapter->aq_required &= ~I40EVF_FLAG_AQ_ADD_MAC_FILTER;
		return;
	}
	adapter->current_op = VIRTCHNL_OP_ADD_ETH_ADDR;

	len = sizeof(struct virtchnl_ether_addr_list) +
	      (count * sizeof(struct virtchnl_ether_addr));
	if (len > I40EVF_MAX_AQ_BUF_SIZE) {
		dev_warn(&adapter->pdev->dev, "Too many add MAC changes in one request\n");
		count = (I40EVF_MAX_AQ_BUF_SIZE -
<<<<<<< HEAD
			 sizeof(struct i40e_virtchnl_ether_addr_list)) /
			sizeof(struct i40e_virtchnl_ether_addr);
		len = sizeof(struct i40e_virtchnl_ether_addr_list) +
		      (count * sizeof(struct i40e_virtchnl_ether_addr));
=======
			 sizeof(struct virtchnl_ether_addr_list)) /
			sizeof(struct virtchnl_ether_addr);
		len = sizeof(struct virtchnl_ether_addr_list) +
		      (count * sizeof(struct virtchnl_ether_addr));
>>>>>>> temp
		more = true;
	}

	veal = kzalloc(len, GFP_KERNEL);
	if (!veal)
		return;

	veal->vsi_id = adapter->vsi_res->vsi_id;
	veal->num_elements = count;
	list_for_each_entry(f, &adapter->mac_filter_list, list) {
		if (f->add) {
			ether_addr_copy(veal->list[i].addr, f->macaddr);
			i++;
			f->add = false;
			if (i == count)
				break;
		}
	}
	if (!more)
		adapter->aq_required &= ~I40EVF_FLAG_AQ_ADD_MAC_FILTER;
<<<<<<< HEAD
	i40evf_send_pf_msg(adapter, I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS,
=======
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_ADD_ETH_ADDR,
>>>>>>> temp
			   (u8 *)veal, len);
	kfree(veal);
}

/**
 * i40evf_del_ether_addrs
 * @adapter: adapter structure
 * @addrs: the MAC address filters to remove (contiguous)
 * @count: number of filtes
 *
 * Request that the PF remove one or more addresses from our filters.
 **/
void i40evf_del_ether_addrs(struct i40evf_adapter *adapter)
{
	struct virtchnl_ether_addr_list *veal;
	struct i40evf_mac_filter *f, *ftmp;
	int len, i = 0, count = 0;
	bool more = false;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot remove filters, command %d pending\n",
			adapter->current_op);
		return;
	}
	list_for_each_entry(f, &adapter->mac_filter_list, list) {
		if (f->remove)
			count++;
	}
	if (!count) {
		adapter->aq_required &= ~I40EVF_FLAG_AQ_DEL_MAC_FILTER;
		return;
	}
	adapter->current_op = VIRTCHNL_OP_DEL_ETH_ADDR;

	len = sizeof(struct virtchnl_ether_addr_list) +
	      (count * sizeof(struct virtchnl_ether_addr));
	if (len > I40EVF_MAX_AQ_BUF_SIZE) {
		dev_warn(&adapter->pdev->dev, "Too many delete MAC changes in one request\n");
		count = (I40EVF_MAX_AQ_BUF_SIZE -
<<<<<<< HEAD
			 sizeof(struct i40e_virtchnl_ether_addr_list)) /
			sizeof(struct i40e_virtchnl_ether_addr);
		len = sizeof(struct i40e_virtchnl_ether_addr_list) +
		      (count * sizeof(struct i40e_virtchnl_ether_addr));
=======
			 sizeof(struct virtchnl_ether_addr_list)) /
			sizeof(struct virtchnl_ether_addr);
		len = sizeof(struct virtchnl_ether_addr_list) +
		      (count * sizeof(struct virtchnl_ether_addr));
>>>>>>> temp
		more = true;
	}
	veal = kzalloc(len, GFP_KERNEL);
	if (!veal)
		return;

	veal->vsi_id = adapter->vsi_res->vsi_id;
	veal->num_elements = count;
	list_for_each_entry_safe(f, ftmp, &adapter->mac_filter_list, list) {
		if (f->remove) {
			ether_addr_copy(veal->list[i].addr, f->macaddr);
			i++;
			list_del(&f->list);
			kfree(f);
			if (i == count)
				break;
		}
	}
	if (!more)
		adapter->aq_required &= ~I40EVF_FLAG_AQ_DEL_MAC_FILTER;
<<<<<<< HEAD
	i40evf_send_pf_msg(adapter, I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS,
=======
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_DEL_ETH_ADDR,
>>>>>>> temp
			   (u8 *)veal, len);
	kfree(veal);
}

/**
 * i40evf_add_vlans
 * @adapter: adapter structure
 * @vlans: the VLANs to add
 * @count: number of VLANs
 *
 * Request that the PF add one or more VLAN filters to our VSI.
 **/
void i40evf_add_vlans(struct i40evf_adapter *adapter)
{
	struct virtchnl_vlan_filter_list *vvfl;
	int len, i = 0, count = 0;
	struct i40evf_vlan_filter *f;
	bool more = false;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot add VLANs, command %d pending\n",
			adapter->current_op);
		return;
	}

	list_for_each_entry(f, &adapter->vlan_filter_list, list) {
		if (f->add)
			count++;
	}
	if (!count) {
		adapter->aq_required &= ~I40EVF_FLAG_AQ_ADD_VLAN_FILTER;
		return;
	}
	adapter->current_op = VIRTCHNL_OP_ADD_VLAN;

	len = sizeof(struct virtchnl_vlan_filter_list) +
	      (count * sizeof(u16));
	if (len > I40EVF_MAX_AQ_BUF_SIZE) {
		dev_warn(&adapter->pdev->dev, "Too many add VLAN changes in one request\n");
		count = (I40EVF_MAX_AQ_BUF_SIZE -
			 sizeof(struct virtchnl_vlan_filter_list)) /
			sizeof(u16);
<<<<<<< HEAD
		len = sizeof(struct i40e_virtchnl_vlan_filter_list) +
=======
		len = sizeof(struct virtchnl_vlan_filter_list) +
>>>>>>> temp
		      (count * sizeof(u16));
		more = true;
	}
	vvfl = kzalloc(len, GFP_KERNEL);
	if (!vvfl)
		return;

	vvfl->vsi_id = adapter->vsi_res->vsi_id;
	vvfl->num_elements = count;
	list_for_each_entry(f, &adapter->vlan_filter_list, list) {
		if (f->add) {
			vvfl->vlan_id[i] = f->vlan;
			i++;
			f->add = false;
			if (i == count)
				break;
		}
	}
	if (!more)
		adapter->aq_required &= ~I40EVF_FLAG_AQ_ADD_VLAN_FILTER;
<<<<<<< HEAD
	i40evf_send_pf_msg(adapter, I40E_VIRTCHNL_OP_ADD_VLAN, (u8 *)vvfl, len);
=======
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_ADD_VLAN, (u8 *)vvfl, len);
>>>>>>> temp
	kfree(vvfl);
}

/**
 * i40evf_del_vlans
 * @adapter: adapter structure
 * @vlans: the VLANs to remove
 * @count: number of VLANs
 *
 * Request that the PF remove one or more VLAN filters from our VSI.
 **/
void i40evf_del_vlans(struct i40evf_adapter *adapter)
{
	struct virtchnl_vlan_filter_list *vvfl;
	struct i40evf_vlan_filter *f, *ftmp;
	int len, i = 0, count = 0;
	bool more = false;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot remove VLANs, command %d pending\n",
			adapter->current_op);
		return;
	}

	list_for_each_entry(f, &adapter->vlan_filter_list, list) {
		if (f->remove)
			count++;
	}
	if (!count) {
		adapter->aq_required &= ~I40EVF_FLAG_AQ_DEL_VLAN_FILTER;
		return;
	}
	adapter->current_op = VIRTCHNL_OP_DEL_VLAN;

	len = sizeof(struct virtchnl_vlan_filter_list) +
	      (count * sizeof(u16));
	if (len > I40EVF_MAX_AQ_BUF_SIZE) {
		dev_warn(&adapter->pdev->dev, "Too many delete VLAN changes in one request\n");
		count = (I40EVF_MAX_AQ_BUF_SIZE -
			 sizeof(struct virtchnl_vlan_filter_list)) /
			sizeof(u16);
<<<<<<< HEAD
		len = sizeof(struct i40e_virtchnl_vlan_filter_list) +
=======
		len = sizeof(struct virtchnl_vlan_filter_list) +
>>>>>>> temp
		      (count * sizeof(u16));
		more = true;
	}
	vvfl = kzalloc(len, GFP_KERNEL);
	if (!vvfl)
		return;

	vvfl->vsi_id = adapter->vsi_res->vsi_id;
	vvfl->num_elements = count;
	list_for_each_entry_safe(f, ftmp, &adapter->vlan_filter_list, list) {
		if (f->remove) {
			vvfl->vlan_id[i] = f->vlan;
			i++;
			list_del(&f->list);
			kfree(f);
			if (i == count)
				break;
		}
	}
	if (!more)
		adapter->aq_required &= ~I40EVF_FLAG_AQ_DEL_VLAN_FILTER;
<<<<<<< HEAD
	i40evf_send_pf_msg(adapter, I40E_VIRTCHNL_OP_DEL_VLAN, (u8 *)vvfl, len);
=======
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_DEL_VLAN, (u8 *)vvfl, len);
>>>>>>> temp
	kfree(vvfl);
}

/**
 * i40evf_set_promiscuous
 * @adapter: adapter structure
 * @flags: bitmask to control unicast/multicast promiscuous.
 *
 * Request that the PF enable promiscuous mode for our VSI.
 **/
void i40evf_set_promiscuous(struct i40evf_adapter *adapter, int flags)
{
	struct virtchnl_promisc_info vpi;
	int promisc_all;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot set promiscuous mode, command %d pending\n",
			adapter->current_op);
		return;
	}

	promisc_all = FLAG_VF_UNICAST_PROMISC |
		      FLAG_VF_MULTICAST_PROMISC;
	if ((flags & promisc_all) == promisc_all) {
		adapter->flags |= I40EVF_FLAG_PROMISC_ON;
		adapter->aq_required &= ~I40EVF_FLAG_AQ_REQUEST_PROMISC;
		dev_info(&adapter->pdev->dev, "Entering promiscuous mode\n");
	}

	if (flags & FLAG_VF_MULTICAST_PROMISC) {
		adapter->flags |= I40EVF_FLAG_ALLMULTI_ON;
		adapter->aq_required &= ~I40EVF_FLAG_AQ_REQUEST_ALLMULTI;
		dev_info(&adapter->pdev->dev, "Entering multicast promiscuous mode\n");
	}

	if (!flags) {
		adapter->flags &= ~I40EVF_FLAG_PROMISC_ON;
		adapter->aq_required &= ~I40EVF_FLAG_AQ_RELEASE_PROMISC;
		dev_info(&adapter->pdev->dev, "Leaving promiscuous mode\n");
	}

	adapter->current_op = VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE;
	vpi.vsi_id = adapter->vsi_res->vsi_id;
	vpi.flags = flags;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_CONFIG_PROMISCUOUS_MODE,
			   (u8 *)&vpi, sizeof(vpi));
}

/**
 * i40evf_request_stats
 * @adapter: adapter structure
 *
 * Request VSI statistics from PF.
 **/
void i40evf_request_stats(struct i40evf_adapter *adapter)
{
	struct virtchnl_queue_select vqs;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* no error message, this isn't crucial */
		return;
	}
	adapter->current_op = VIRTCHNL_OP_GET_STATS;
	vqs.vsi_id = adapter->vsi_res->vsi_id;
	/* queue maps are ignored for this message - only the vsi is used */
	if (i40evf_send_pf_msg(adapter, VIRTCHNL_OP_GET_STATS,
			       (u8 *)&vqs, sizeof(vqs)))
		/* if the request failed, don't lock out others */
		adapter->current_op = VIRTCHNL_OP_UNKNOWN;
}

/**
 * i40evf_get_hena
 * @adapter: adapter structure
 *
 * Request hash enable capabilities from PF
 **/
void i40evf_get_hena(struct i40evf_adapter *adapter)
{
	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot get RSS hash capabilities, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_GET_RSS_HENA_CAPS;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_GET_HENA;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_GET_RSS_HENA_CAPS,
			   NULL, 0);
}

/**
 * i40evf_set_hena
 * @adapter: adapter structure
 *
 * Request the PF to set our RSS hash capabilities
 **/
void i40evf_set_hena(struct i40evf_adapter *adapter)
{
	struct virtchnl_rss_hena vrh;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot set RSS hash enable, command %d pending\n",
			adapter->current_op);
		return;
	}
	vrh.hena = adapter->hena;
	adapter->current_op = VIRTCHNL_OP_SET_RSS_HENA;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_SET_HENA;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_SET_RSS_HENA,
			   (u8 *)&vrh, sizeof(vrh));
}

/**
 * i40evf_set_rss_key
 * @adapter: adapter structure
 *
 * Request the PF to set our RSS hash key
 **/
void i40evf_set_rss_key(struct i40evf_adapter *adapter)
{
	struct virtchnl_rss_key *vrk;
	int len;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot set RSS key, command %d pending\n",
			adapter->current_op);
		return;
	}
	len = sizeof(struct virtchnl_rss_key) +
	      (adapter->rss_key_size * sizeof(u8)) - 1;
	vrk = kzalloc(len, GFP_KERNEL);
	if (!vrk)
		return;
	vrk->vsi_id = adapter->vsi.id;
	vrk->key_len = adapter->rss_key_size;
	memcpy(vrk->key, adapter->rss_key, adapter->rss_key_size);

	adapter->current_op = VIRTCHNL_OP_CONFIG_RSS_KEY;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_SET_RSS_KEY;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_CONFIG_RSS_KEY,
			   (u8 *)vrk, len);
	kfree(vrk);
}

/**
 * i40evf_set_rss_lut
 * @adapter: adapter structure
 *
 * Request the PF to set our RSS lookup table
 **/
void i40evf_set_rss_lut(struct i40evf_adapter *adapter)
{
	struct virtchnl_rss_lut *vrl;
	int len;

	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot set RSS LUT, command %d pending\n",
			adapter->current_op);
		return;
	}
	len = sizeof(struct virtchnl_rss_lut) +
	      (adapter->rss_lut_size * sizeof(u8)) - 1;
	vrl = kzalloc(len, GFP_KERNEL);
	if (!vrl)
		return;
	vrl->vsi_id = adapter->vsi.id;
	vrl->lut_entries = adapter->rss_lut_size;
	memcpy(vrl->lut, adapter->rss_lut, adapter->rss_lut_size);
	adapter->current_op = VIRTCHNL_OP_CONFIG_RSS_LUT;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_SET_RSS_LUT;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_CONFIG_RSS_LUT,
			   (u8 *)vrl, len);
	kfree(vrl);
}

/**
 * i40evf_enable_vlan_stripping
 * @adapter: adapter structure
 *
 * Request VLAN header stripping to be enabled
 **/
void i40evf_enable_vlan_stripping(struct i40evf_adapter *adapter)
{
	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot enable stripping, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_ENABLE_VLAN_STRIPPING;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_ENABLE_VLAN_STRIPPING;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_ENABLE_VLAN_STRIPPING,
			   NULL, 0);
}

/**
 * i40evf_disable_vlan_stripping
 * @adapter: adapter structure
 *
 * Request VLAN header stripping to be disabled
 **/
void i40evf_disable_vlan_stripping(struct i40evf_adapter *adapter)
{
	if (adapter->current_op != VIRTCHNL_OP_UNKNOWN) {
		/* bail because we already have a command pending */
		dev_err(&adapter->pdev->dev, "Cannot disable stripping, command %d pending\n",
			adapter->current_op);
		return;
	}
	adapter->current_op = VIRTCHNL_OP_DISABLE_VLAN_STRIPPING;
	adapter->aq_required &= ~I40EVF_FLAG_AQ_DISABLE_VLAN_STRIPPING;
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_DISABLE_VLAN_STRIPPING,
			   NULL, 0);
}

/**
 * i40evf_print_link_message - print link up or down
 * @adapter: adapter structure
 *
 * Log a message telling the world of our wonderous link status
 */
static void i40evf_print_link_message(struct i40evf_adapter *adapter)
{
	struct net_device *netdev = adapter->netdev;
	char *speed = "Unknown ";

	if (!adapter->link_up) {
		netdev_info(netdev, "NIC Link is Down\n");
		return;
	}

	switch (adapter->link_speed) {
	case I40E_LINK_SPEED_40GB:
		speed = "40 G";
		break;
	case I40E_LINK_SPEED_25GB:
		speed = "25 G";
		break;
	case I40E_LINK_SPEED_20GB:
		speed = "20 G";
		break;
	case I40E_LINK_SPEED_10GB:
		speed = "10 G";
		break;
	case I40E_LINK_SPEED_1GB:
		speed = "1000 M";
		break;
	case I40E_LINK_SPEED_100MB:
		speed = "100 M";
		break;
	default:
		break;
	}

	netdev_info(netdev, "NIC Link is Up %sbps Full Duplex\n", speed);
}

/**
 * i40evf_request_reset
 * @adapter: adapter structure
 *
 * Request that the PF reset this VF. No response is expected.
 **/
void i40evf_request_reset(struct i40evf_adapter *adapter)
{
	/* Don't check CURRENT_OP - this is always higher priority */
	i40evf_send_pf_msg(adapter, VIRTCHNL_OP_RESET_VF, NULL, 0);
	adapter->current_op = VIRTCHNL_OP_UNKNOWN;
}

/**
 * i40evf_virtchnl_completion
 * @adapter: adapter structure
 * @v_opcode: opcode sent by PF
 * @v_retval: retval sent by PF
 * @msg: message sent by PF
 * @msglen: message length
 *
 * Asynchronous completion function for admin queue messages. Rather than busy
 * wait, we fire off our requests and assume that no errors will be returned.
 * This function handles the reply messages.
 **/
void i40evf_virtchnl_completion(struct i40evf_adapter *adapter,
				enum virtchnl_ops v_opcode,
				i40e_status v_retval,
				u8 *msg, u16 msglen)
{
	struct net_device *netdev = adapter->netdev;

	if (v_opcode == VIRTCHNL_OP_EVENT) {
		struct virtchnl_pf_event *vpe =
			(struct virtchnl_pf_event *)msg;
		bool link_up = vpe->event_data.link_event.link_status;
		switch (vpe->event) {
		case VIRTCHNL_EVENT_LINK_CHANGE:
			adapter->link_speed =
				vpe->event_data.link_event.link_speed;

			/* we've already got the right link status, bail */
			if (adapter->link_up == link_up)
				break;

			/* If we get link up message and start queues before
			 * our queues are configured it will trigger a TX hang.
			 * In that case, just ignore the link status message,
			 * we'll get another one after we enable queues and
			 * actually prepared to send traffic.
			 */
			if (link_up && adapter->state != __I40EVF_RUNNING)
				break;

			adapter->link_up = link_up;
			if (link_up) {
				netif_tx_start_all_queues(netdev);
				netif_carrier_on(netdev);
			} else {
				netif_tx_stop_all_queues(netdev);
				netif_carrier_off(netdev);
			}
			i40evf_print_link_message(adapter);
			break;
		case VIRTCHNL_EVENT_RESET_IMPENDING:
			dev_info(&adapter->pdev->dev, "PF reset warning received\n");
			if (!(adapter->flags & I40EVF_FLAG_RESET_PENDING)) {
				adapter->flags |= I40EVF_FLAG_RESET_PENDING;
				dev_info(&adapter->pdev->dev, "Scheduling reset task\n");
				schedule_work(&adapter->reset_task);
			}
			break;
		default:
			dev_err(&adapter->pdev->dev, "Unknown event %d from PF\n",
				vpe->event);
			break;
		}
		return;
	}
	if (v_retval) {
		switch (v_opcode) {
<<<<<<< HEAD
		case I40E_VIRTCHNL_OP_ADD_VLAN:
			dev_err(&adapter->pdev->dev, "Failed to add VLAN filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case I40E_VIRTCHNL_OP_ADD_ETHER_ADDRESS:
			dev_err(&adapter->pdev->dev, "Failed to add MAC filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case I40E_VIRTCHNL_OP_DEL_VLAN:
			dev_err(&adapter->pdev->dev, "Failed to delete VLAN filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case I40E_VIRTCHNL_OP_DEL_ETHER_ADDRESS:
=======
		case VIRTCHNL_OP_ADD_VLAN:
			dev_err(&adapter->pdev->dev, "Failed to add VLAN filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case VIRTCHNL_OP_ADD_ETH_ADDR:
			dev_err(&adapter->pdev->dev, "Failed to add MAC filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case VIRTCHNL_OP_DEL_VLAN:
			dev_err(&adapter->pdev->dev, "Failed to delete VLAN filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		case VIRTCHNL_OP_DEL_ETH_ADDR:
>>>>>>> temp
			dev_err(&adapter->pdev->dev, "Failed to delete MAC filter, error %s\n",
				i40evf_stat_str(&adapter->hw, v_retval));
			break;
		default:
			dev_err(&adapter->pdev->dev, "PF returned error %d (%s) to our request %d\n",
				v_retval,
				i40evf_stat_str(&adapter->hw, v_retval),
				v_opcode);
		}
	}
	switch (v_opcode) {
	case VIRTCHNL_OP_GET_STATS: {
		struct i40e_eth_stats *stats =
			(struct i40e_eth_stats *)msg;
		netdev->stats.rx_packets = stats->rx_unicast +
					   stats->rx_multicast +
					   stats->rx_broadcast;
		netdev->stats.tx_packets = stats->tx_unicast +
					   stats->tx_multicast +
					   stats->tx_broadcast;
		netdev->stats.rx_bytes = stats->rx_bytes;
		netdev->stats.tx_bytes = stats->tx_bytes;
		netdev->stats.tx_errors = stats->tx_errors;
		netdev->stats.rx_dropped = stats->rx_discards;
		netdev->stats.tx_dropped = stats->tx_discards;
		adapter->current_stats = *stats;
		}
		break;
	case VIRTCHNL_OP_GET_VF_RESOURCES: {
		u16 len = sizeof(struct virtchnl_vf_resource) +
			  I40E_MAX_VF_VSI *
			  sizeof(struct virtchnl_vsi_resource);
		memcpy(adapter->vf_res, msg, min(msglen, len));
		i40e_vf_parse_hw_config(&adapter->hw, adapter->vf_res);
		/* restore current mac address */
		ether_addr_copy(adapter->hw.mac.addr, netdev->dev_addr);
		i40evf_process_config(adapter);
		}
		break;
	case VIRTCHNL_OP_ENABLE_QUEUES:
		/* enable transmits */
		i40evf_irq_enable(adapter, true);
		break;
	case VIRTCHNL_OP_DISABLE_QUEUES:
		i40evf_free_all_tx_resources(adapter);
		i40evf_free_all_rx_resources(adapter);
<<<<<<< HEAD
		if (adapter->state == __I40EVF_DOWN_PENDING)
			adapter->state = __I40EVF_DOWN;
=======
		if (adapter->state == __I40EVF_DOWN_PENDING) {
			adapter->state = __I40EVF_DOWN;
			wake_up(&adapter->down_waitqueue);
		}
>>>>>>> temp
		break;
	case VIRTCHNL_OP_VERSION:
	case VIRTCHNL_OP_CONFIG_IRQ_MAP:
		/* Don't display an error if we get these out of sequence.
		 * If the firmware needed to get kicked, we'll get these and
		 * it's no problem.
		 */
		if (v_opcode != adapter->current_op)
			return;
		break;
	case VIRTCHNL_OP_IWARP:
		/* Gobble zero-length replies from the PF. They indicate that
		 * a previous message was received OK, and the client doesn't
		 * care about that.
		 */
		if (msglen && CLIENT_ENABLED(adapter))
			i40evf_notify_client_message(&adapter->vsi,
						     msg, msglen);
		break;

	case VIRTCHNL_OP_CONFIG_IWARP_IRQ_MAP:
		adapter->client_pending &=
				~(BIT(VIRTCHNL_OP_CONFIG_IWARP_IRQ_MAP));
		break;
	case VIRTCHNL_OP_GET_RSS_HENA_CAPS: {
		struct virtchnl_rss_hena *vrh = (struct virtchnl_rss_hena *)msg;
		if (msglen == sizeof(*vrh))
			adapter->hena = vrh->hena;
		else
			dev_warn(&adapter->pdev->dev,
				 "Invalid message %d from PF\n", v_opcode);
		}
		break;
	case VIRTCHNL_OP_REQUEST_QUEUES: {
		struct virtchnl_vf_res_request *vfres =
			(struct virtchnl_vf_res_request *)msg;
		if (vfres->num_queue_pairs != adapter->num_req_queues) {
			dev_info(&adapter->pdev->dev,
				 "Requested %d queues, PF can support %d\n",
				 adapter->num_req_queues,
				 vfres->num_queue_pairs);
			adapter->num_req_queues = 0;
			adapter->flags &= ~I40EVF_FLAG_REINIT_ITR_NEEDED;
		}
		}
		break;
	default:
		if (adapter->current_op && (v_opcode != adapter->current_op))
			dev_warn(&adapter->pdev->dev, "Expected response %d from PF, received %d\n",
				 adapter->current_op, v_opcode);
		break;
	} /* switch v_opcode */
	adapter->current_op = VIRTCHNL_OP_UNKNOWN;
}
