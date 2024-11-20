#include <iostream>

#include "Log.h"

int main() {
    Log log;
    Log::set_log_config({.entry_max_lines=1, .print_log_to_stdout=4});

    log.open("/tmp/h2zmu/log");
    log.write({
        .message="TEST log.write()",
        .thrownAt=__PRETTY_FUNCTION__
    });
    log.error({
        .message="TEST log.write()",
        .thrownAt=__PRETTY_FUNCTION__
    });
    log.write({
        .message="TEST log.write(logfile::error)\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\nabc\n",
        .thrownAt=__PRETTY_FUNCTION__,
    },
              logfile::error
    );
    log.write({
        .message="TEST log.write(logfile::sql)",
        .thrownAt=__PRETTY_FUNCTION__,
    },
              logfile::sql
    );
    log.write({
        .message="TEST log.write(logfile::info)",
        .thrownAt=__PRETTY_FUNCTION__,
    },
              logfile::info
    );
    log.write({
        .message="TEST log.write(logfile::log_access)",
        .thrownAt=__PRETTY_FUNCTION__,
    },
              logfile::log_access
    );
    log.write({
        .message="TEST log.write(logfile::func)",
        .thrownAt=__PRETTY_FUNCTION__,
    },
              logfile::func
    );
    return 0;
}
