#include "pybind11/pybind11.h"            // Pybind11 import to define Python bindings
#include "pybind11/stl.h"                 // map, vector, etc
#define FORCE_IMPORT_ARRAY                // numpy C api loading
#include "xtensor-python/pytensor.hpp"    // Numpy bindings

#include "Environment.hpp"
#include "types.hpp"

PYBIND11_MODULE(Environment, m)
{
    xt::import_numpy();
    m.doc() = "Blokus game environment";
    
    pybind11::class_<Blokus>(m, "Blokus")
    .def(pybind11::init<>())
    .def("reset", &Blokus::reset)
    .def("step", &Blokus::pystep, pybind11::arg("state"), pybind11::arg("action"))
    .def("place_possible", &Blokus::pyplace_possible, pybind11::arg("board"), pybind11::arg("player"), pybind11::arg("action"))
    // would be nice to add printState as well.
    // .def_readonly("pieces", &Blokus::pieces)
    .def_readonly("SIZE", &Blokus::SIZE)
    .def_readonly("N_PLAYERS", &Blokus::N_PLAYERS)
    .def_readonly("N_PIECES", &Blokus::N_PIECES)
    .def_readonly("N_STATE_LAYER", &Blokus::N_STATE_LAYER)
    .def_readonly("N_ACTION_LAYER", &Blokus::N_ACTION_LAYER)
    .def_readonly("player_list", &Blokus::player_list)
    .def_readonly("BOARD_SHAPE", &Blokus::BOARD_SHAPE)
    .def_readonly("ACTION_SHAPE", &Blokus::ACTION_SHAPE)
    .def_readonly("layer2irf", &Blokus::layer2irf)
    .def_readonly("irf2layer", &Blokus::irf2layer)
    .def_readonly("DIAGONAL", &Blokus::DIAGONAL)
    .def_readonly("NEIGHBOR", &Blokus::NEIGHBOR)
    .def_readonly("FIRST", &Blokus::FIRST)
    .def_readonly("DONE", &Blokus::DONE)
    .def_readonly("TURN", &Blokus::TURN)
    ;
}
