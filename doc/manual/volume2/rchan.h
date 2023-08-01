typedef struct list_rchan {
	char                    *li_mta; /* the MTA */
	CHAN                    *li_chan; /* the channel its on */
	union {
		AUTH            *li_un_auth; /* auth stuff used in submit */
		char		*li_un_dir; /* the directory associated */
	} list_rchan_un;
	struct list_rchan       *li_next; /* next in the chain */
} LIST_RCHAN;
#define li_auth list_rchan_un.li_un_auth
#define li_dir list_rchan_un.li_un_dir

#define NULLIST_RCHAN           ((LIST_RCHAN *)0)
