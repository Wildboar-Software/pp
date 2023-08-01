io_init(rp);      /* initialisation */
io_wprm(&prm,rp); /* write management parameters */
io_wrq(&q,rp);    /* per message parameters */

ap = ad_originator;             /* originator */
io_wadr(ap,AD_ORIGINATOR,rp);
ap = ad_recipient;
do {                            /* list of recipients */
    io_wadr(ap,AD_RECIPIENT,rp);
    ap = ap->ad_next;
    } while ( ap != NULL );
io_adend(rp);                   /* terminate address list */

if((p=next_arg("body",argv)) != NULL) {
    io_tinit(rp);               /* prepare for body part(s) */
    do {
        io_tpart(p,FALSE,rp);   /* prepare one body part */
        loop_data(p);           /* transmit with io_tdata() */
        io_tdend(rp);           /* end one file */
    } while(p=next_arg("body",argv));
}
io_tend(rp));   /* complete message */
io_end(type);   /* terminate session with submit */
