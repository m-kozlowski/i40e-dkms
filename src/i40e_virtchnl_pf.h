 /* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (C) 2013-2025 Intel Corporation */

#ifndef _I40E_VIRTCHNL_PF_H_
#define _I40E_VIRTCHNL_PF_H_

#include "i40e.h"

#define I40E_MAX_VLANID 4095

#define I40E_VIRTCHNL_SUPPORTED_QTYPES 2

#define I40E_DEFAULT_NUM_MDD_EVENTS_ALLOWED	3

#define I40E_VLAN_PRIORITY_SHIFT	13
#define I40E_VLAN_MASK			0xFFF
#define I40E_PRIORITY_MASK		0xE000

#define I40E_MAX_VF_PROMISC_FLAGS	3

#define I40E_VF_STATE_WAIT_COUNT	20
#define I40E_VFR_WAIT_COUNT		100
#define I40E_VF_RESET_TIME_MIN		30000000	// time in nsec

/* Various queue ctrls */
enum i40e_queue_ctrl {
	I40E_QUEUE_CTRL_UNKNOWN = 0,
	I40E_QUEUE_CTRL_ENABLE,
	I40E_QUEUE_CTRL_ENABLECHECK,
	I40E_QUEUE_CTRL_DISABLE,
	I40E_QUEUE_CTRL_DISABLECHECK,
	I40E_QUEUE_CTRL_FASTDISABLE,
	I40E_QUEUE_CTRL_FASTDISABLECHECK,
};

/* VF states */
enum i40e_vf_states {
	I40E_VF_STATE_INIT = 0,
	I40E_VF_STATE_ACTIVE,
	I40E_VF_STATE_DISABLED,
	I40E_VF_STATE_MC_PROMISC,
	I40E_VF_STATE_UC_PROMISC,
	I40E_VF_STATE_PRE_ENABLE,
	I40E_VF_STATE_RESETTING,
	I40E_VF_STATE_RESOURCES_LOADED,
};

/* VF capabilities */
enum i40e_vf_capabilities {
	I40E_VIRTCHNL_VF_CAP_PRIVILEGE = 0,
	I40E_VIRTCHNL_VF_CAP_L2,
};

/* In ADq, max 4 VSI's can be allocated per VF including primary VF VSI.
 * These variables are used to store indices, id's and number of queues
 * for each VSI including that of primary VF VSI. Each Traffic class is
 * termed as channel and each channel can in-turn have 4 queues which
 * means max 16 queues overall per VF.
 */
struct i40evf_channel {
	u16 vsi_idx; /* index in PF struct for all channel VSIs */
	u16 vsi_id; /* VSI ID used by firmware */
	u16 num_qps; /* number of queue pairs requested by user */
	u64 max_tx_rate; /* bandwidth rate allocation for VSIs */
};

/* used for VLAN list 'vm_vlan_list' by VM for trusted and untrusted VF */
struct i40e_vm_vlan {
	struct list_head list;
	s16 vlan;
	u16 vsi_id;
};

/* used for MAC list 'vm_mac_list' to recognize MACs added by VM */
struct i40e_vm_mac {
	struct list_head list;
	u8 macaddr[ETH_ALEN];
};

/* used for following share for given traffic class by VF*/
struct i40e_vf_tc_info {
	bool applied;
	u8 applied_tc_share[I40E_MAX_TRAFFIC_CLASS];
	u8 requested_tc_share[I40E_MAX_TRAFFIC_CLASS];
	u16 max_tc_tx_rate[I40E_MAX_TRAFFIC_CLASS];
};

struct i40e_time_mac {
	unsigned long time_modified;
	u8 addr[ETH_ALEN];
};

struct i40e_mdd_vf_events {
	u64 count;	/* total count of Rx|Tx events */
	/* count number of the last printed event */
	u64 last_printed;
};

/* VF information structure */
struct i40e_vf {
	struct i40e_pf *pf;

	/* VF id in the PF space */
	s16 vf_id;
	/* all VF vsis connect to the same parent */
	enum i40e_switch_element_types parent_type;
	struct virtchnl_version_info vf_ver;
	u32 driver_caps; /* reported by VF driver */

	/* VF Port Extender (PE) stag if used */
	u16 stag;

	struct virtchnl_ether_addr default_lan_addr;
	struct i40e_time_mac legacy_last_added_umac; /* keeps last added MAC address */
	s16 port_vlan_id;
	bool pf_set_mac;	/* The VMM admin set the VF MAC address */
	bool trusted;
	bool source_pruning;
	u64 reset_timestamp;

	/* VSI indices - actual VSI pointers are maintained in the PF structure
	 * When assigned, these will be non-zero, because VSI 0 is always
	 * the main LAN VSI for the PF.
	 */
	u16 lan_vsi_idx;	/* index into PF struct */
	u16 lan_vsi_id;		/* ID as used by firmware */

	u8 num_queue_pairs;	/* num of qps assigned to VF vsis */
	u8 num_req_queues;	/* num of requested qps */
	/* num of mdd tx and rx events detected */
	struct i40e_mdd_vf_events mdd_rx_events;
	struct i40e_mdd_vf_events mdd_tx_events;

	unsigned long vf_caps;	/* vf's adv. capabilities */
	unsigned long vf_states;	/* vf's runtime states */
	unsigned int tx_rate;	/* Tx bandwidth limit in Mbps */
#ifdef HAVE_NDO_SET_VF_LINK_STATE
	bool link_forced;
	bool link_up;		/* only valid if VF link is forced */
#endif
	bool mac_anti_spoof;
	bool vlan_anti_spoof;
	u16 num_vlan;
	DECLARE_BITMAP(mirror_vlans, VLAN_N_VID);
	u16 vlan_rule_id;
#define I40E_NO_VF_MIRROR	-1
/* assuming vf ids' range is <0..max_supported> */
#define I40E_IS_MIRROR_VLAN_ID_VALID(id) ((id) >= 0)
	u16 ingress_rule_id;
	int ingress_vlan;
	u16 egress_rule_id;
	int egress_vlan;
	DECLARE_BITMAP(trunk_vlans, VLAN_N_VID);
	bool trunk_set_by_pf;
	bool allow_untagged; /* update filters, when changing value */
	bool loopback;
	bool vlan_stripping;
	u8 promisc_mode;
	u8 bw_share;
	bool bw_share_applied; /* true if config is applied to the device */
	bool tc_bw_share_req;
	u8 queue_type;
	bool allow_bcast;
	bool is_disabled_from_host; /* bool for PF ctrl of VF enable/disable */
	/* VLAN list created by VM for trusted and untrusted VF */
	struct list_head vm_vlan_list;
	/* MAC list created by VM */
	struct list_head vm_mac_list;
	/* ADq related variables */
	bool adq_enabled; /* flag to enable adq */
	u8 num_tc;
	struct i40evf_channel ch[I40E_MAX_VF_VSI];
	struct hlist_head cloud_filter_list;
	u16 num_cloud_filters;
	struct i40e_vf_tc_info tc_info;
	struct virtchnl_vlan_caps vlan_v2_caps;
};

void i40e_free_vfs(struct i40e_pf *pf);
#if defined(HAVE_SRIOV_CONFIGURE) || defined(HAVE_RHEL6_SRIOV_CONFIGURE)
int i40e_pci_sriov_configure(struct pci_dev *dev, int num_vfs);
#endif
int i40e_alloc_vfs(struct i40e_pf *pf, u16 num_alloc_vfs);
int i40e_vc_process_vf_msg(struct i40e_pf *pf, s16 vf_id, u32 v_opcode,
			   u32 v_retval, u8 *msg, u16 msglen);
int i40e_vc_process_vflr_event(struct i40e_pf *pf);
void i40e_vc_reset_vf(struct i40e_vf *vf, bool notify_vf);
bool i40e_reset_vf(struct i40e_vf *vf, bool flr);
bool i40e_reset_all_vfs(struct i40e_pf *pf, bool flr);
void i40e_vc_notify_vf_reset(struct i40e_vf *vf);

/* VF configuration related iplink handlers */
int i40e_ndo_set_vf_mac(struct net_device *netdev, int vf_id, u8 *mac);
#ifdef IFLA_VF_VLAN_INFO_MAX
int i40e_ndo_set_vf_port_vlan(struct net_device *netdev, int vf_id,
			      u16 vlan_id, u8 qos, __be16 vlan_proto);
#else
int i40e_ndo_set_vf_port_vlan(struct net_device *netdev,
			      int vf_id, u16 vlan_id, u8 qos);
#endif
#ifdef HAVE_NDO_SET_VF_MIN_MAX_TX_RATE
int i40e_ndo_set_vf_bw(struct net_device *netdev, int vf_id, int min_tx_rate,
		       int max_tx_rate);
#else
int i40e_ndo_set_vf_bw(struct net_device *netdev, int vf_id, int tx_rate);
#endif
#ifdef HAVE_NDO_SET_VF_TRUST
int i40e_ndo_set_vf_trust(struct net_device *netdev, int vf_id, bool setting);
#endif
int i40e_ndo_enable_vf(struct net_device *netdev, int vf_id, bool enable);
#ifdef IFLA_VF_MAX
int i40e_ndo_get_vf_config(struct net_device *netdev,
			   int vf_id, struct ifla_vf_info *ivi);
#ifdef HAVE_NDO_SET_VF_LINK_STATE
int i40e_ndo_set_vf_link_state(struct net_device *netdev, int vf_id, int link);
#endif
#ifdef HAVE_VF_SPOOFCHK_CONFIGURE
int i40e_ndo_set_vf_spoofchk(struct net_device *netdev, int vf_id, bool enable);
#endif
#endif

void i40e_vc_notify_link_state(struct i40e_pf *pf);
void i40e_vc_notify_reset(struct i40e_pf *pf);
void i40e_restore_all_vfs_msi_state(struct pci_dev *pdev);
#ifdef HAVE_VF_STATS
int i40e_get_vf_stats(struct net_device *netdev, int vf_id,
		      struct ifla_vf_stats *vf_stats);
#endif
extern const struct vfd_ops i40e_vfd_ops;

#endif /* _I40E_VIRTCHNL_PF_H_ */
