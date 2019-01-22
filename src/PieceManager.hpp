#ifndef PieceManager_hpp
#define PieceManager_hpp

#include <vector>
#include <map>

#include "types.hpp"
        
class PieceManager {
public:
    static PieceManager& get() {
        static PieceManager unique;
        return unique;
    }
    
    int num_pieces = 21;
    
    std::map<int, Piece> pieces;
    
    
private:
    PieceManager();
    
    // this reads blocks as matrices from .txt file
    std::map<int, Matrix> _default_blocks_setup();
    
    // for each block, this works out all possible rotations/flip,
    // and also calculates the corners, neighbors and diagonals for each rot/flip
    std::vector<Matrix> _augment(const Matrix& block);
    
    bool _shape_match(const Matrix& l, const Matrix& r);
};

#endif /* PieceManager_hpp */
