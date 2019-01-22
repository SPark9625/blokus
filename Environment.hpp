#ifndef Environment_hpp
#define Environment_hpp

#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "config.hpp"
#include "types.hpp"

class Blokus {
public:
    Blokus();
    
    // returns State
    State reset() const;
    
    // returns tuple that contains {State, reward, done}
    std::tuple<State, Reward, Done> step(const State& state, const Action& action) const;
    
    // self-explanatory
    bool place_possible(const Board& board, int player, const Action& action) const;
    
#ifdef _PYTHON
    void printState(const pyState& state, int verbosity = 1) const;


    pyState pyreset() const;
    
    
    std::tuple<pyState, Reward, Done> pystep(const pyState& state, const Action& action) const;
    
    
    bool pyplace_possible(const pyBoard& board, int player, const Action& action) const;
#endif
    
private:
    // returns list of 5-dim tuples
    std::set<Action> _possible_actions(const State& state, int player) const;
    
    // removes dead actions for a player
    std::set<Action>& _update_actions(State& state, int player) const;
    
    // self-explanatory
    bool _check_game_finished(State& state) const;
    
    // returns augmented{block, corners, neighbors, diagonals}
    const std::vector<Matrix>& _adjust(const Piece& piece, int r, bool f) const;
    
    void _printMatrix(const Matrix& board) const;
    
    
public:
    // self variables
    const config& conf = config::get();
    const std::map<int, Piece> pieces;
    
    unsigned long SIZE;
    
    const int N_PLAYERS;
    const int N_PIECES;
    const int N_STATE_LAYER;
    const int N_ACTION_LAYER;
    
    const std::vector<int> player_list;
    const std::vector<int> BOARD_SHAPE;
    const std::vector<int> ACTION_SHAPE;
    
    const std::vector<IRF> layer2irf;
    const std::map<IRF, int> irf2layer;
    
    const int DIAGONAL;
    const int NEIGHBOR;
    const int FIRST;
    const int DONE;
    const int TURN;
    
};
    
#endif /* Environment_hpp */
