/* order.c: routines for use in ordering calculations */

# ifndef lint
static char Rcsid[] = "@(#)$Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/order.c,v 6.0 1991/12/18 20:23:58 jpo Rel $";
# endif

/*
 * $Header: /xtel/pp/pp-beta/Lib/qmgr/RCS/order.c,v 6.0 1991/12/18 20:23:58 jpo Rel $
 *
 * $Log: order.c,v $
 * Revision 6.0  1991/12/18  20:23:58  jpo
 * Release 6.0
 *
 */

#include "util.h"
#include "qmgr-int.h"

/*  */

/* -1 == MAX */

double measure_Channel(chan, measure, currenttime)
register ChannelInfo	*chan;
register MeasureInfo	*measure;
time_t			currenttime;
{
	double	average_db = 0.0;
	int	noFactors = 0;

	if (NULL == measure) 
		return 0;

	if (0.0 != measure->ub_number) {
		if (chan->numberMessages + chan->numberReports >= measure->ub_number)
			return -1;
		average_db += (chan->numberMessages+chan->numberReports)*100/measure->ub_number;
		noFactors++;
	}

	if (0.0 != measure->ub_volume) {
		if (chan->volumeMessages >= measure->ub_volume)
			return -1;
		average_db += (chan->volumeMessages*100)/measure->ub_volume;
		noFactors++;
	}

	if (0.0 != measure->ub_age) {
		double	age;
		if (chan->oldestMessage != 0
		    && (chan->numberMessages != 0
			|| chan->numberReports != 0))
			age = (currenttime - chan->oldestMessage) / 60.0;
		else
			age = 0.0;
		if (age >= measure -> ub_age)
			return -1;
		average_db += (age * 100)/measure->ub_age;
		noFactors++;
	}

	if (0.0 != measure->ub_last) {
		double	last;
		if (0 != chan->status->lastSuccess)
			last = (currenttime - chan->status->lastSuccess)/60.0;
		else
			last = 0.0;
		if (last >= measure->ub_last)
			return -1;
		if (last >= measure->ub_last/2) {
			/* only kick in at halfway */
			average_db += (last * 100)/ measure->ub_last;
			noFactors++;
		}
	}

	if (1 < noFactors)
		average_db = average_db / noFactors;

	return average_db;
}

/*  */

/* -1 == MAX */

double measure_Mta(mta, measure, currenttime)
register MtaInfo	*mta;
register MeasureInfo	*measure;
time_t			currenttime;
{
	double	average_db = 0.0;
	int	noFactors = 0;

	if (NULL == measure) 
		return 0;

	if (0.0 != measure->ub_number) {
		if (mta->numberMessages + mta->numberReports >= measure->ub_number)
			return -1;
		average_db += (mta->numberMessages+mta->numberReports)*100/measure->ub_number;
		noFactors++;
	}

	if (0.0 != measure->ub_volume) {
		if (mta->volumeMessages >= measure->ub_volume)
			return -1;
		average_db += (mta->volumeMessages*100)/measure->ub_volume;
		noFactors++;
	}

	if (0.0 != measure->ub_age) {
		double	age;
		if (mta->oldestMessage != 0
		    && (mta->numberMessages != 0
			|| mta->numberReports != 0))
			age = (currenttime - mta->oldestMessage) / 60.0;
		else
			age = 0.0;
		if (age >= measure -> ub_age)
			return -1;
		average_db += (age * 100)/measure->ub_age;
		noFactors++;
	}

	if (0.0 != measure->ub_last) {
		double	last;
		if (0 != mta->status->lastSuccess)
			last = (currenttime - mta->status->lastSuccess)/60.0;
		else
			last = 0.0;
		if (last >= measure->ub_last)
			return -1;
		if (last >= measure->ub_last/2) {
			/* only kick in at halfway */
			average_db += (last * 100)/ measure->ub_last;
			noFactors++;
		}
	}

	if (1 < noFactors)
		average_db = average_db / noFactors;

	return average_db;
}

/*  */

/* -1 == MAX */

double measure_Msg(msg, measure, currenttime)
register MsgInfo	*msg;
register MeasureInfo	*measure;
time_t			currenttime;
{
	double	average_db = 0.0;
	int	noFactors = 0;

	if (NULL == measure)
		return 0;

	if (msg->permsginfo->expiryTime != 0
	    && (currenttime - msg->permsginfo->expiryTime) >= 0)
	    /* expired so return MAX */
	    return -1;

	if (0.0 != measure->ub_age) {
		double	age;
		age = (currenttime - msg->permsginfo->age) / 60.0;
		if (age >= measure->ub_age)
			return -1;
		average_db += (age * 100)/measure->ub_age;
		noFactors++;
	}

	if (0.0 != measure->ub_volume) {
		if (msg->permsginfo->size >= measure->ub_volume)
			return -1;
		average_db += (msg->permsginfo->size * 100) / measure->ub_volume;
		noFactors++;
	}

	if (1 < noFactors)
		average_db = average_db / noFactors;
	return average_db;
}
