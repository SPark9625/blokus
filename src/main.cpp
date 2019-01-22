#include <Python.h>
#define FORCE_IMPORT_ARRAY

#include <iostream>
#include <chrono>
#include <numeric>
#include <random>
#include <set>
#include <tuple>

#include "xtensor-python/pytensor.hpp"
#include "xtensor/xio.hpp"

#include "Environment.hpp"
#include "PieceManager.hpp"

#include "types.hpp"
#include "config.hpp"


using namespace std;
using namespace py;

tuple<State, bool> take_action(Blokus& env, State& state, Action action, int& turn, int verbose = 1);

Action get_action(const map<IRF,int>& irf2layer);

/* ---------------------------------- */
//
//                 main
//
/* ---------------------------------- */


int main() {
    Py_Initialize();
    xt::import_numpy();

    Blokus env;
    State state = env.reset();
    py::config& conf = config::get();


    int turn = 0;
    Reward reward;
    bool done;
    Action action;
    while (true) {
        while (true) {
            action = get_action(env.irf2layer);
            if (env.place_possible(get<0>(state), get<0>(state)(0, 0, conf.turn), action)) {
                break;
            } else {
                cout << get<0>(action) << ',' << get<1>(action) << ',' << get<2>(action) << endl;
                cout << "Try again." << endl;
            }
        }


        tie(state, done) = take_action(env, state, action, turn);
        if (done) break;
    }

    Py_Finalize();


    
    // cout << "get State" << endl;
    return 0;
}









tuple<State, bool> take_action(Blokus& env, State& state, Action action, int& turn, int verbose) {
    int player;
    char name;
    set<int> rem_pieces;
    set<Action> actions;
    bool done;

    if (verbose > 0) {
        turn++;
        player = get<0>(state)(0, 0, env.conf.turn);
        name = (player ? 'X' : 'O');
        actions = get<1>(get<1>(state))[player];
        rem_pieces = get<3>(get<1>(state))[player];
    }


    tuple<State, Reward, Done> next = env.step(state, action);
    state = get<0>(next);

    if (verbose > 0) {
        Reward reward = get<1>(next);
        done = get<2>(next);


        // Print basic info
        cout << "Turn " << turn << endl;
        cout << "Player " << player << '(' << name << "), Action: (";
        cout << get<0>(action) << ',' << get<1>(action) << ',' << get<2>(action) << ')' << endl;


        // Print remaining pieces
        cout << "Remaining pieces: [";
        for (int pc : rem_pieces) cout << pc << ' ';
        cout << ']' << endl;


        // Print actions if less than 10
        if (actions.size() < 10) {
            cout << endl;
            for (Action action: actions) {
                cout << '(' << get<0>(action) << ',' << get<1>(action) << ',' << get<2>(action) << ')' << endl;
            }
        }


        // Print state
        env.printState(state, verbose);


        // Print reward
        cout << "reward: [ ";
        for (int r: reward) cout << r << ' ';
        cout << "], ";


        // Print done
        if (done) cout << "done: true" << endl;
        else cout << "done: false" << endl;
        player = get<0>(state)(0, 0, env.conf.turn);
        cout << "Now player: " << player << endl;


        cout << endl;
    }

    return tie(state, done);
}



Action get_action(const map<IRF,int>& irf2layer) {
    string s;
    cout << "Enter action: ";
    getline(cin, s);

    int idx, r, f, i, j;
    vector<int*> dim5{&idx, &r, &f, &i, &j};

    string a;
    for (int k = 0; k < 5; k++) {
        auto idx = s.find(',');
        a = s.substr(0, idx);
        s = s.substr(idx + 1);
        *dim5[k] = stoi(a);
    }

    int layer = irf2layer.at({idx, r, f});
    Action action = {i, j, layer};

    return action;
}
