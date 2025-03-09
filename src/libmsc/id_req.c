#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <osmocom/core/byteswap.h>
#include <osmocom/core/msgb.h>
#include <osmocom/core/utils.h>
#include <osmocom/msc/signal.h>
#include <osmocom/msc/debug.h>
#include <osmocom/msc/gsm_data.h>
#include <osmocom/msc/gsm_subscriber.h>
#include <osmocom/msc/vlr.h>
#include <osmocom/msc/msc_a.h>
#include <osmocom/msc/paging.h>
#include <osmocom/msc/transaction.h>
#include <osmocom/msc/silent_call.h>
#include <osmocom/gsm/protocol/gsm_04_08.h>
#include <osmocom/msc/vty.h>
#include <osmocom/sigtran/sccp_helpers.h>

/* paging of the requested subscriber has completed */
void paging_id_req_w(struct msc_a *msc_a, struct gsm_trans *trans)
{
	struct id_req_signal_data sigdata = {
		.msc_a = msc_a,
        .id_type = trans->id_req.id_type,
		.vty = trans->id_req.from_vty,
	};

	if (!msc_a) {
		LOG_MSC_A(msc_a, LOGL_ERROR, "Identity Request: MS not responding to Paging\n");
		osmo_signal_dispatch(SS_SUBSCR, S_SUBSCR_DETACHED, &sigdata);
		trans_free(trans);
		return;
	}

	LOG_MSC_A(msc_a, LOGL_INFO, "Identity Request: MS responding to Paging\n");

	trans->msc_a = msc_a;
	msc_a_get(msc_a, MSC_A_USE_ID_REQ);

    trans->vsub->vlr->ops.tx_id_req(trans->msc_a, trans->id_req.id_type);

    switch (trans->id_req.id_type)
    {
    case GSM_MI_TYPE_IMEISV:
        vty_out(trans->id_req.from_vty, "IMEISV = %s\n", trans->vsub->imeisv);
        break;

    case GSM_MI_TYPE_IMEI:
        vty_out(trans->id_req.from_vty, "IMEI = %s\n", trans->vsub->imei);
        break;

    case GSM_MI_TYPE_IMSI:
        vty_out(trans->id_req.from_vty, "IMSI = %s\n", trans->vsub->imsi);
        break;

    case GSM_MI_TYPE_TMSI:
        vty_out(trans->id_req.from_vty, "TMSI = 0x%x\n", trans->vsub->tmsi);
        break;
    
    default:
        break;
    }

}

void trans_id_req_free(struct gsm_trans *trans)
{
	struct id_req_signal_data sigdata = {
		.msc_a = trans->msc_a,
        .id_type = trans->id_req.id_type,
		.vty = trans->id_req.from_vty,
	};
	osmo_signal_dispatch(SS_SUBSCR, S_SUBSCR_DETACHED, &sigdata);
}


/* initiate a identity request with a given subscriber */
int gsm_id_req_send(struct vlr_subscr *vsub,
	int id_type,
	struct vty *vty)
{
	struct gsm_network *net = vsub->vlr->user_ctx;
	struct gsm_trans *trans = trans_alloc(net, vsub, TRANS_ID_REQ, 0, 0);

	trans->id_req.from_vty = vty;
    trans->id_req.id_type = id_type;

	if (!paging_request_start(vsub, PAGING_CAUSE_SIGNALLING_HIGH_PRIO, paging_id_req_w, trans,
				  "send identity request")) {
		trans_free(trans);
		return -ENODEV;
	}

	return 0;
}