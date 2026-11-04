/*
    event_log.cpp
    Event logging module implementation.
    Cold-Chain Transport Condition Logger
    UESTC 2004 Embedded Processors - University of Glasgow

    Events are kept in a fixed array. When the array fills up the
    oldest event is overwritten (a simple circular buffer). All the
    serial output splits floats into integer parts (e.g. 24 and 25
    for 24.25) because the Mbed minimal printf does not support %f.
*/

#include "event_log.h"
#include <stdio.h>
#include <string.h>
#include "rtos/Kernel.h"

// Storage for the events and the tracking counters
static LogEvent_t     event_buffer[MAX_EVENTS];
static int            write_index = 0;   // next slot to write to
static int            total_count = 0;   // total events ever recorded
static EventSummary_t summary;           // running totals

// Turn an event type into a short readable label
static const char* eventTypeStr(EventType_t type)
{
    switch (type) {
        case EVENT_TEMP_HIGH:    return "TEMP_HIGH";
        case EVENT_TEMP_LOW:     return "TEMP_LOW";
        case EVENT_TEMP_OK:      return "TEMP_OK";
        case EVENT_SHOCK:        return "SHOCK";
        case EVENT_STATE_CHANGE: return "STATE_CHANGE";
        case EVENT_SYSTEM_START: return "SYS_START";
        default:                 return "UNKNOWN";
    }
}

// Turn a state into a short readable label
static const char* stateStr(SystemState_t state)
{
    switch (state) {
        case STATE_IDLE:      return "IDLE";
        case STATE_TRANSPORT: return "TRANSPORT";
        case STATE_ALERT:     return "ALERT";
        default:              return "UNKNOWN";
    }
}

// Get the current time in milliseconds since the system started
static uint32_t getTimeMs(void)
{
    return (uint32_t)chrono::duration_cast<chrono::milliseconds>(
               Kernel::Clock::now().time_since_epoch()).count();
}

/*
    EventLog_Init
    Clears the event buffer and resets all counters.
*/
void EventLog_Init(void)
{
    memset(event_buffer, 0, sizeof(event_buffer));
    write_index = 0;
    total_count = 0;
    summary.temp_high_count = 0;
    summary.temp_low_count  = 0;
    summary.shock_count     = 0;
    summary.total_events    = 0;
}

/*
    EventLog_Record
    Stores a new event, updates the counters, and prints the event to
    serial so it can be seen live during testing.
*/
void EventLog_Record(EventType_t type, float value, SystemState_t state)
{
    // Fill in the event at the current write position
    event_buffer[write_index].type         = type;
    event_buffer[write_index].timestamp_ms = getTimeMs();
    event_buffer[write_index].value        = value;
    event_buffer[write_index].state        = state;

    // Remember the timestamp before we move the index on
    uint32_t this_time = event_buffer[write_index].timestamp_ms;

    // Advance the write position, wrapping around when full
    write_index = (write_index + 1) % MAX_EVENTS;
    total_count++;

    // Update the running totals
    summary.total_events++;
    if (type == EVENT_TEMP_HIGH) summary.temp_high_count++;
    if (type == EVENT_TEMP_LOW)  summary.temp_low_count++;
    if (type == EVENT_SHOCK)     summary.shock_count++;

    // Print the event (value split into integer and 2-decimal parts)
    printf("[EVENT] %lums | %s | %d.%02d | %s\n",
           (unsigned long)this_time,
           eventTypeStr(type),
           (int)value,
           abs((int)(value * 100)) % 100,
           stateStr(state));
}

/*
    EventLog_PrintAll
    Prints the whole event log as a table. Used at the end of a
    transport when the user presses the button to inspect the log.
*/
void EventLog_PrintAll(void)
{
    int num_events = (total_count < MAX_EVENTS) ? total_count : MAX_EVENTS;

    printf("\n--- COLD CHAIN TRANSPORT LOG ---\n");
    printf("No. | Time (ms)  | Event        | Value  | State\n");
    printf("----+------------+--------------+--------+----------\n");

    for (int i = 0; i < num_events; i++) {
        printf(" %2d | %10lu | %-12s | %3d.%02d | %s\n",
               i + 1,
               (unsigned long)event_buffer[i].timestamp_ms,
               eventTypeStr(event_buffer[i].type),
               (int)event_buffer[i].value,
               abs((int)(event_buffer[i].value * 100)) % 100,
               stateStr(event_buffer[i].state));
    }

    printf("--------------------------------\n");
    EventLog_PrintSummary();
    printf("--------------------------------\n\n");
}

/*
    EventLog_PrintSummary
    Prints one line with the count of each kind of event.
*/
void EventLog_PrintSummary(void)
{
    printf("[SUMMARY] Total: %d | TempHigh: %d | TempLow: %d | Shocks: %d\n",
           summary.total_events,
           summary.temp_high_count,
           summary.temp_low_count,
           summary.shock_count);
}

/*
    EventLog_GetSummary
    Returns a copy of the current summary counters.
*/
EventSummary_t EventLog_GetSummary(void)
{
    return summary;
}

/*
    EventLog_GetCount
    Returns how many events have been recorded in total.
*/
int EventLog_GetCount(void)
{
    return total_count;
}
