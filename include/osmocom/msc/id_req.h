#ifndef _ID_REQ_H
#define _ID_REQ_H

struct gsm_trans;

int gsm_id_req_send(struct vlr_subscr *vsub,
	int id_type,
	struct vty *vty);

extern int gsm_id_req_recv(struct vlr_subscr *vsub);

void trans_id_req_free(struct gsm_trans *trans);

// #if 0
// extern int silent_call_rx(struct ran_conn *conn, struct msgb *msg);
// extern int silent_call_reroute(struct ran_conn *conn, struct msgb *msg);
// #endif

#endif /* _ID_REQ_H */
