/*
 * This file is a part of TiledArray.
 * Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iomanip>
#include <tiledarray.h>
#include "input_data.h"

using namespace TiledArray;
using namespace TiledArray::expressions;

int main(int argc, char** argv) {
  // Initialize runtime
  madness::World& world = madness::initialize(argc, argv);

  std::string file_name = argv[1];

  // Open input file.
  std::ifstream input(file_name.c_str());

  if(! input.fail()) {

    // Read input data.

    if(world.rank() == 0)
      std::cout << "Reading input...";

    InputData data(input);
    input.close();

    if(world.rank() == 0)
      std::cout << " done.\nConstructing Fock tensors...";

    // Construct Fock tensor
    TArray2s f_a_oo = data.make_f(world, alpha, occ, occ);
    TArray2s f_a_vv = data.make_f(world, alpha, vir, vir);
    // Just make references to the data since the input is closed shell.
    TArray2s& f_b_oo = f_a_oo;
    TArray2s& f_b_vv = f_a_vv;

    // Fence to make sure Fock tensors are initialized on all nodes
    world.gop.fence();

    if(world.rank() == 0)
      std::cout << " done.\nConstructing v_ab tensors...";

    // Construct the integral tensors
    TArray4s v_ab_oooo = data.make_v_ab(world, occ, occ, occ, occ);
    TArray4s v_ab_vvoo = data.make_v_ab(world, vir, vir, occ, occ);
    TArray4s v_ab_oovv = data.make_v_ab(world, occ, occ, vir, vir);
    TArray4s v_ab_vovo = data.make_v_ab(world, vir, occ, vir, occ);
    TArray4s v_ab_ovov = data.make_v_ab(world, occ, vir, occ, vir);
    TArray4s v_ab_voov = data.make_v_ab(world, vir, occ, occ, vir);
    TArray4s v_ab_ovvo = data.make_v_ab(world, occ, vir, vir, occ);
    TArray4s v_ab_vvvv = data.make_v_ab(world, vir, vir, vir, vir);

    // Fence to make sure data on all nodes has been initialized
    world.gop.fence();

    if(world.rank() == 0)
      std::cout << " done.\nConstructing v_aa and v_bb tensors...";

    TArray4s v_aa_oooo;
    v_aa_oooo("i,j,k,l") = v_ab_oooo("i,j,k,l") - v_ab_oooo("i,j,l,k");
    TArray4s v_aa_vvoo;
    v_aa_vvoo("a,b,i,j") = v_ab_vvoo("a,b,i,j") - v_ab_vvoo("a,b,j,i");
    TArray4s v_aa_vovo;
    v_aa_vovo("a,i,b,j") = v_ab_vovo("a,i,b,j") - v_ab_voov("a,i,j,b");
    TArray4s v_aa_oovv;
    v_aa_oovv("i,j,a,b") = v_ab_oovv("i,j,a,b") - v_ab_oovv("i,j,b,a");
    TArray4s v_aa_vvvv;
    v_aa_vvvv("a,b,c,d") = v_ab_vvvv("a,b,c,d") - v_ab_vvvv("a,b,d,c");
    // Just make references to the data since the input is closed shell.
    TArray4s& v_bb_oooo = v_aa_oooo;
    TArray4s& v_bb_vvoo = v_aa_vvoo;
    TArray4s& v_bb_vovo = v_aa_vovo;
    TArray4s& v_bb_oovv = v_aa_oovv;
    TArray4s& v_bb_vvvv = v_aa_vvvv;

    // Fence again to make sure data all the integral tensors have been initialized
    world.gop.fence();

    if(world.rank() == 0)
      std::cout << " done.\n";

    TArray4s t_aa_vvoo(world, v_aa_vvoo.trange(), v_aa_vvoo.get_shape());
    for(TArray4s::range_type::const_iterator it = t_aa_vvoo.range().begin(); it != t_aa_vvoo.range().end(); ++it)
      if(t_aa_vvoo.is_local(*it) && (! t_aa_vvoo.is_zero(*it)))
        t_aa_vvoo.set(*it, 0.0);

    TArray4s t_ab_vvoo(world, v_ab_vvoo.trange(), v_ab_vvoo.get_shape());
    for(TArray4s::range_type::const_iterator it = t_ab_vvoo.range().begin(); it != t_ab_vvoo.range().end(); ++it)
      if(t_ab_vvoo.is_local(*it) && (! t_ab_vvoo.is_zero(*it)))
        t_ab_vvoo.set(*it, 0.0);

    TArray4s t_bb_vvoo(world, v_bb_vvoo.trange(), v_bb_vvoo.get_shape());
    for(TArray4s::range_type::const_iterator it = t_bb_vvoo.range().begin(); it != t_bb_vvoo.range().end(); ++it)
      if(t_bb_vvoo.is_local(*it) && (! t_bb_vvoo.is_zero(*it)))
        t_bb_vvoo.set(*it, 0.0);

    TArray4s D_vvoo(world, v_ab_vvoo.trange(), v_ab_vvoo.get_shape());
    for(TArray4s::range_type::const_iterator it = D_vvoo.range().begin(); it != D_vvoo.range().end(); ++it)
      if(D_vvoo.is_local(*it) && (! D_vvoo.is_zero(*it)))
        D_vvoo.set(*it, world.taskq.add(data, & InputData::make_D_vvoo_tile, D_vvoo.trange().make_tile_range(*it)));


    world.gop.fence();

    data.clear();

    if(world.rank() == 0)
      std::cout << "Calculating t amplitudes...\n";


    double energy = 0.0;

    for(unsigned int i = 0ul; i < 100; ++i) {

      if(world.rank() == 0)
        std::cout << "Iteration " << i << "\n";

      TArray4s r_aa_vvoo;
      r_aa_vvoo("p1a,p2a,h1a,h2a") =
          v_aa_vvoo("p1a,p2a,h1a,h2a")
          -f_a_vv("p1a,p3a")*t_aa_vvoo("p2a,p3a,h1a,h2a")
          +f_a_vv("p2a,p3a")*t_aa_vvoo("p1a,p3a,h1a,h2a")
          +f_a_oo("h3a,h1a")*t_aa_vvoo("p1a,p2a,h2a,h3a")
          -f_a_oo("h3a,h2a")*t_aa_vvoo("p1a,p2a,h1a,h3a")
          +0.5*t_aa_vvoo("p3a,p4a,h1a,h2a")*v_aa_vvvv("p1a,p2a,p3a,p4a")
          +v_ab_voov("p1a,h3b,h1a,p3b")*t_ab_vvoo("p2a,p3b,h2a,h3b")
          -v_aa_vovo("p1a,h3a,p3a,h1a")*t_aa_vvoo("p2a,p3a,h2a,h3a")
          -v_ab_voov("p1a,h3b,h2a,p3b")*t_ab_vvoo("p2a,p3b,h1a,h3b")
          +v_aa_vovo("p1a,h3a,p3a,h2a")*t_aa_vvoo("p2a,p3a,h1a,h3a")
          -v_ab_voov("p2a,h3b,h1a,p3b")*t_ab_vvoo("p1a,p3b,h2a,h3b")
          +v_aa_vovo("p2a,h3a,p3a,h1a")*t_aa_vvoo("p1a,p3a,h2a,h3a")
          +v_ab_voov("p2a,h3b,h2a,p3b")*t_ab_vvoo("p1a,p3b,h1a,h3b")
          -v_aa_vovo("p2a,h3a,p3a,h2a")*t_aa_vvoo("p1a,p3a,h1a,h3a")
          +0.5*v_aa_oooo("h3a,h4a,h1a,h2a")*t_aa_vvoo("p1a,p2a,h3a,h4a")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p2a,p4b,h3a,h4b")*t_aa_vvoo("p1a,p3a,h1a,h2a")
          -0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p2a,p4a,h3a,h4a")*t_aa_vvoo("p1a,p3a,h1a,h2a")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p1a,p4b,h3a,h4b")*t_aa_vvoo("p2a,p3a,h1a,h2a")
          +0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p1a,p4a,h3a,h4a")*t_aa_vvoo("p2a,p3a,h1a,h2a")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h2a,h4b")*t_aa_vvoo("p1a,p2a,h1a,h3a")
          -0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p3a,p4a,h2a,h4a")*t_aa_vvoo("p1a,p2a,h1a,h3a")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h1a,h4b")*t_aa_vvoo("p1a,p2a,h2a,h3a")
          -0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p3a,p4a,h1a,h3a")*t_aa_vvoo("p1a,p2a,h2a,h4a")
          +0.25*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p3a,p4a,h1a,h2a")*t_aa_vvoo("p1a,p2a,h3a,h4a")
          +v_bb_oovv("h3b,h4b,p3b,p4b")*t_ab_vvoo("p1a,p3b,h1a,h3b")*t_ab_vvoo("p2a,p4b,h2a,h4b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p1a,p4b,h1a,h4b")*t_aa_vvoo("p2a,p3a,h2a,h3a")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_aa_vvoo("p1a,p3a,h1a,h3a")*t_ab_vvoo("p2a,p4b,h2a,h4b")
          +v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p1a,p3a,h1a,h3a")*t_aa_vvoo("p2a,p4a,h2a,h4a")
          -v_bb_oovv("h3b,h4b,p3b,p4b")*t_ab_vvoo("p2a,p3b,h1a,h3b")*t_ab_vvoo("p1a,p4b,h2a,h4b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p2a,p4b,h1a,h4b")*t_aa_vvoo("p1a,p3a,h2a,h3a")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_aa_vvoo("p2a,p3a,h1a,h3a")*t_ab_vvoo("p1a,p4b,h2a,h4b")
          -v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p2a,p3a,h1a,h3a")*t_aa_vvoo("p1a,p4a,h2a,h4a");

      world.gop.fence();

      TArray4s r_ab_vvoo;
      r_ab_vvoo("p1a,p2b,h1a,h2b") =
          v_ab_vvoo("p1a,p2b,h1a,h2b")
          +f_a_vv("p1a,p3a")*t_ab_vvoo("p3a,p2b,h1a,h2b")
          +f_b_vv("p2b,p3b")*t_ab_vvoo("p1a,p3b,h1a,h2b")
          -f_a_oo("h3a,h1a")*t_ab_vvoo("p1a,p2b,h3a,h2b")
          -f_b_oo("h3b,h2b")*t_ab_vvoo("p1a,p2b,h1a,h3b")
          +t_ab_vvoo("p3a,p4b,h1a,h2b")*v_ab_vvvv("p1a,p2b,p3a,p4b")
          +v_ab_voov("p1a,h3b,h1a,p3b")*t_bb_vvoo("p2b,p3b,h2b,h3b")
          -v_aa_vovo("p1a,h3a,p3a,h1a")*t_ab_vvoo("p3a,p2b,h3a,h2b")
          -v_ab_vovo("p1a,h3b,p3a,h2b")*t_ab_vvoo("p3a,p2b,h1a,h3b")
          -v_ab_ovov("h3a,p2b,h1a,p3b")*t_ab_vvoo("p1a,p3b,h3a,h2b")
          -v_bb_vovo("p2b,h3b,p3b,h2b")*t_ab_vvoo("p1a,p3b,h1a,h3b")
          +v_ab_ovvo("h3a,p2b,p3a,h2b")*t_aa_vvoo("p1a,p3a,h1a,h3a")
          +v_ab_oooo("h3a,h4b,h1a,h2b")*t_ab_vvoo("p1a,p2b,h3a,h4b")
          -0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p2b,p4b,h3b,h4b")*t_ab_vvoo("p1a,p3b,h1a,h2b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p2b,h3a,h4b")*t_ab_vvoo("p1a,p4b,h1a,h2b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p1a,p4b,h3a,h4b")*t_ab_vvoo("p3a,p2b,h1a,h2b")
          -0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p1a,p4a,h3a,h4a")*t_ab_vvoo("p3a,p2b,h1a,h2b")
          -0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p3b,p4b,h2b,h4b")*t_ab_vvoo("p1a,p2b,h1a,h3b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h3a,h2b")*t_ab_vvoo("p1a,p2b,h1a,h4b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h1a,h4b")*t_ab_vvoo("p1a,p2b,h3a,h2b")
          +0.5*v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p3a,p4a,h1a,h3a")*t_ab_vvoo("p1a,p2b,h4a,h2b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h1a,h2b")*t_ab_vvoo("p1a,p2b,h3a,h4b")
          +v_bb_oovv("h3b,h4b,p3b,p4b")*t_ab_vvoo("p1a,p3b,h1a,h3b")*t_bb_vvoo("p2b,p4b,h2b,h4b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p1a,p4b,h1a,h4b")*t_ab_vvoo("p3a,p2b,h3a,h2b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_aa_vvoo("p1a,p3a,h1a,h3a")*t_bb_vvoo("p2b,p4b,h2b,h4b")
          +v_aa_oovv("h3a,h4a,p3a,p4a")*t_aa_vvoo("p1a,p3a,h1a,h3a")*t_ab_vvoo("p4a,p2b,h4a,h2b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p2b,h1a,h4b")*t_ab_vvoo("p1a,p4b,h3a,h2b");

      world.gop.fence();

      TArray4s r_bb_vvoo;
      r_bb_vvoo("p1b,p2b,h1b,h2b") =
          v_bb_vvoo("p1b,p2b,h1b,h2b")
          -f_b_vv("p1b,p3b")*t_bb_vvoo("p2b,p3b,h1b,h2b")
          +f_b_vv("p2b,p3b")*t_bb_vvoo("p1b,p3b,h1b,h2b")
          +f_b_oo("h3b,h1b")*t_bb_vvoo("p1b,p2b,h2b,h3b")
          -f_b_oo("h3b,h2b")*t_bb_vvoo("p1b,p2b,h1b,h3b")
          +0.5*t_bb_vvoo("p3b,p4b,h1b,h2b")*v_bb_vvvv("p1b,p2b,p3b,p4b")
          -v_bb_vovo("p1b,h3b,p3b,h1b")*t_bb_vvoo("p2b,p3b,h2b,h3b")
          +v_ab_ovvo("h3a,p1b,p3a,h1b")*t_ab_vvoo("p3a,p2b,h3a,h2b")
          +v_bb_vovo("p1b,h3b,p3b,h2b")*t_bb_vvoo("p2b,p3b,h1b,h3b")
          -v_ab_ovvo("h3a,p1b,p3a,h2b")*t_ab_vvoo("p3a,p2b,h3a,h1b")
          +v_bb_vovo("p2b,h3b,p3b,h1b")*t_bb_vvoo("p1b,p3b,h2b,h3b")
          -v_ab_ovvo("h3a,p2b,p3a,h1b")*t_ab_vvoo("p3a,p1b,h3a,h2b")
          -v_bb_vovo("p2b,h3b,p3b,h2b")*t_bb_vvoo("p1b,p3b,h1b,h3b")
          +v_ab_ovvo("h3a,p2b,p3a,h2b")*t_ab_vvoo("p3a,p1b,h3a,h1b")
          +0.5*v_bb_oooo("h3b,h4b,h1b,h2b")*t_bb_vvoo("p1b,p2b,h3b,h4b")
          -0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p2b,p4b,h3b,h4b")*t_bb_vvoo("p1b,p3b,h1b,h2b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p2b,h3a,h4b")*t_bb_vvoo("p1b,p4b,h1b,h2b")
          +0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p1b,p4b,h3b,h4b")*t_bb_vvoo("p2b,p3b,h1b,h2b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p1b,h3a,h4b")*t_bb_vvoo("p2b,p4b,h1b,h2b")
          -0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p3b,p4b,h2b,h4b")*t_bb_vvoo("p1b,p2b,h1b,h3b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h3a,h2b")*t_bb_vvoo("p1b,p2b,h1b,h4b")
          -0.5*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p3b,p4b,h1b,h3b")*t_bb_vvoo("p1b,p2b,h2b,h4b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p4b,h3a,h1b")*t_bb_vvoo("p1b,p2b,h2b,h4b")
          +0.25*v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p3b,p4b,h1b,h2b")*t_bb_vvoo("p1b,p2b,h3b,h4b")
          +v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p1b,p3b,h1b,h3b")*t_bb_vvoo("p2b,p4b,h2b,h4b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p2b,h3a,h2b")*t_bb_vvoo("p1b,p4b,h1b,h4b")
          +v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p1b,h3a,h1b")*t_bb_vvoo("p2b,p4b,h2b,h4b")
          +v_aa_oovv("h3a,h4a,p3a,p4a")*t_ab_vvoo("p3a,p1b,h3a,h1b")*t_ab_vvoo("p4a,p2b,h4a,h2b")
          -v_bb_oovv("h3b,h4b,p3b,p4b")*t_bb_vvoo("p2b,p3b,h1b,h3b")*t_bb_vvoo("p1b,p4b,h2b,h4b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p1b,h3a,h2b")*t_bb_vvoo("p2b,p4b,h1b,h4b")
          -v_ab_oovv("h3a,h4b,p3a,p4b")*t_ab_vvoo("p3a,p2b,h3a,h1b")*t_bb_vvoo("p1b,p4b,h2b,h4b")
          -v_aa_oovv("h3a,h4a,p3a,p4a")*t_ab_vvoo("p3a,p2b,h3a,h1b")*t_ab_vvoo("p4a,p1b,h4a,h2b");


      world.gop.fence();

      t_aa_vvoo("a,b,i,j") =
          D_vvoo("a,b,i,j") * r_aa_vvoo("a,b,i,j")
          + t_aa_vvoo("a,b,i,j");

      t_ab_vvoo("a,b,i,j") =
          D_vvoo("a,b,i,j") * r_ab_vvoo("a,b,i,j")
          + t_ab_vvoo("a,b,i,j");

      t_bb_vvoo("a,b,i,j") =
          D_vvoo("a,b,i,j") * r_bb_vvoo("a,b,i,j")
          + t_bb_vvoo("a,b,i,j");

      const double error = (r_aa_vvoo("a,b,i,j")
          + r_ab_vvoo("a,b,i,j") + r_bb_vvoo("a,b,i,j")).norm();

      energy =
           0.25 * ( t_aa_vvoo("a,b,i,j").dot(v_aa_vvoo("a,b,i,j"))
                   + t_bb_vvoo("a,b,i,j").dot(v_bb_vvoo("a,b,i,j"))
                  )
          + t_ab_vvoo("a,b,i,j").dot(v_ab_vvoo("a,b,i,j"));

      world.gop.fence();

      if(world.rank() == 0)
        std::cout << " error  = "  << std::setprecision(12) << error << "\n"
                  << " energy = " << std::setprecision(12) << energy << "\n";

      if(error < 1.0e-10)
        break;
    }

    if(world.rank() == 0) {
      std::cout << "CCD energy = " << std::setprecision(12) << energy << "\n";
      std::cout << "Done!\n";
    }

  } else  {
    std::cout << "Unable to open file: " << file_name << "\n";
    // stop the madenss runtime
    madness::finalize();
    return 1;
  }

  madness::finalize();
	return 0;
}
