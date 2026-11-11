#include "kitezh/shift.hpp"

#include "kitezh/lock.hpp"

#include <stdexcept>
#include <set>
#include <utility>

namespace kitezh {

ShiftReport runNightShift(const std::vector<Vessel>& queue) {
    // 1. Слева направо вычеркнуть duplicate, затем invalid, затем draft.
    //    Вычеркнутые не стоят в очереди и не рвут партию.
    // 2. Пока кто-то ждёт, собрать одну партию по правилам README.
    // 3. Провести её объектом Lock. Следующую начинать с закрытых затворов.
    // 4. pump_cycles взять из lock.pumpCycles() в конце, не из отдельного счётчика.
    // throw std::logic_error("runNightShift: реализуйте");

    std::set<std::string> was;
    std::vector<Vessel> waiting;

    std::vector<std::string> passed;
    std::vector<Refusal> refused;

    for (size_t i = 0; i < queue.size(); i++) {
        Vessel v = queue[i];

        if (was.count(v.call_sign) != 0) {
            refused.push_back({v.call_sign, "duplicate"});
            continue;
        }
        was.insert(v.call_sign);
        
        if (validateVessel(v)) {
            refused.push_back({v.call_sign, "invalid"});
            continue;
        }

        if (v.draft_dm > kLowDepthDm) {
            refused.push_back({v.call_sign, "draft"});
            continue;
        }

        waiting.push_back(v);
    }

    Lock lock;

    while (!waiting.empty()) {
        std::vector<Vessel> batch;
        auto D = waiting.front().direction;
        bool skipped = false;

        batch.push_back(waiting.front());

        if (waiting.front().kind != VesselKind::Oversized) {
            for (size_t i = 1; i < waiting.size(); i++) {
                if (waiting[i].direction == D &&
                      waiting[i].kind != VesselKind::Oversized &&
                      fitsInChamber(batch, waiting[i]) &&
                      (!skipped || waiting[i].kind == VesselKind::Ambulance))
                    batch.push_back(waiting[i]);
                else skipped = true;
            }   
        }

        std::vector<Vessel> new_waiting;
        for (auto w_v : waiting) {
            bool founded = false;

            for (auto v : batch) {
                if (w_v.call_sign == v.call_sign) {
                    founded = true;
                    break;
                }
                    
            }

            if (!founded)
                new_waiting.push_back(w_v);
        }

        if (D == Direction::Upstream) {
            if (lock.level() == Level::High)
                lock.pump();
            
            lock.open(Gate::Lower);

            for (auto v : batch)
                lock.enter(v);

            lock.close(Gate::Lower);

            lock.pump();

            lock.open(Gate::Upper);

            for (auto v : batch) {
                lock.exit(v.call_sign);
                passed.push_back(v.call_sign);
            }
            
            lock.close(Gate::Upper);
        }
        else {
            if (lock.level() == Level::Low)
                lock.pump();

            lock.open(Gate::Upper);

            for (auto v : batch)
                lock.enter(v);

            lock.close(Gate::Upper);

            lock.pump();

            lock.open(Gate::Lower);

            for (auto v : batch) {
                lock.exit(v.call_sign);
                passed.push_back(v.call_sign);
            }

            lock.close(Gate::Lower);
        }

        waiting = std::move(new_waiting);
    }

    return ShiftReport({passed, refused, lock.pumpCycles()});
}

}  // namespace kitezh
