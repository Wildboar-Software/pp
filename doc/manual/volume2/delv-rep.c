...
    {
	char buffer[BUFSIZ];

	(void) sprintf (buffer, "Address %s failed for reason %s",
			ap -> ad_r822adr, rstr);
	
	(void) delivery_set (ap -> ad_no,
			     int_Qmgr_status_permanentFailure);
	set_dr (qp, ap -> ad_no, msg_name,
		  DRR_UNABLE_TO_TRANSFER,
		  DRD_UNRECOGNISED_OR, buffer);
    }
...
