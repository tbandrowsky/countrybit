 /* --------------------------------------------------------
 HEADER SECTION
*/



/* ------------------------------------------------------------------
 MESSAGE DEFINITION SECTION
*/
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_RUNTIME                 0x1
#define FACILITY_ENGINE                  0x2
#define FACILITY_ENGINE                  0x3
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: CORONA_GENERAL_CATEGORY
//
// MessageText:
//
// General
//
#define CORONA_GENERAL_CATEGORY          ((WORD)0x00000001L)

//
// MessageId: CORONA_OTHER_CATEGORY
//
// MessageText:
//
// Other
//
#define CORONA_OTHER_CATEGORY            ((WORD)0x00000002L)

//
// MessageId: CORONA_SERVICE_INFORMATION
//
// MessageText:
//
// %1%nat SourceFile:%2%nLine:%3
//
#define CORONA_SERVICE_INFORMATION       ((DWORD)0x00030100L)

//
// MessageId: CORONA_SERVICE_WARNING
//
// MessageText:
//
// %1%nat SourceFile:%2%nLine:%3
//
#define CORONA_SERVICE_WARNING           ((DWORD)0x80030101L)

//
// MessageId: CORONA_SERVICE_ERROR
//
// MessageText:
//
// %1%nat SourceFile:%2%nLine:%3
//
#define CORONA_SERVICE_ERROR             ((DWORD)0xC0000102L)

