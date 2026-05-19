/* $Id: datetype.c 3983 2011-03-21 17:23:56Z mark_ellis $ */
#include "synce.h"
#include <math.h>
#include <string.h>
#include <synce_log.h>

#define VAR_TIMEVALUEONLY	DATE_TIMEVALUEONLY
#define VAR_DATEVALUEONLY	DATE_DATEVALUEONLY

static BOOL TmToDATE( struct tm* pTm, DATE *pDateOut );
static BOOL DateToTm( DATE dateIn, DWORD dwFlags, struct tm* pTm );

bool date_from_tm(struct tm* pTm, DATE *pDateOut)
{
	struct tm copy = *pTm;
	copy.tm_year += 1900;
	return TmToDATE(&copy, pDateOut);
}

bool date_to_tm(DATE dateIn, DWORD dwFlags, struct tm* pTm)
{
	struct tm result;
	bool success = DateToTm(dateIn, dwFlags, &result);
	if (success)
	{
		synce_trace("result.tm_year=%i", result.tm_year);
		result.tm_year += 1900;
		*pTm = result;
	}
	return success;
}

/*
 * Note a leap year is one that is a multiple of 4
 * but not of a 100.  Except if it is a multiple of
 * 400 then it is a leap year.
 */
#define isleap(y) (((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0)))

/*
 * Copyright 1998 Jean-Claude Cote
 *
 * These functions come from wine/dlls/oleaut32/variant.c in release 20020228
 * of the WINE project. See http://www.winehq.org/ for more information. 
 *
 * Licensing information for the code below:
 *
 *   http://source.winehq.org/source/LICENSE?v=wine20020228
 *
 */

/*
 * Note a leap year is one that is a multiple of 4
 * but not of a 100.  Except if it is a multiple of
 * 400 then it is a leap year.
 */
/* According to postgreSQL date parsing functions there is
 * a leap year when this expression is true.
 * (((y % 4) == 0) && (((y % 100) != 0) || ((y % 400) == 0)))
 * So according to this there is 365.2515 days in one year.
 * One + every four years: 1/4 -> 365.25
 * One - every 100 years: 1/100 -> 365.01
 * One + every 400 years: 1/400 -> 365.0025
 */
/* static const double DAYS_IN_ONE_YEAR = 365.2515;
 *
 *  ^^  Might this be the key to an easy way to factor large prime numbers?
 *  Let's try using arithmetic.  <lawson_whitney@juno.com> 7 Mar 2000
 */
static const double DAYS_IN_ONE_YEAR = 365.0; /*365.2425;*/


/******************************************************************************
 *	   TmToDATE 	[INTERNAL]
 *
 * The date is implemented using an 8 byte floating-point number.
 * Days are represented by whole numbers increments starting with 0.00 has
 * being December 30 1899, midnight.
 * The hours are expressed as the fractional part of the number.
 * December 30 1899 at midnight = 0.00
 * January 1 1900 at midnight = 2.00
 * January 4 1900 at 6 AM = 5.25
 * January 4 1900 at noon = 5.50
 * December 29 1899 at midnight = -1.00
 * December 18 1899 at midnight = -12.00
 * December 18 1899 at 6AM = -12.25
 * December 18 1899 at 6PM = -12.75
 * December 19 1899 at midnight = -11.00
 * The tm structure is as follows:
 * struct tm {
 *		  int tm_sec;	   seconds after the minute - [0,59]
 *		  int tm_min;	   minutes after the hour - [0,59]
 *		  int tm_hour;	   hours since midnight - [0,23]
 *		  int tm_mday;	   day of the month - [1,31]
 *		  int tm_mon;	   months since January - [0,11]
 *		  int tm_year;	   years
 *		  int tm_wday;	   days since Sunday - [0,6]
 *		  int tm_yday;	   days since January 1 - [0,365]
 *		  int tm_isdst;    daylight savings time flag
 *		  };
 *
 * Note: This function does not use the tm_wday, tm_yday, tm_wday,
 * and tm_isdst fields of the tm structure. And only converts years
 * after 1900.
 *
 * Returns TRUE if successful.
 */
static BOOL TmToDATE( struct tm* pTm, DATE *pDateOut )
{
    int leapYear = 0;

    if( (pTm->tm_year - 1900) < 0 ) return FALSE;

    /* Start at 1. This is the way DATE is defined.
     * January 1, 1900 at Midnight is 1.00.
     * January 1, 1900 at 6AM is 1.25.
     * and so on.
     */
    *pDateOut = 1;

    /* Add the number of days corresponding to
     * tm_year.
     */
    *pDateOut += (pTm->tm_year - 1900) * 365;

    /* Add the leap days in the previous years between now and 1900.
     * Note a leap year is one that is a multiple of 4
     * but not of a 100.  Except if it is a multiple of
     * 400 then it is a leap year.
     */
    *pDateOut += ( (pTm->tm_year - 1) / 4 ) - ( 1900 / 4 );
    *pDateOut -= ( (pTm->tm_year - 1) / 100 ) - ( 1900 / 100 );
    *pDateOut += ( (pTm->tm_year - 1) / 400 ) - ( 1900 / 400 );

    /* Set the leap year flag if the
     * current year specified by tm_year is a
     * leap year. This will be used to add a day
     * to the day count.
     */
    if( isleap( pTm->tm_year ) )
        leapYear = 1;

    /* Add the number of days corresponding to
     * the month.
     */
    switch( pTm->tm_mon )
    {
    case 2:
        *pDateOut += 31;
        break;
    case 3:
        *pDateOut += ( 59 + leapYear );
        break;
    case 4:
        *pDateOut += ( 90 + leapYear );
        break;
    case 5:
        *pDateOut += ( 120 + leapYear );
        break;
    case 6:
        *pDateOut += ( 151 + leapYear );
        break;
    case 7:
        *pDateOut += ( 181 + leapYear );
        break;
    case 8:
        *pDateOut += ( 212 + leapYear );
        break;
    case 9:
        *pDateOut += ( 243 + leapYear );
        break;
    case 10:
        *pDateOut += ( 273 + leapYear );
        break;
    case 11:
        *pDateOut += ( 304 + leapYear );
        break;
    case 12:
        *pDateOut += ( 334 + leapYear );
        break;
    }
    /* Add the number of days in this month.
     */
    *pDateOut += pTm->tm_mday;

    /* Add the number of seconds, minutes, and hours
     * to the DATE. Note these are the fracionnal part
     * of the DATE so seconds / number of seconds in a day.
     */
    *pDateOut += pTm->tm_hour / 24.0;
    *pDateOut += pTm->tm_min / 1440.0;
    *pDateOut += pTm->tm_sec / 86400.0;
    return TRUE;
}

/******************************************************************************
 *	   DateToTm 	[INTERNAL]
 *
 * This function converts a windows DATE to a tm structure.
 *
 * It does not fill all the fields of the tm structure.
 * Here is a list of the fields that are filled:
 * tm_sec, tm_min, tm_hour, tm_year, tm_day, tm_mon.
 *
 * Note this function does not support dates before the January 1, 1900
 * or ( dateIn < 2.0 ).
 *
 * Returns TRUE if successful.
 */
static BOOL DateToTm( DATE dateIn, DWORD dwFlags, struct tm* pTm )
{
    double decimalPart = 0.0;
    double wholePart = 0.0;

    /* Do not process dates smaller than January 1, 1900.
     * Which corresponds to 2.0 in the windows DATE format.
     */
    if( dateIn < 2.0 ) return FALSE;

    memset(pTm,0,sizeof(*pTm));

    /* Because of the nature of DATE format which
     * associates 2.0 to January 1, 1900. We will
     * remove 1.0 from the whole part of the DATE
     * so that in the following code 1.0
     * will correspond to January 1, 1900.
     * This simplifies the processing of the DATE value.
     */
    dateIn -= 1.0;

    wholePart = (double) floor( dateIn );
    decimalPart = fmod( dateIn, wholePart );

    if( !(dwFlags & VAR_TIMEVALUEONLY) )
    {
        int nDay = 0;
        int leapYear = 0;
        double yearsSince1900 = 0;
        /* Start at 1900, this is where the DATE time 0.0 starts.
         */
        pTm->tm_year = 1900;
        /* find in what year the day in the "wholePart" falls into.
         * add the value to the year field.
         */
        yearsSince1900 = floor( (wholePart / DAYS_IN_ONE_YEAR) + 0.001 );
        pTm->tm_year += yearsSince1900;
        /* determine if this is a leap year.
         */
        if( isleap( pTm->tm_year ) )
        {
            leapYear = 1;
            wholePart++;
        }

        /* find what day of that year the "wholePart" corresponds to.
         * Note: nDay is in [1-366] format
         */
        nDay = (int) ( wholePart - floor( yearsSince1900 * DAYS_IN_ONE_YEAR ) );
        /* Set the tm_yday value.
         * Note: The day must be converted from [1-366] to [0-365]
         */
        /*pTm->tm_yday = nDay - 1;*/
        /* find which month this day corresponds to.
         */
        if( nDay <= 31 )
        {
            pTm->tm_mday = nDay;
            pTm->tm_mon = 0;
        }
        else if( nDay <= ( 59 + leapYear ) )
        {
            pTm->tm_mday = nDay - 31;
            pTm->tm_mon = 1;
        }
        else if( nDay <= ( 90 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 59 + leapYear );
            pTm->tm_mon = 2;
        }
        else if( nDay <= ( 120 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 90 + leapYear );
            pTm->tm_mon = 3;
        }
        else if( nDay <= ( 151 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 120 + leapYear );
            pTm->tm_mon = 4;
        }
        else if( nDay <= ( 181 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 151 + leapYear );
            pTm->tm_mon = 5;
        }
        else if( nDay <= ( 212 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 181 + leapYear );
            pTm->tm_mon = 6;
        }
        else if( nDay <= ( 243 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 212 + leapYear );
            pTm->tm_mon = 7;
        }
        else if( nDay <= ( 273 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 243 + leapYear );
            pTm->tm_mon = 8;
        }
        else if( nDay <= ( 304 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 273 + leapYear );
            pTm->tm_mon = 9;
        }
        else if( nDay <= ( 334 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 304 + leapYear );
            pTm->tm_mon = 10;
        }
        else if( nDay <= ( 365 + leapYear ) )
        {
            pTm->tm_mday = nDay - ( 334 + leapYear );
            pTm->tm_mon = 11;
        }
    }
    if( !(dwFlags & VAR_DATEVALUEONLY) )
    {
        /* find the number of seconds in this day.
         * fractional part times, hours, minutes, seconds.
         */
        pTm->tm_hour = (int) ( decimalPart * 24 );
        pTm->tm_min = (int) ( ( ( decimalPart * 24 ) - pTm->tm_hour ) * 60 );
        pTm->tm_sec = (int) ( ( ( decimalPart * 24 * 60 ) - ( pTm->tm_hour * 60 ) - pTm->tm_min ) * 60 );
    }
    return TRUE;
}



