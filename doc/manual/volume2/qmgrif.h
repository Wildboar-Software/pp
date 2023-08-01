struct type_Qmgr_ProcMsg {
    struct type_Qmgr_QID *qid;

    struct type_Qmgr_UserList *users;

    struct type_Qmgr_Channel *channel;
};

struct type_Qmgr_DeliveryStatus {
        struct type_Qmgr_IndividualDeliveryStatus *IndividualDeliveryStatus;

        struct type_Qmgr_DeliveryStatus *next;
};

struct type_Qmgr_IndividualDeliveryStatus {
    struct type_Qmgr_RecipientId *recipient;

    integer     status;
#define	int_Qmgr_status_success	0
#define	int_Qmgr_status_successSharedDR	1
#define	int_Qmgr_status_failureSharedDR	2
#define	int_Qmgr_status_negativeDR	3
#define	int_Qmgr_status_positiveDR	4
#define	int_Qmgr_status_messageFailure	5
#define	int_Qmgr_status_mtaFailure	6
#define	int_Qmgr_status_mtaAndMessageFailure	7

    struct	qbuf	*info;
};
