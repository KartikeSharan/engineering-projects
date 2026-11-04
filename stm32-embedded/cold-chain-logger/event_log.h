/*
    event_log.h
    Header file for the event logging module.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    Stores events (temperature breaches, shocks, state changes) in a
    fixed array in RAM and reports them over serial. A real product
    would write these to an SD card for permanent storage, but serial
    output is used here for testing and inspection.
*/

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "mbed.h"

// Maximum events stored. Each event is about 16 bytes, so 50 events
// uses roughly 800 bytes of the 64KB SRAM on the NUCLEO-L432KC.
#define MAX_EVENTS  50

// The different kinds of event the logger can record
typedef enum {
    EVENT_TEMP_HIGH    = 0,   // temperature went above the upper limit
    EVENT_TEMP_LOW     = 1,   // temperature went below the lower limit
    EVENT_TEMP_OK      = 2,   // temperature returned to the safe range
    EVENT_SHOCK        = 3,   // rough handling / shock detected
    EVENT_STATE_CHANGE = 4,   // the system changed operating state
    EVENT_SYSTEM_START = 5    // the system powered on
} EventType_t;

// The operating states of the logger
typedef enum {
    STATE_IDLE      = 0,   // powered on but not actively monitoring
    STATE_TRANSPORT = 1,   // actively monitoring during transport
    STATE_ALERT     = 2    // a breach has been detected
} SystemState_t;

// One stored event record
typedef struct {
    EventType_t   type;          // what kind of event
    uint32_t      timestamp_ms;  // time since power on in milliseconds
    float         value;         // the sensor reading that triggered it
    SystemState_t state;         // the state the system was in
} LogEvent_t;

// Running totals used for the summary line
typedef struct {
    int temp_high_count;
    int temp_low_count;
    int shock_count;
    int total_events;
} EventSummary_t;

// Clear the log and counters. Call once at startup.
void EventLog_Init(void);

// Record a new event and print it to serial straight away.
void EventLog_Record(EventType_t type, float value, SystemState_t state);

// Print every stored event as a table over serial.
void EventLog_PrintAll(void);

// Print a one-line summary of the event counts over serial.
void EventLog_PrintSummary(void);

// Get a copy of the current summary counters.
EventSummary_t EventLog_GetSummary(void);

// Get the total number of events recorded.
int EventLog_GetCount(void);

#endif
