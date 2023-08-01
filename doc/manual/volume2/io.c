submit_message (orig, recips, fp)
ADDR	*orig;
ADDR	*recips;
FILE	*fp;
{
	RP_Buf	rps, *rp = &rps;
	char	buf[BUFSIZ];
	ADDR	*ap;

	if (rp_isbad (io_init(rp)))
		error ();

	if (rp_isbad (io_wprm (prm, rp)))
		error ();

	if (rp_isbad (io_wrq (q, rp)))
		error ();

	if (rp_isbad (io_wadr (orig, AD_ORIGINATOR, rp)))
		error ();

	for (ap = ad_recipients; ap; ap = ap -> ad_next)
		if (rp_isbad (io_wadr (ap, AD_RECIPIENT, rp)))
			error ();

	if (rp_isbad (io_adend(rp)))
		error ();

	if (rp_isbad (io_tinit(rp)))
		error ();

	if (rp_isbad (io_tpart ("hdr.822", FALSE, rp)))
		error ();

	while (fgets( buf, sizeof buf, fp))
		if (rp_isbad (io_tdata (buf, strlen (buf))))
			error ();

	if (rp_isbad (io_tdend(rp)))
		error ();

	if (rp_isbad (io_tpart ("1.ia5", FALSE, rp)))
		error ();

	while (fgets (buf, sizeof buf, stdin))
		if (rp_isbad (io_tdata (buf, strlen (buf))))
			error ();

	if (rp_isbad (io_tdend (rp)))
		error ();

	if (rp_isbad (io_tend (rp)))
		error ();

	 io_end(OK);
}
