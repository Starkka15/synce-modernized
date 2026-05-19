/*
 * This is a very generous contribution that will allow 
 * SynCE to properly handle dates before 1970.
 *
 * So, everywhere "time_t" or "struct tm" is used, use TIME_FIELDS instead!
 */
#include "synce.h"

static VOID RtlTimeToTimeFields(
    const LARGE_INTEGER *liTime,
    PTIME_FIELDS TimeFields);

static BOOLEAN RtlTimeFieldsToTime(
    const TIME_FIELDS* tfTimeFields,
    PLARGE_INTEGER Time);

/**
 * Wrapper for RtlTimeToTimeFields
 */
void time_fields_from_filetime(const FILETIME* filetime, TIME_FIELDS* timeFields)
{
  LARGE_INTEGER tmp;
  
  tmp.u.LowPart  = filetime->dwLowDateTime;
  tmp.u.HighPart = filetime->dwHighDateTime;

  RtlTimeToTimeFields(&tmp, timeFields);
}

/**
 * Wrapper for RtlTimeFieldsToTime
 */
bool time_fields_to_filetime(const TIME_FIELDS* timeFields, FILETIME* filetime)
{
  bool result;
  LARGE_INTEGER tmp;

  result = RtlTimeFieldsToTime(timeFields, &tmp);

  filetime->dwLowDateTime  = tmp.u.LowPart;
  filetime->dwHighDateTime = tmp.u.HighPart;

  return result;
}


/*
 * RtlTimeToTimeFields and RtlTimeFieldsToTime relicensed from Wine under the
 * terms of this project's license.
 *
 * Huw D M Davies <h.davies1@physics.ox.ac.uk>
 * Rein Klazes <wijn@wanadoo.nl>
 * 2005-02-15
 *
 */

#define TICKSPERSEC        10000000
#define TICKSPERMSEC       10000
#define SECSPERDAY         86400
#define SECSPERHOUR        3600
#define SECSPERMIN         60
#define MINSPERHOUR        60
#define HOURSPERDAY        24
#define EPOCHWEEKDAY       1  /* Jan 1, 1601 was Monday */
#define DAYSPERWEEK        7
#define EPOCHYEAR          1601
#define DAYSPERNORMALYEAR  365
#define DAYSPERLEAPYEAR    366
#define MONSPERYEAR        12
#define DAYSPERQUADRICENTENNIUM (365 * 400 + 97)
#define DAYSPERNORMALCENTURY (365 * 100 + 24)
#define DAYSPERNORMALQUADRENNIUM (365 * 4 + 1)

static const int MonthLengths[2][MONSPERYEAR] =
{
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int IsLeapYear(int Year)
{
  return Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0) ? 1 : 0;
}

/******************************************************************************
 *       RtlTimeToTimeFields [NTDLL.@]
 *
 * Convert a time into a TIME_FIELDS structure.
 *
 * PARAMS
 *   liTime     [I] Time to convert.
 *   TimeFields [O] Destination for the converted time.
 *
 * RETURNS
 *   Nothing.
 */
static VOID RtlTimeToTimeFields(
    const LARGE_INTEGER *liTime,
    PTIME_FIELDS TimeFields)
{
  int SecondsInDay;
  long int cleaps, years, yearday, months;
  long int Days;
  LONGLONG Time;

  /* Extract millisecond from time and convert time into seconds */
  TimeFields->Milliseconds =
    (CSHORT) (( liTime->QuadPart % TICKSPERSEC) / TICKSPERMSEC);
  Time = liTime->QuadPart / TICKSPERSEC;

  /* The native version of RtlTimeToTimeFields does not take leap seconds
   * into account */

  /* Split the time into days and seconds within the day */
  Days = Time / SECSPERDAY;
  SecondsInDay = Time % SECSPERDAY;

  /* compute time of day */
  TimeFields->Hour = (CSHORT) (SecondsInDay / SECSPERHOUR);
  SecondsInDay = SecondsInDay % SECSPERHOUR;
  TimeFields->Minute = (CSHORT) (SecondsInDay / SECSPERMIN);
  TimeFields->Second = (CSHORT) (SecondsInDay % SECSPERMIN);

  /* compute day of week */
  TimeFields->Weekday = (CSHORT) ((EPOCHWEEKDAY + Days) % DAYSPERWEEK);

  /* compute year, month and day of month. */
  cleaps=( 3 * ((4 * Days + 1227) / DAYSPERQUADRICENTENNIUM) + 3 ) / 4;
  Days += 28188 + cleaps;
  years = (20 * Days - 2442) / (5 * DAYSPERNORMALQUADRENNIUM);
  yearday = Days - (years * DAYSPERNORMALQUADRENNIUM)/4;
  months = (64 * yearday) / 1959;
  /* the result is based on a year starting on March.
   * To convert take 12 from Januari and Februari and
   * increase the year by one. */
  if( months < 14 ) {
    TimeFields->Month = months - 1;
    TimeFields->Year = years + 1524;
  } else {
    TimeFields->Month = months - 13;
    TimeFields->Year = years + 1525;
  }
  /* calculation of day of month is based on the wonderful
   * sequence of INT( n * 30.6): it reproduces the 
   * 31-30-31-30-31-31 month lengths exactly for small n's */
  TimeFields->Day = yearday - (1959 * months) / 64 ;
  return;
}

/******************************************************************************
 *       RtlTimeFieldsToTime [NTDLL.@]
 *
 * Convert a TIME_FIELDS structure into a time.
 *
 * PARAMS
 *   ftTimeFields [I] TIME_FIELDS structure to convert.
 *   Time         [O] Destination for the converted time.
 *
 * RETURNS
 *   Success: TRUE.
 *   Failure: FALSE.
 */
static BOOLEAN RtlTimeFieldsToTime(
    const TIME_FIELDS* tfTimeFields,
    PLARGE_INTEGER Time)
{
  int month, year, cleaps, day;

  /* FIXME: normalize the TIME_FIELDS structure here */
  /* No, native just returns 0 (error) if the fields are not */
  if( tfTimeFields->Milliseconds< 0 || tfTimeFields->Milliseconds > 999 ||
      tfTimeFields->Second < 0 || tfTimeFields->Second > 59 ||
      tfTimeFields->Minute < 0 || tfTimeFields->Minute > 59 ||
      tfTimeFields->Hour < 0 || tfTimeFields->Hour > 23 ||
      tfTimeFields->Month < 1 || tfTimeFields->Month > 12 ||
      tfTimeFields->Day < 1 ||
      tfTimeFields->Day > MonthLengths
      [ tfTimeFields->Month ==2 || IsLeapYear(tfTimeFields->Year)]
      [ tfTimeFields->Month - 1] ||
      tfTimeFields->Year < 1601 )
    return FALSE;

  /* now calculate a day count from the date
   * First start counting years from March. This way the leap days
   * are added at the end of the year, not somewhere in the middle.
   * Formula's become so much less complicate that way.
   * To convert: add 12 to the month numbers of Jan and Feb, and 
   * take 1 from the year */
  if(tfTimeFields->Month < 3) {
    month = tfTimeFields->Month + 13;
    year = tfTimeFields->Year - 1;
  } else {
    month = tfTimeFields->Month + 1;
    year = tfTimeFields->Year;
  }
  cleaps = (3 * (year / 100) + 3) / 4;   /* nr of "century leap years"*/
  day =  (36525 * year) / 100 - cleaps + /* year * dayperyr, corrected */
    (1959 * month) / 64 +         /* months * daypermonth */
    tfTimeFields->Day -          /* day of the month */
    584817 ;                      /* zero that on 1601-01-01 */
  /* done */

  Time->QuadPart = (((((LONGLONG) day * HOURSPERDAY +
            tfTimeFields->Hour) * MINSPERHOUR +
          tfTimeFields->Minute) * SECSPERMIN +
        tfTimeFields->Second ) * 1000 +
      tfTimeFields->Milliseconds ) * TICKSPERMSEC;

  return TRUE;
}


