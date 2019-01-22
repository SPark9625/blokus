#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "PieceManager.hpp"
#include "types.hpp"
#include "utils.hpp"

std::map<int, Matrix> PieceManager::_default_blocks_setup() {
    std::map<int, Matrix> data;
    
    std::stringstream text = util::get_text();
    
    for (int k = 0; k < num_pieces; k++) {
        
        std::string line;
        std::getline(text, line);
        
        assert(line.find("{") == 0);
        int idx = std::stoi(line.substr(1));
        
        std::getline(text, line);
        util::ltrim(line);
        
        int nrow = int(line[0] - '0');
        int ncol = int(line[1] - '0');
        
        Matrix block = xt::empty<int>({nrow, ncol});
        for (int i = 0; i < nrow; i++) {
            
            std::getline(text, line);
            util::ltrim(line);
            
            for (int j = 0; j < ncol; j++) {
                block(i,j) = int(line[j] - '0');
            }
        }
        
        std::getline(text, line);
        util::ltrim(line);
        
        assert(line.find("}") == 0);
        
        
        data.emplace(idx, block);
    }
    return data;
}


std::vector<Matrix> PieceManager::_augment(const Matrix& block) {
    long nrow = block.shape()[0], ncol = block.shape()[1];
    
    Matrix corners   = xt::zeros<int>({nrow, ncol});
    Matrix neighbors = xt::zeros<int>({nrow + 2, ncol + 2});
    Matrix diagonals = xt::zeros<int>({nrow + 2, ncol + 2});
    
    size_t last_row = nrow - 1;
    size_t last_col = ncol - 1;
    
    // for neighbors and diagonals
    size_t offset = 1;
    
    for (size_t i = 0; i < nrow; i++) {
        for (size_t j = 0; j < ncol; j++) {
            if (block(i, j)) {
                
                // check which neighbors are empty. (0 means empty)
                bool up    = (!i            || !block(i - 1, j) ? true : false);
                bool down  = (i == last_row || !block(i + 1, j) ? true : false);
                bool left  = (!j            || !block(i, j - 1) ? true : false);
                bool right = (j == last_col || !block(i, j + 1) ? true : false);
                
                // This overwrites multiple times and is very inefficient,
                // but it's still OK since we only do this once during init
                if (up)    neighbors(offset + i - 1, offset + j) = 1;
                if (down)  neighbors(offset + i + 1, offset + j) = 1;
                if (left)  neighbors(offset + i, offset + j - 1) = 1;
                if (right) neighbors(offset + i, offset + j + 1) = 1;
                
                
                if ((up && right) || (right && down) || (down && left) || (left && up)) {
                    corners(i, j) = 1;
                    
                    if (up && right) {
                        diagonals(offset + i - 1, offset + j + 1) = 1;
                    }
                    if (right && down) {
                        diagonals(offset + i + 1, offset + j + 1) = 1;
                    }
                    if (down && left) {
                        diagonals(offset + i + 1, offset + j - 1) = 1;
                    }
                    if (left && up) {
                        diagonals(offset + i - 1, offset + j - 1) = 1;
                    }
                }
            }
        }
    }
    
    return std::vector<Matrix>{block, corners, neighbors, diagonals};
}


PieceManager::PieceManager() {
    // block_data contains all 21 blocks (no rot/flip)
    std::map<int, Matrix> block_data = _default_blocks_setup();
    
    for (auto it = block_data.begin(); it != block_data.end(); it++) {
        // piece maps rotflip(vector) to augmented(vector of matrices)
        Piece piece;
        
        bool already_in;
        for (int r = 0; r < 4; r++) {
            
            // rotation
            Matrix rot_block;
            
            if (r == 0) {
                rot_block = it->second;
            } else if (r == 1) {
                rot_block = xt::rot90<1>(it->second);
            } else if (r == 2) {
                rot_block = xt::rot90<2>(it->second);
            } else {
                rot_block = xt::rot90<3>(it->second);
            }
            
            already_in = false;
            for (auto p_it = piece.begin(); p_it != piece.end(); p_it++) {
                Matrix m = p_it->second[0];
                
                if (!_shape_match(rot_block, m)) continue;
                
                if (xt::all(xt::equal(rot_block, m))) {
                    already_in = true;
                    break;
                }
            }
            if (!already_in) {
                int f = 0;
                piece[std::tie(r, f)] = _augment(rot_block);
            }
            
            // flip
            Matrix flip_block = xt::flip(rot_block, 1);
            
            already_in = false;
            for (auto p_it = piece.begin(); p_it != piece.end(); p_it++) {
                Matrix m = p_it->second[0];
                
                if (!_shape_match(rot_block, m)) continue;
                
                if (xt::all(xt::equal(flip_block, m))) {
                    already_in = true;
                    break;
                }
            }
            if (!already_in) {
                int f = 1;
                piece[std::tie(r, f)] = _augment(flip_block);
            }
        }
        pieces[it->first] = piece;
    }
}

bool PieceManager::_shape_match(const Matrix& l, const Matrix& r) {
    for (int i = 0; i < 2; i++) {
        if (l.shape()[i] != r.shape()[i]) return false;
    }
    return true;
}
