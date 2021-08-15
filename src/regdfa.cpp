#include <queue>

#include "regdfa.hpp"
#include "util.hpp"

namespace std {
    template <>
    struct hash<RegPosSets::SymbolPosSet> {
        size_t operator()(const RegPosSets::SymbolPosSet &set) const {
            using Hasher = std::hash<RegPosSets::SymbolPos>;

            size_t hash = 0;

            auto beg = set.begin(), end = set.end();

            if (beg == end) {
                return hash;
            }

            Hasher hasher;
            hash = hasher(*beg++);

            for (; beg != end; ++beg) {
                hash = combine_hashes(hash, hasher(*beg));
            }

            return hash;
        }
    };
}

RegPosSets::SymbolPosSet intersect_pos_sets(
    RegPosSets::SymbolPosSet a,
    RegPosSets::SymbolPosSet b
) {
    RegPosSets::SymbolPosSet result;
    for (auto &&it : a) {
        if (b.find(it) != b.end()) {
            result.emplace(it);
        }
    }

    return result;
}

RegDFA::RegDFA(const RegPosSets &pos_sets) {
    using Hasher = std::hash<RegPosSets::SymbolPosSet>;
    using PosSetAndState = std::pair<RegPosSets::SymbolPosSet, State>;

    Hasher hasher;
    std::queue<PosSetAndState> unvisited;
    std::unordered_set<RegPosSets::SymbolPosSet> visited;
    RegPosSets::SymbolPosSet new_set;

    auto first_set = pos_sets.first();
    auto first_state = hasher(first_set);

    dfa_current_state = first_state;
    unvisited.push({ first_set, first_state });

    do {
        auto &[current_set, current_state] = unvisited.front();

        visited.emplace(current_set);

        for (auto &&[symbol, sym_pos_set] : pos_sets.symbol_occurrences()) {
            new_set.clear();

            auto valid_pos_set = intersect_pos_sets(sym_pos_set, current_set);

            for (auto &&pos : valid_pos_set) {
                auto &follow_pos = pos_sets.follow(pos);

                new_set.insert(
                    follow_pos.cbegin(),
                    follow_pos.cend()
                );
            }

            if (new_set.empty()) {
                continue;
            }

            auto new_state = hasher(new_set);

            if (visited.find(new_set) == visited.end()) {
                unvisited.push({new_set, new_state});
            }

            StateTransition transition(current_state, symbol);
            dfa_table.emplace(transition, new_state);
        }

        unvisited.pop();
    } while (!unvisited.empty());
}

RegDFA::TransitionResult RegDFA::do_transition(const RegSymbol symbol) {
    StateTransition transition(dfa_current_state, symbol);

    auto next_state_it = dfa_table.find(transition);

    if (next_state_it != dfa_table.end()) {
        dfa_current_state = next_state_it->second;
        return TransitionResult::SUCCESS;
    } else {
        return TransitionResult::DEADSTATE;
    }
}

void RegDFA::set_pos_action(RegPosSets::SymbolPos pos, RegDFA::PosAction action) {
    auto &&[it, _] = pos_actions.try_emplace(pos, PosActions());
    it->second.push_back(action);
}

const RegDFA::PosActions &RegDFA::current_pos_actions() const {
    static PosActions no_actions;

    auto it = pos_actions.find(dfa_current_state);

    if (it != pos_actions.end()) {
        return it->second;
    } else {
        return no_actions;
    }
}

RegDFA::State RegDFA::current_state() const {
    return dfa_current_state;
}

void RegDFA::set_current_state(RegDFA::State state) {
    dfa_current_state = state;
}
