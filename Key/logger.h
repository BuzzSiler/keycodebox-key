#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>
#include "logcategory.h"


namespace kcb
{

    #define LOG_INFO(category, msg)     qCInfo(category()) << msg
    #define LOG_DEBUG(category, msg)    qCDebug(category()) << msg
    #define LOG_WARN(category, msg)     qCWarning(category()) << msg
    #define LOG_CRIT(category, msg)     qCCritical(category()) << msg
    #define LOG_FATAL(category, msg)    qCFatal(category()) << msg

    #define LOG_ENTRY()                 LOG_INFO(kcb::entry(), Q_FUNC_INFO)
    #define LOG_EXIT()                  LOG_INFO(kcb::exit(), Q_FUNC_INFO)
    #define LOG_TRACE_INFO(msg)         LOG_INFO(kcb::trace(), msg)
    #define LOG_TRACE_DEBUG(msg)        LOG_DEBUG(kcb::trace(), msg)
    #define LOG_TRACE_WARN(msg)         LOG_WARN(kcb::trace(), msg)
    #define LOG_TRACE_CRIT(msg)         LOG_CRIT(kcb::trace(), msg)
    #define LOG_TRACE_FATAL(msg)        LOG_FATAL(kcb::trace(), msg)

    // Debug Macros
    #define KCB_DEBUG_ENTRY           LOG_DEBUG(kcb::entry(), Q_FUNC_INFO)
    #define KCB_DEBUG_EXIT            LOG_DEBUG(kcb::exit(), Q_FUNC_INFO)

    #define KCB_DEBUG_TRACE(msg)      LOG_DEBUG(kcb::trace(), msg)
    #define KCB_WARNING_TRACE(msg)    LOG_WARN(kcb::trace(), msg)
    #define KCB_CRITICAL_TRACE(msg)   LOG_CRIT(kcb::trace(), msg)
    #define KCB_FATAL_TRACE(msg)      LOG_FATAL(kcb::trace(), msg)

    class Logger
    {
        public:
            enum LOG_LEVEL { LEVEL_INFO, LEVEL_DEBUG, LEVEL_WARNING, LEVEL_CRITICAL, LEVEL_FATAL };

            static void installHandler();
            static void setLevel(LOG_LEVEL value);
            static LOG_LEVEL level();
    };

}

#endif // LOGGER_H
