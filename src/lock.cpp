#include "kitezh/lock.hpp"

#include <stdexcept>

namespace kitezh {

Lock::Lock() {
    // throw std::logic_error("Lock: реализуйте");

    UpperGate_ = Gate::Upper;
    UpperGateState_ = GateState::Closed;

    LowerGate_ = Gate::Lower;
    LowerGateState_ = GateState::Closed;

    Level_ = Level::Low;
    PumpCount_ = 0; 

    Inside_ = std::vector<Vessel>();
}

Level Lock::level() const {
    // throw std::logic_error("Lock::level: реализуйте");

    return Level_;
}

GateState Lock::gate(Gate gate) const {
    // throw std::logic_error("Lock::gate: реализуйте");

    if (gate == Gate::Lower)
        return LowerGateState_;
    return UpperGateState_;
}

int Lock::pumpCycles() const {
    // throw std::logic_error("Lock::pumpCycles: реализуйте");

    return PumpCount_;
}

std::vector<Vessel> Lock::inside() const {
    // throw std::logic_error("Lock::inside: реализуйте");

    return Inside_;
}

void Lock::open(Gate gate) {
    // Недопустимое открытие бросает LockError и не меняет камеру.
    // throw std::logic_error("Lock::open: реализуйте");

    if (gate == Gate::Lower && 
        Level_ == Level::Low &&
        UpperGateState_ == GateState::Closed)
        LowerGateState_ = GateState::Open;
    else if (gate == Gate::Upper &&
             Level_ == Level::High &&
             LowerGateState_ == GateState::Closed)
        UpperGateState_ = GateState::Open;
    else throw kitezh::LockError("open error");
}

void Lock::close(Gate gate) {
    // throw std::logic_error("Lock::close: реализуйте");

    if (gate == Gate::Lower)
        LowerGateState_ = GateState::Closed;
    else if (gate == Gate::Upper)
        UpperGateState_ = GateState::Closed;
    else throw kitezh::LockError("close error");
}

void Lock::pump() {
    // Отказ насоса не тратит такт и не сажает уже вошедшее судно на грунт.
    // throw std::logic_error("Lock::pump: реализуйте");

    if (LowerGateState_ == GateState::Closed && UpperGateState_ == GateState::Closed) {
        int max_draft_dm = 0;
        for (Vessel v : Inside_)
            max_draft_dm = std::max(max_draft_dm, v.draft_dm);
        
        if (Level_ == Level::High) {
            if (max_draft_dm <= kLowDepthDm) {
                Level_ = Level::Low;
            }
            else throw kitezh::LockError("pump error");
        }
        
        else if (Level_ == Level::Low) {
            if (max_draft_dm <= kHighDepthDm) {
                Level_ = Level::High;
            }
            else throw kitezh::LockError("pump error");
        }

        PumpCount_++;
    }
    else throw kitezh::LockError("pump error");
}

void Lock::enter(const Vessel& vessel) {
    // Внутри должна оказаться копия. Входной затвор сам не открывается.
    // throw std::logic_error("Lock::enter: реализуйте");

    if (validateVessel(vessel))
        throw kitezh::LockError("enter error");

    if (!fitsInChamber(Inside_, vessel))
        throw kitezh::LockError("enter error");

    for (Vessel v : Inside_) {
        if (v.call_sign == vessel.call_sign)
            throw kitezh::LockError("enter error");
        if (v.kind == VesselKind::Oversized)
            throw kitezh::LockError("enter error");
    }

    if (Level_ == Level::High) {
        if (vessel.draft_dm > kHighDepthDm || UpperGateState_ != GateState::Open)
            throw kitezh::LockError("enter error"); 
    }
    else if (Level_ == Level::Low) {
        if (vessel.draft_dm > kLowDepthDm || LowerGateState_ != GateState::Open)
            throw kitezh::LockError("enter error"); 
    }

    if (vessel.kind == VesselKind::Oversized && !Inside_.empty())
        throw kitezh::LockError("enter error");

    if (vessel.direction == Direction::Upstream && LowerGateState_ != GateState::Open)
        throw kitezh::LockError("enter error");
    else if (vessel.direction == Direction::Downstream && UpperGateState_ != GateState::Open)
        throw kitezh::LockError("enter error");

    Inside_.push_back(vessel);
}

void Lock::exit(const std::string& call_sign) {
    // Выходит только голова очереди и только в свои ворота.
    // throw std::logic_error("Lock::exit: реализуйте");

    if (Inside_.empty() || call_sign != Inside_.front().call_sign)
        throw kitezh::LockError("exit error");

    if (Inside_.front().direction == Direction::Upstream && UpperGateState_ == GateState::Closed)
        throw kitezh::LockError("exit error");
    else if (Inside_.front().direction == Direction::Downstream && LowerGateState_ == GateState::Closed)
        throw kitezh::LockError("exit error");

    Inside_.erase(Inside_.begin());
}

}  // namespace kitezh
