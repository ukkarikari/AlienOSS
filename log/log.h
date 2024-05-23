#include "../list/list.h"

/// @enum Enum to represent the type of logs.
///
/// The log type will be used to redirect the message to
/// its corresponding window in the user interface
typedef enum { PROCESS_LOG, MEMORY_LOG } LOG_TYPE;

/// @brief Log structure used to represent a single message of log
///

typedef struct log {
  char log_message[1000]; /// < log message
  LOG_TYPE log_type;      ///< enum log type
} LogMessage;

/// @brief  This function appends a log message into the corresponding
///         log strucutre
/// @param message message
/// @param log_type log type
///
/// @details It calls the function to append on list and, then, posts
/// the semaphore used in the MainWindow to wake up the thread
/// responsible for printing log messages.
void append_log_message(char *message, LOG_TYPE log_type);

/// @brief  Rebuild all the log, printing every log message again
void rebuild_all_log();

int compare_log(void *d1, void *d2);