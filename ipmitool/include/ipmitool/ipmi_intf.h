/*
 * Copyright (c) 2003 Sun Microsystems, Inc.  All Rights Reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistribution of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistribution in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of Sun Microsystems, Inc. or the names of
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * This software is provided "AS IS," without a warranty of any kind.
 * ALL EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED.
 * SUN MICROSYSTEMS, INC. ("SUN") AND ITS LICENSORS SHALL NOT BE LIABLE
 * FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.  IN NO EVENT WILL
 * SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA,
 * OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR
 * PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF
 * LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
 * EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * 
 * You acknowledge that this software is not designed or intended for use
 * in the design, construction, operation or maintenance of any nuclear
 * facility.
 */

#ifndef IPMI_INTF_H
#define IPMI_INTF_H

#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_constants.h>


/*
 * An enumeration that describes every possible session state for
 * an IPMIv2 / RMCP+ session.
 */
enum LANPLUS_SESSION_STATE {
	LANPLUS_STATE_PRESESSION = 0,
	LANPLUS_STATE_OPEN_SESSION_SENT,
	LANPLUS_STATE_OPEN_SESSION_RECEIEVED,
	LANPLUS_STATE_RAKP_1_SENT,
	LANPLUS_STATE_RAKP_2_RECEIVED,
	LANPLUS_STATE_RAKP_3_SENT,
	LANPLUS_STATE_ACTIVE,
	LANPLUS_STATE_CLOSE_SENT,
};


#define IPMI_AUTHCODE_BUFFER_SIZE 16
#define IPMI_SIK_BUFFER_SIZE      20
#define IPMI_KG_BUFFER_SIZE       21 /* key plus null byte */

struct ipmi_session {
	unsigned char hostname[64];
	unsigned char username[16];
	unsigned char authcode[IPMI_AUTHCODE_BUFFER_SIZE];
	unsigned char challenge[16];
	unsigned char authtype;
	unsigned char privlvl;
	int password;
	int port;
	int active;

	uint32_t session_id;

	uint32_t in_seq;
	uint32_t out_seq;

	uint32_t timeout;

	/*
	 * This struct holds state data specific to IMPI v2 / RMCP+ sessions
	 */
	struct {
		enum LANPLUS_SESSION_STATE session_state;

		/* These are the algorithms agreed upon for the session */
		unsigned char auth_alg;
		unsigned char integrity_alg;
		unsigned char crypt_alg;
		unsigned char max_priv_level;

		uint32_t console_id;
		uint32_t bmc_id;

		/*
		 * Values required for RAKP mesages
		 */

		/* Random number generated byt the console */
		unsigned char console_rand[16]; 
		/* Random number generated by the BMC */
		unsigned char bmc_rand[16];

		unsigned char bmc_guid[16];
		unsigned char requested_role;   /* As sent in the RAKP 1 message */
		unsigned char rakp2_return_code;

		unsigned char sik[IPMI_SIK_BUFFER_SIZE]; /* Session integrity key */
		unsigned char kg[IPMI_KG_BUFFER_SIZE];   /* BMC key */
		unsigned char k1[20];   /* Used for Integrity checking? */
		unsigned char k2[20];   /* First 16 bytes used for AES  */
	} v2_data;


	/*
	 * This data is specific to the Serial Over Lan session
	 */
	struct {
		uint16_t max_inbound_payload_size;
		uint16_t max_outbound_payload_size;
		uint16_t port;
		unsigned char sequence_number;

		/*  This data describes the last SOL packet */
		unsigned char last_received_sequence_number;
		unsigned char last_received_byte_count;
		void (*sol_input_handler)(struct ipmi_rs * rsp);
	} sol_data;
};


struct ipmi_intf {
	char name[32];
	int fd;
	int opened;
	int abort;
	int pedantic;
	int (*open)(struct ipmi_intf *);
	void (*close)(struct ipmi_intf *);
	struct ipmi_rs *(*sendrecv)(struct ipmi_intf *, struct ipmi_rq *);
	struct ipmi_rs *(*recv_sol)(struct ipmi_intf *);
	struct ipmi_rs *(*send_sol)(struct ipmi_intf *,
								struct ipmi_v2_payload * payload);
	struct ipmi_session * session;
};

struct static_intf {
	char * name;
	int (*setup)(struct ipmi_intf ** intf);
};

int ipmi_intf_init(void);
void ipmi_intf_exit(void);
struct ipmi_intf * ipmi_intf_load(char * name);

int ipmi_intf_session_set_hostname(struct ipmi_intf * intf, char * hostname);
int ipmi_intf_session_set_username(struct ipmi_intf * intf, char * username);
int ipmi_intf_session_set_password(struct ipmi_intf * intf, char * password);
int ipmi_intf_session_set_privlvl(struct ipmi_intf * intf, unsigned char privlvl);
int ipmi_intf_session_set_port(struct ipmi_intf * intf, int port);

#endif /* IPMI_INTF_H */
