#ifndef config_hpp
#define config_hpp

#include <string>
#include <iostream>
#include <cmath>
#include <map>
#include <tuple>
#include <vector>

#include "PieceManager.hpp"
#include "types.hpp"
        
class config {
private:
    const PieceManager& pmanager = PieceManager::get();
    config(){};
    
    
public:
    static config& get() {
        static config unique;
        return unique;
    }
    
    /* ------------------------ YOU CAN MODIFY ------------------------ */
    
    
    int num_players = 2;
    //    int howlongtokeeptau1 = 5;
    
    
    /* ----------------------------- END ------------------------------ */
    
    
    
    
    
    
    
    
    /* ------------------------- DO NOT TOUCH ------------------------- */
    std::map<int, Piece> pieces  = pmanager.pieces;
    int num_pieces = 21;
    
    
    std::vector<IRF> layer2irf   = get_layer2irf();
    std::map<IRF, int> irf2layer = get_irf2layer(layer2irf);
    
    std::vector<int> player_list = get_player_list();
    
    
    int num_state_layers  = num_players * (num_pieces + 4) + 1;
    int num_action_layers = (int) layer2irf.size(); // 91
    
    
    int board_size = (num_players == 2 ? 13 : 20);
    std::string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string PREFIX = ALPHABET.substr(0, board_size);
    
    
    std::vector<int> board_shape{num_state_layers, board_size, board_size};
    std::vector<int> action_shape{num_action_layers, board_size, board_size};
    
    
    int diagonal = num_players * num_pieces; // 42 - 43
    int neighbor = diagonal + num_players;   // 44 - 45
    int first = neighbor + num_players;      // 46 - 47
    int done = first + num_players;          // 48 - 49
    int turn = 50;                           // 50
    
    
    float alpha = 10 / pow(board_size, 2);
    std::set<int> all_pieces{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    /* ----------------------------- END ------------------------------ */
    
    
    
    
    
    
    
    
    
    
    
    
private:
    std::vector<IRF> get_layer2irf()
    {
        std::vector<IRF> layer2irf;
        for (int i = 0; i < num_pieces; i++) {
            for (auto it = pieces[i].begin(); it != pieces[i].end(); it++) {
                int r = std::get<0>(it->first);
                int f = std::get<1>(it->first);
                layer2irf.push_back(std::tie(i,r,f));
            }
        }
        return layer2irf;
    }
    
    
    std::map<IRF, int> get_irf2layer(const std::vector<IRF>& layer2irf)
    {
        std::map<IRF, int> irf2layer;
        
        for (int layer = 0; layer < layer2irf.size(); layer++) {
            irf2layer[layer2irf[layer]] = layer;
        }
        
        return irf2layer;
    }
    
    
    std::vector<int> get_player_list()
    {
        std::vector<int> plist(num_players);
        for (int i = 0; i < num_players; i++)
            plist[i] = i;
        return plist;
    }
};

#endif /* config_hpp */
