#ifndef types_hpp
#define types_hpp

#include <array>
#include <iostream>
#include <map>
#include <set>
#include <tuple>
#include <vector>

#include "xtensor/xtensor.hpp"


// I(ndex), R(otation), F(lip) -- Index is the piece number (0 ~ 20)
using IRF = std::tuple<int,int,int>;
using RotFlip = std::tuple<int,int>;

using Action = std::tuple<int,int,int>;
using ID = std::vector<Action>;

using Reward = std::vector<float>;
using Done = bool;

using Point = std::array<unsigned long, 2>;
using NewDiagonals = std::vector<Point>;

using Piece = std::map<RotFlip, std::vector<xt::xtensor<int, 2>>>;
using Matrix = xt::xtensor<int, 2>;

using Board = xt::xtensor<int, 3>;

// Meta contains the state ID, list of actions (for all players),
using Meta = std::tuple<ID, std::vector<std::set<Action>>, std::vector<NewDiagonals>, std::vector<std::set<int>>>;
using State = std::tuple<Board, Meta>;
using Policy = xt::xtensor<float, 3>;
    
#ifdef _PYTHON
#include "xtensor-python/pytensor.hpp"

using pyBoard = xt::pytensor<int, 3>;
using pyState = std::tuple<pyBoard, Meta>;
using pyPolicy = xt::pytensor<float, 3>;

#endif // _PYTHON
        

#endif /* types_hpp */
