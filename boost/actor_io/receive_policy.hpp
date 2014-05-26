#ifndef RECEIVE_POLICY_H
#define RECEIVE_POLICY_H

#include <cstddef>
#include <utility>

namespace boost {
namespace actor_io {

enum class receive_policy_flag { at_least, at_most, exactly };

struct receive_policy {
    using config = std::pair<receive_policy_flag, size_t>;
    static inline config at_least(size_t num_bytes) {
        return {receive_policy_flag::at_least, num_bytes};
    }
    static inline config at_most(size_t num_bytes) {
        return {receive_policy_flag::at_most, num_bytes};
    }
    static inline config exactly(size_t num_bytes) {
        return {receive_policy_flag::exactly, num_bytes};
    }
};

} // namespace actor_io
} // namespace boost

#endif // RECEIVE_POLICY_H
