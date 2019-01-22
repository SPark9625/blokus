#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "xtensor/xview.hpp"
#include "xtensor/xio.hpp"

#include "Environment.hpp"
#include "config.hpp"
#include "types.hpp"

#include "xtensor/xtensor.hpp"
#include "xtensor/xadapt.hpp"

#ifdef _PYTHON
#include "xtensor-python/pytensor.hpp"
#endif // _PYTHON



Blokus::Blokus()
  : pieces(conf.pieces),
    SIZE(conf.board_size),

    N_PLAYERS(conf.num_players),
    N_PIECES(conf.num_pieces),
    N_STATE_LAYER(conf.num_state_layers),
    N_ACTION_LAYER(conf.num_action_layers),

    player_list(conf.player_list),
    BOARD_SHAPE(conf.board_shape),
    ACTION_SHAPE(conf.action_shape),

    layer2irf(conf.layer2irf),
    irf2layer(conf.irf2layer),

    DIAGONAL(conf.diagonal),
    NEIGHBOR(conf.neighbor),
    FIRST(conf.first),
    DONE(conf.done),
    TURN(conf.turn)
{
    // pass
}


State Blokus::reset() const
{
    Board board = xt::zeros<int>(BOARD_SHAPE);
    
    // Two player for now
    std::vector<NewDiagonals> first_pos{
        {{0,0}},
        {{SIZE - 1, SIZE - 1}}
    };
    
    for (int pl : player_list) {
        // diagonal layers
        Point pt = first_pos[pl][0];
        
        board(DIAGONAL + pl, pt[0], pt[1]) = 1;
        
        // first layers
        xt::view(board, FIRST + pl, xt::all(), xt::all()) = 1;
    }
    
    // NewDiagonals, Actions_all, Remaining_Pieces_all, Meta
    ID root_id{std::make_tuple(-1,-1,-1)};
    
    std::vector<std::set<Action>> actions_all(N_PLAYERS);
    
    std::vector<std::set<int>> rem_pieces_all(N_PLAYERS);
    std::fill(rem_pieces_all.begin(), rem_pieces_all.end(), conf.all_pieces);
    
    
    Meta meta = std::tie(root_id, actions_all, first_pos, rem_pieces_all);
    
    State state = std::tie(board, meta);
    
    for (int pl : player_list) {
        std::get<1>(std::get<1>(state))[pl] = _possible_actions(state, pl);
    }
    return state;
}


std::tuple<State, Reward, Done> Blokus::step(const State& state, const Action& action) const
{
    Board next_board = std::get<0>(state);
    int player = next_board(TURN, 0, 0);
    
    assert(place_possible(next_board, player, action));
    
    // Metadata
    const Meta& prev_meta = std::get<1>(state);
    
    // Everything should be copied
    ID id = std::get<0>(prev_meta);
    id.push_back(action);
    std::vector<std::set<Action>> actions_all = std::get<1>(prev_meta);
    std::vector<NewDiagonals> new_diagonals_all = std::get<2>(prev_meta);
    std::vector<std::set<int>> rem_pcs_all = std::get<3>(prev_meta);
    std::set<int>& rem_pcs = rem_pcs_all[player];
    
    int idx, r, f, i, j, layer;
    std::tie(layer, i, j) = action;
    std::tie(idx, r, f) = layer2irf[layer];
    
    assert(std::find(rem_pcs.begin(), rem_pcs.end(), idx) != rem_pcs.end());
    
    // Stuff to care about if this is one's first turn.
    if (next_board(FIRST + player, 0, 0)) {
        // Set other corner spots to zero
        next_board(DIAGONAL + player, 0       , 0       ) = 0;
        next_board(DIAGONAL + player, 0       , SIZE - 1) = 0;
        next_board(DIAGONAL + player, SIZE - 1, 0       ) = 0;
        next_board(DIAGONAL + player, SIZE - 1, SIZE - 1) = 0;
        
        // Set FIRST layer to zero
        xt::view(next_board, FIRST + player, xt::all(), xt::all()) = 0;
    }
    
    const Piece& piece = pieces.at(idx);
    const std::vector<Matrix>& block_info = (piece.find(std::tie(r,f)) == piece.end()) ?
    _adjust(piece, r, f) : piece.at(std::tie(r,f));
    const Matrix& block     = block_info[0];
    const Matrix& neighbors = block_info[2];
    const Matrix& diagonals = block_info[3];
    
    // 1. Adding block. Assumes a valid action, thus just add
    for (auto p : xt::argwhere(block)) {
        
        long m = i + p[0], n = j + p[1];
        
        next_board(N_PIECES * player + idx, m, n)++;
        
        // Remove any existing diagonals overlapping with the new piece.
        for (int pl : player_list) {
            next_board(DIAGONAL + pl, m, n) = 0;
        }
        
    }
    
    
    // 2. Adding neighbors -- this ignores the neighbors matrix shading over other pieces
    for (auto p : xt::argwhere(neighbors)) {
        
        long m = i + p[0] - 1, n = j + p[1] - 1;
        
        if (m >= 0 && n >= 0 && m < SIZE && n < SIZE) {
            
            // Adding neighbors
            next_board(NEIGHBOR + player, m, n) = 1;
            
            // Remove any existing diagonals of this player overlapping with the new neighbors
            next_board(DIAGONAL + player, m, n) = 0;
            
        }
    }
    
    
    // 3. Adding diagonals -- must be perfectly accurate
    NewDiagonals new_diagonals;
    for (auto p : xt::argwhere(diagonals)) {
        
        auto m = i + p[0] - 1, n = j + p[1] - 1;
        
        if (m >= 0 && n >= 0 && m < SIZE && n < SIZE) {
            bool new_diag = true;
            
            // If this place is neighboring one's own color, or is already a diagonal, ignore
            if (next_board(NEIGHBOR + player, m, n) ||
                next_board(DIAGONAL + player, m, n)) {
                continue;
            }
            
            // If this place already exists on the main board, ignore
            for (int l = 0; l < DIAGONAL; l++) {
                
                if (next_board(l, m, n)) {
                    new_diag = false;
                    break;
                }
                
            }
            if (new_diag) {
                
                new_diagonals.push_back({m, n});
                next_board(DIAGONAL + player, m, n) = 1;
                
            }
        }
    }
    
    // Update actions_all to remove current piece idx
    std::set<Action>& player_actions = actions_all[player];
    for (auto it = player_actions.begin(); it != player_actions.end(); ) {
        
        int action_idx = std::get<0>(layer2irf[std::get<0>(*it)]);
        
        if (action_idx == idx) {
            it = player_actions.erase(it);
        } else {
            it++;
        }
        
    }
    
    new_diagonals_all[player] = new_diagonals;
    rem_pcs.erase(idx);
    
    Meta next_meta = std::tie(id, actions_all, new_diagonals_all, rem_pcs_all);
    State next_state = std::tie(next_board, next_meta);
    
    Reward reward(N_PLAYERS);
    for (int i = 0; i < N_PLAYERS; i++)
        reward[i] = 0;
    
    // This also updates current player's possible actions
    // and removes any dead actions for the next player
    bool game_done = _check_game_finished(next_state);
    
    
    if (game_done) {
        std::vector<int> scores;
        for (int p : player_list) {
            
            auto focus = xt::view(next_board, xt::range(N_PIECES * p, N_PIECES * (p + 1)), xt::all(), xt::all());
            double sum = xt::sum(focus)[0]; // [0] required to get the value as a double
            
            scores.push_back(sum);
            
        }
        // Assuming two players
        reward = Reward{1, -1};
        if (scores[0] < scores[1])
            std::reverse(reward.begin(), reward.end());
    }
    return std::tie(next_state, reward, game_done);
}


bool Blokus::place_possible(const Board& board, int player, const Action& action) const
{
    int idx, r, f, i, j, layer;
    std::tie(layer, i, j) = action;
    std::tie(idx, r, f) = layer2irf[layer];
    
    const Piece& piece = pieces.at(idx);
    
    const std::vector<Matrix>& block_info = (piece.find(std::tie(r,f)) == piece.end()) ?
    _adjust(piece, r, f) : piece.at(std::tie(r,f));
    const Matrix& block = block_info[0];
    const Matrix& corners = block_info[1];
    
    for (auto p : xt::argwhere(block)) {
        
        // check if there are any common flat edge
        if (board(NEIGHBOR + player, i + p[0], j + p[1]))
            return false;
        
        // check overlap
        for (int layer = 0; layer < DIAGONAL; layer++) {
            
            if (board(layer, i + p[0], j + p[1]))
                return false;
            
        }
    }
    
    // make sure the corners touch
    for (auto p : xt::argwhere(corners)) {
        
        if (board(DIAGONAL + player, i + p[0], j + p[1]))
            return true;
        
    }
    return false;
}




/* ---------------------------------------------- */
/*                Private methods                 */
/* ---------------------------------------------- */
std::set<Action> Blokus::_possible_actions(const State& state, int player) const
{
    const Board&    board = std::get<0>(state);
    const auto& prev_meta = std::get<1>(state);
    const auto& new_diagonals = std::get<2>(prev_meta)[player];
    
    std::set<Action> actions = std::get<1>(prev_meta)[player];
    
    const std::set<int>& remaining_pieces = std::get<3>(prev_meta)[player];
    
    for (auto it = actions.begin(); it != actions.end(); ) {
        
        if (!place_possible(board, player, *it))
            it = actions.erase(it);
        else
            it++;
    }
    
    
    for (auto diag_pos : new_diagonals) {
        
        for (int idx : remaining_pieces) {
            
            Piece piece = pieces.at(idx);
            for (auto it = piece.begin(); it != piece.end(); it++) {
                
                int r = std::get<0>(it->first);
                int f = std::get<1>(it->first);
                
                const Matrix& block   = (it->second)[0];
                const Matrix& corners = (it->second)[1];
                
                long width  = block.shape()[1];
                long height = block.shape()[0];
                
                for (auto offset : xt::argwhere(corners)) {
                    
                    int i = int(diag_pos[0] - offset[0]);
                    int j = int(diag_pos[1] - offset[1]);
                    
                    // if coord is outside the board, ignore and continue
                    if (i < 0 || j < 0 || i + height > SIZE || j + width > SIZE) {
                        continue;
                    } else {
                        
                        int layer = irf2layer.at(std::tie(idx, r, f));
                        
                        Action action{layer, i, j};
                        if (place_possible(board, player, action)) {
                            actions.insert(action);
                        }
                        
                    }
                }
            }
        }
    }
    return actions;
}


std::set<Action>& Blokus::_update_actions(State& state, int player) const {
    /*
     This just removes any dead actions for a player
     */
    
    const Board& board = std::get<0>(state);
    Meta& prev_meta = std::get<1>(state);
    std::set<Action>& actions = std::get<1>(prev_meta)[player]; // size 3 vectors
    
    for (auto it = actions.begin(); it != actions.end(); ) {
        if (place_possible(board, player, *it))
            it++;
        else
            it = actions.erase(it);
    }
    return actions;
}

bool Blokus::_check_game_finished(State& state) const {
    // 1. Calculate all possible actions for the current player
    // 2. Check the next player (and remove any dead actions)
    // (3. If next player doesn't exist, return true;)
    Board& board = std::get<0>(state); // board should not be const
    bool finished = true;
    
    int cur = board(TURN, 0, 0);
    
    // Update current player's possible actions
    std::set<Action>& cur_actions = std::get<1>(std::get<1>(state))[cur];
    cur_actions = _possible_actions(state, cur);
    
    if (!cur_actions.size()) {
        xt::view(board, DONE + cur, xt::all(), xt::all()) = 1;
    } else {
        finished = false;
    }
    
    // Find the first next player that hasn't finished yet.
    int next = cur + 1;
    for (int pl = 0; pl < N_PLAYERS; pl++) {
        
        int player = (pl + next) % N_PLAYERS;
        
        if (!board(DONE + player, 0, 0)) {
            
            std::set<Action>& player_actions = _update_actions(state, player);
            
            if (player_actions.size() > 0) {
                
                xt::view(board, TURN, xt::all(), xt::all()) = player;
                return false;
                
            } else {
                
                xt::view(board, DONE + player, xt::all(), xt::all()) = 1;
                continue;
                
            }
        }
    }
    
    return finished;
}


const std::vector<Matrix>& Blokus::_adjust(const Piece& piece, int r_, bool f_) const {
    r_ = r_ % 4;
    Matrix block_ = piece.at(std::make_tuple(0,0))[0];
    Matrix target;
    
    if (r_ == 0) {
        target = block_;
    } else if (r_ == 1) {
        target = xt::rot90<1>(block_);
    } else if (r_ == 2) {
        target = xt::rot90<2>(block_);
    } else { // r_ == 3
        target = xt::rot90<3>(block_);
    }
    
    if (f_) target = xt::flip(target, 1);
    
    
    int r, f;
    for (int i = 0; i < 4; i++) {
        Matrix rot;
        if (i == 0) {
            rot = block_;
        } else if (i == 1) {
            rot = xt::rot90<1>(block_);
        } else if (i == 2) {
            rot = xt::rot90<2>(block_);
        } else {  // i == 3
            rot = xt::rot90<3>(block_);
        }
        if (xt::all(xt::equal(rot, target))) {
            r = i;
            f = 0;
            break;
        }
        
        Matrix flip = xt::flip(rot, 1);
        if (xt::all(xt::equal(flip, target))) {
            r = i;
            f = 1;
            break;
        }
    }
    return piece.at(std::tie(r,f));
}
#ifdef _PYTHON
void Blokus::printState(const pyState& state, int verbosity) const {
    if (verbosity == 0) return;
    
    Board board = std::get<0>(state);
    
    Matrix main_board = xt::zeros<int>({SIZE, SIZE});
    for (int pl : player_list) {
        
        xt::xtensor<int,3> v = xt::view(board, xt::range(N_PIECES * pl, N_PIECES * (pl + 1)), xt::all(), xt::all());
        main_board = main_board + (pl + 1) * xt::sum(v, {0});
        
    }
    _printMatrix(main_board);
    std::cout << std::endl;
}
#endif // _PYTHON

void Blokus::_printMatrix(const Matrix& board) const {
    
    std::cout << "   ";
    for (char c : conf.PREFIX) std::cout << c << ' ';
    std::cout << std::endl;
    
    char mark;
    for (int i = 0; i < board.shape()[0]; i++) {
        
        std::cout << std::setw(2) << i << ' ';
        for (int j = 0; j < board.shape()[1]; j++) {
            
            mark = '.';
            if (board(i, j) == 1) {
                mark = 'O';
            } else if (board(i, j) == 2) {
                mark = 'X';
            } else if (board(i, j) == 3) {
                mark = 'Z';
            }
            std::cout << mark << ' ';
            
        }
        std::cout << std::endl;
        
    }
}


#ifdef _PYTHON
pyState Blokus::pyreset() const
{
    return reset();
    // State state = reset();
    // pyState ret;
    // std::get<0>(ret) = xt::adapt(std::get<0>(state).data(), BOARD_SHAPE);
    
    // std::get<1>(ret) = std::get<1>(state);
    // return ret;
}

std::tuple<pyState, Reward, Done> Blokus::pystep(const pyState& state, const Action& action) const
{
    return step(state, action);
    // State s;
    // std::get<0>(s) = xt::adapt(std::get<0>(state).data(), BOARD_SHAPE);
    // std::get<1>(s) = std::get<1>(state);
    
    // State cc_state;
    // Reward reward;
    // Done done;
    
    // std::tie(cc_state, reward, done) = step(s, action);
    // pyState py_state;
    // std::get<0>(py_state) = xt::adapt(std::get<0>(cc_state).data(), BOARD_SHAPE);
    // std::get<1>(py_state) = std::get<1>(cc_state);
    
    // return std::tie(py_state, reward, done);
}

bool Blokus::pyplace_possible(const pyBoard& board, int player, const Action& action) const
{
    return place_possible(board, player, action);
    // Board b = xt::adapt(board.data(), BOARD_SHAPE);
    // return place_possible(b, player, action);
}
#endif // _PYTHON
