#include "cabinet_type.h"

namespace kcb
{
    bool operator==(CABINET_INFO const & lhs, CABINET_INFO const & rhs)
    {
        bool model_equal = lhs.model == rhs.model;
        bool num_locks_equal = lhs.num_locks == rhs.num_locks;
        bool start_equal = lhs.start == rhs.start;
        bool stop_equal = lhs.stop == rhs.stop;
        bool sw_version_equal = lhs.sw_version == rhs.sw_version;
        bool addr_equal = lhs.addr == rhs.addr;

        return model_equal && num_locks_equal && start_equal && stop_equal && sw_version_equal && addr_equal;
    }
}