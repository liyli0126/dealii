// ---------------------------------------------------------------------
//
// Copyright (C) 2007 - 2016 by the deal.II authors
//
// This file is part of the deal.II library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the deal.II distribution.
//
// ---------------------------------------------------------------------



// test assemble_flags.h

#include "../tests.h"
#include <deal.II/grid/tria.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/meshworker/assemble_flags.h>
#include <deal.II/meshworker/mesh_loop.h>
#include <deal.II/base/work_stream.h>

#include <fstream>

struct ScratchData {};
struct CopyData {};

using namespace MeshWorker;

template<int dim, int spacedim>
void test() {
  Triangulation<dim,spacedim> tria;
  GridGenerator::hyper_cube(tria);
  tria.refine_global(1);
  tria.begin_active()->set_refine_flag();
  tria.execute_coarsening_and_refinement();

  ScratchData scratch;
  CopyData copy;

  auto cell=tria.begin_active();
  auto endc=tria.end();

  typedef decltype(cell) Iterator;



  auto cell_worker = [] (const Iterator &cell, ScratchData &s, CopyData &c) {
    deallog << "Cell worker on : " << cell << std::endl;
  };

  auto boundary_worker = [] (const Iterator &cell, const unsigned int &f, ScratchData &, CopyData &) {
    deallog << "Boundary worker on : " << cell << ", Face : "<< f << std::endl;
  };

  auto face_worker = []
      (const Iterator &cell, const unsigned int & f, const unsigned int &sf,
       const Iterator &ncell, const unsigned int &nf, const unsigned int & nsf,
        ScratchData &s, CopyData &c) {
        deallog << "Face worker on : " << cell << ", Neighbor cell : "  << ncell
                << ", Face : "<< f << ", Neighbor Face : " << nf
                << ", Subface: " << sf
                << ", Neighbor Subface: " << nsf << std::endl;
  };

  auto copyer = [](const CopyData &) {
    deallog << "Copyer" << std::endl;
  };

  deallog << "CELLS ONLY" << std::endl << std::endl;

  mesh_loop(cell, endc, cell_worker, copyer, scratch, copy,
            assemble_own_cells,
            boundary_worker, face_worker);


  deallog << "CELLS ONLY WORKSTREAM" << std::endl << std::endl;

  WorkStream::run(cell, endc, cell_worker, copyer, scratch, copy);


}


int main()
{
  initlog();

  test<2,2>();
}
