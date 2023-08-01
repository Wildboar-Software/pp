main (argc, argv)
int     argc;
char    **argv;
{
	sys_init (argv[0]);   /* init the channel - and find out who we are */

	dirinit();              /* get to the right directory */
#ifdef PP_DEBUG
	if (argc > 1 && strcmp (argv[1], "debug") == 0)
		debug_channel_control (argc, argv, chaninit, process, endproc);
	else
#endif
		channel_control (argc, argv, chaninit, process, endproc);
	exit (0);
}

static int chaninit (arg)
struct type_Qmgr_Channel *arg;
{
	char    *p = qb2str (arg);

	if ((mychan = ch_nm2struct (p)) == (CHAN *)0)
		err_abrt (RP_PARM, "Channel '%s' not known", p);
	exam_init (mychan);
	free (p);
	return OK;
}

static int endproc ()
{
	if (cur_host)
		exam_close (OK);
}

static struct type_Qmgr_DeliveryStatus *process (arg)
struct type_Qmgr_ProcMsg *arg;
{
	struct prm_vars prm;
	struct type_Qmgr_UserList *up;
	Q_struct        Qstruct, *qp = &Qstruct;
	int     retval;
	ADDR    *ap,
		*ad_sendr = NULLADDR,
		*ad_recip = NULLADDR,
		*alp,
		*ad_list = NULLADDR;
	int     ad_count;

	if (this_msg) free (this_msg);

	this_msg = qb2str (arg -> qid);

	PP_TRACE (("process msg %s", this_msg));

	bzero ((char *)&prm, sizeof prm);
	bzero ((char *)qp, sizeof *qp);

	(void) delivery_init (arg -> users);

	retval = rd_msg (this_msg, &prm, qp, &ad_sendr, &ad_recip, &ad_count);

	if (rp_isbad (retval)) {
		PP_LOG (LLOG_EXCEPTIONS, ("rd_msg err: %s", this_msg));
		return delivery_setall (int_Qmgr_status_messageFailure);
	}

	sender = ad_sendr -> ad_r822adr;


	for (ap = ad_recip; ap; ap = ap -> ad_next) {
		for (up = arg ->users; up; up = up -> next) {
			if (up -> RecipientId -> parm != ap -> ad_no)
				continue;

			if (chan_acheck (ap, mychan,
					 ad_list == NULL, &cur_mta) == NOTOK)
				continue;
		}
		if (up == NULL)
			continue;

		if (ad_list == NULLADDR)
			ad_list = alp = (ADDR *) calloc (1, sizeof *alp);
		else {
			alp -> ad_next = (ADDR *) calloc (1, sizeof *alp);
			alp = alp -> ad_next;
		}
		*alp = *ap;
		alp -> ad_next = NULLADDR;
	}

	if (ad_list == NULLADDR) {
		PP_LOG (LLOG_EXCEPTIONS, ("No recipients in user list"));
		rd_end ();
		return deliverystate;
	}

	deliver (ad_list, qp); /* do what is required */

	rd_end();

	return deliverystate;
}


static void dirinit()       /* Change into pp queue space */
{
	if (chdir (quedfldir) < 0)
		err_abrt (RP_LIO, "Unable to change directory to '%s'",
						quedfldir);
}
