#include "kitezh/vessel.hpp"

#include <stdexcept>

namespace kitezh {

std::optional<std::string> validateVessel(const Vessel& vessel) {
    // throw std::logic_error("validateVessel: реализуйте");

    if (vessel.call_sign.empty()) 
        return std::optional<std::string>("empty call sign");
    if (vessel.call_sign.find('\t') != vessel.call_sign.npos || 
        vessel.call_sign.find(' ') != vessel.call_sign.npos || 
        vessel.call_sign.find('\n') != vessel.call_sign.npos)
        return std::optional<std::string>("bad call sign");
    if (vessel.length_m <= 0 || vessel.length_m > kChamberLengthM)
        return std::optional<std::string>("bad length");
    if (vessel.draft_dm <= 0) 
        return std::optional<std::string>("bad draft");
    return std::nullopt;
}

bool fitsInChamber(const std::vector<Vessel>& inside, const Vessel& next) {
    // throw std::logic_error("fitsInChamber: реализуйте");
    int length = 0;

    for (size_t i = 0; i < inside.size(); i++) {
        length += inside[i].length_m + kGapM;
    }
    
    return length + next.length_m <= kChamberLengthM;
}

}  // namespace kitezh
