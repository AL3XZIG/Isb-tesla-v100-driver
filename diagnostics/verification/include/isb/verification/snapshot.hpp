#ifndef ISB_VERIFICATION_SNAPSHOT_HPP
#define ISB_VERIFICATION_SNAPSHOT_HPP

#include "isb/verification/types.hpp"

namespace isb::verification {

SnapshotDiff compareSnapshots(const Snapshot& baseline, const Snapshot& actual);

} // namespace isb::verification

#endif // ISB_VERIFICATION_SNAPSHOT_HPP
