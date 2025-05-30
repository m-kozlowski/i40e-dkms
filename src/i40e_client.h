 /* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2013-2025 Intel Corporation */

#ifndef _I40E_CLIENT_H_
#define _I40E_CLIENT_H_

#ifdef USE_INTEL_AUX_BUS
#include "linux/auxiliary_bus.h"
#else
#include <linux/auxiliary_bus.h>
#endif /* USE_INTEL_AUX_BUS */

#define I40E_PEER_RDMA_NAME    "i40e_rdma"
#define I40E_PEER_RDMA_ID      PLATFORM_DEVID_AUTO

#define I40E_CLIENT_STR_LENGTH 10
/* Client interface version should be updated anytime there is a change in the
 * existing APIs or data structures.
 * Note that in-tree i40e has major version of 0 and the structures are not
 * identical. For example struct i40e_info and i40e_ops are different.
 */
#define I40E_CLIENT_VERSION_MAJOR 1
#define I40E_CLIENT_VERSION_MINOR 01
#define I40E_CLIENT_VERSION_BUILD 00
#define I40E_CLIENT_VERSION_STR     \
	__stringify(I40E_CLIENT_VERSION_MAJOR) "." \
	__stringify(I40E_CLIENT_VERSION_MINOR) "." \
	__stringify(I40E_CLIENT_VERSION_BUILD)

struct i40e_client_version {
	u8 major;
	u8 minor;
	u8 build;
	u8 rsvd;
};

enum i40e_client_state {
	__I40E_CLIENT_NULL,
	__I40E_CLIENT_REGISTERED
};

enum i40e_client_instance_state {
	__I40E_CLIENT_INSTANCE_NONE,
	__I40E_CLIENT_INSTANCE_OPENED,
};

struct i40e_ops;
struct i40e_client;

/* HW does not define a type value for AEQ; only for RX/TX and CEQ.
 * In order for us to keep the interface simple, SW will define a
 * unique type value for AEQ.
 */
#define I40E_QUEUE_TYPE_PE_AEQ  0x80
#define I40E_QUEUE_INVALID_IDX	0xFFFF

struct i40e_qv_info {
	u32 v_idx; /* msix_vector */
	u16 ceq_idx;
	u16 aeq_idx;
	u8 itr_idx;
};

struct i40e_qvlist_info {
	u32 num_vectors;
	struct i40e_qv_info qv_info[1];
};

#define I40E_CLIENT_MSIX_ALL 0xFFFFFFFF

/* set of LAN parameters useful for clients managed by LAN */

/* Struct to hold per priority info */
struct i40e_prio_qos_params {
	u16 qs_handle; /* qs handle for prio */
	u8 tc; /* TC mapped to prio */
	u8 reserved;
};

#define I40E_CLIENT_MAX_USER_PRIORITY        8
/* Struct to hold Client QoS */
struct i40e_qos_params {
	struct i40e_prio_qos_params prio_qos[I40E_CLIENT_MAX_USER_PRIORITY];
};

struct i40e_params {
	struct i40e_qos_params qos;
	u16 mtu;
};

/* Structure to hold Lan device info for a client device */
struct i40e_info {
	struct i40e_client_version version;
	u8 lanmac[6];
	struct net_device *netdev;
	struct pci_dev *pcidev;
	struct auxiliary_device *aux_dev;
	u8 __iomem *hw_addr;
	u8 fid;	/* function id, PF id or VF id */
#define I40E_CLIENT_FTYPE_PF 0
#define I40E_CLIENT_FTYPE_VF 1
	u8 ftype; /* function type, PF or VF */
	void *pf;

	/* All L2 params that could change during the life span of the PF
	 * and needs to be communicated to the client when they change
	 */
	struct i40e_qvlist_info *qvlist_info;
	struct i40e_params params;
	struct i40e_ops *ops;

	u16 msix_count;	 /* number of msix vectors*/
	/* Array down below will be dynamically allocated based on msix_count */
	struct msix_entry *msix_entries;
	u16 itr_index; /* Which ITR index the PE driver is suppose to use */
	u16 fw_maj_ver;                 /* firmware major version */
	u16 fw_min_ver;                 /* firmware minor version */
	u32 fw_build;                   /* firmware build number */
	struct i40e_client *client;
};

struct i40e_auxiliary_device {
	struct auxiliary_device aux_dev;
	struct i40e_info *ldev;
};

#define I40E_CLIENT_RESET_LEVEL_PF   1
#define I40E_CLIENT_RESET_LEVEL_CORE 2
#define I40E_CLIENT_VSI_FLAG_TCP_ENABLE  BIT(1)

struct i40e_ops {
	/* setup_q_vector_list enables queues with a particular vector */
	int (*setup_qvlist)(struct i40e_info *ldev, struct i40e_client *client,
			    struct i40e_qvlist_info *qv_info);

	int (*virtchnl_send)(struct i40e_info *ldev, struct i40e_client *client,
			     u32 vf_id, u8 *msg, u16 len);

	/* If the PE Engine is unresponsive, RDMA driver can request a reset.
	 * The level helps determine the level of reset being requested.
	 */
	void (*request_reset)(struct i40e_info *ldev,
			      struct i40e_client *client, u32 level);

	/* API for the RDMA driver to set certain VSI flags that control
	 * PE Engine.
	 */
	int (*update_vsi_ctxt)(struct i40e_info *ldev,
			       struct i40e_client *client,
			       bool is_vf, u32 vf_id,
			       u32 flag, u32 valid_flag);

	int (*client_device_register)(struct i40e_info *ldev);

	void (*client_device_unregister)(struct i40e_info *ldev);
};

struct i40e_client_ops {
	/* Should be called from register_client() or whenever PF is ready
	 * to create a specific client instance.
	 */
	int (*open)(struct i40e_info *ldev, struct i40e_client *client);

	/* Should be called when netdev is unavailable or when unregister
	 * call comes in. If the close is happenening due to a reset being
	 * triggered set the reset bit to true.
	 */
	void (*close)(struct i40e_info *ldev, struct i40e_client *client,
		      bool reset);

	/* called when some l2 managed parameters changes - mtu */
	void (*l2_param_change)(struct i40e_info *ldev,
				struct i40e_client *client,
				struct i40e_params *params);

	int (*virtchnl_receive)(struct i40e_info *ldev,
				struct i40e_client *client, u32 vf_id,
				u8 *msg, u16 len);

	/* called when a VF is reset by the PF */
	void (*vf_reset)(struct i40e_info *ldev,
			 struct i40e_client *client, u32 vf_id);

	/* called when the number of VFs changes */
	void (*vf_enable)(struct i40e_info *ldev,
			  struct i40e_client *client, u32 num_vfs);

	/* returns true if VF is capable of specified offload */
	int (*vf_capable)(struct i40e_info *ldev,
			  struct i40e_client *client, u32 vf_id);
};

/* Client device */
struct i40e_client_instance {
	struct list_head list;
	struct i40e_info lan_info;
	struct i40e_client *client;
	unsigned long  state;
};

struct i40e_client {
	struct list_head list;		/* list of registered clients */
	char name[I40E_CLIENT_STR_LENGTH];
	struct i40e_client_version version;
	unsigned long state;		/* client state */
	atomic_t ref_cnt;  /* Count of all the client devices of this kind */
	u32 flags;
#define I40E_CLIENT_FLAGS_LAUNCH_ON_PROBE	BIT(0)
#define I40E_TX_FLAGS_NOTIFY_OTHER_EVENTS	BIT(2)
	u8 type;
#define I40E_CLIENT_IWARP 0
	/* client ops provided by the client */
	const struct i40e_client_ops *ops;
};

static inline bool i40e_client_is_registered(struct i40e_client *client)
{
	return test_bit(__I40E_CLIENT_REGISTERED, &client->state);
}

#endif /* _I40E_CLIENT_H_ */

