/*
  dcdinfo.cpp

  Dumps information about a DCD


  Usage:
     dcdinfo trajectory.dcd
*/



/*

  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <loos.hpp>
#include <boost/format.hpp>



using namespace std;
using namespace loos;


void analyzeBoxes(DCD& dcd) {
  dcd.rewind();
  GCoord max(0,0,0), min(1e38, 1e38, 1e38), avg(0,0,0);
  uint n = 0;

  while (dcd.readFrame()) {
    GCoord b = dcd.periodicBox();

    avg += b;
    ++n;

    if (b.length2() > max.length2())
      max = b;
    if (b.length2() < min.length2())
      min = b;
  }

  avg /= n;

  cout << "*   Average box size is " << avg << ", min is " << min << ", and max is " << max << endl;

}



int main(int argc, char *argv[]) {
  
  if (argc < 2 || argc > 3) {
    cerr << "Usage - dcdinfo [-s] trajectory.dcd\n";
    cerr << "    -s  scan the DCD for box information\n";
    exit(-1);
  }

  int opt = 1;
  bool scan = false;
  if (strcmp(argv[opt], "-s") == 0) {
    scan = true;
    ++opt;
  }

  DCD dcd(argv[opt]);

  cout << boost::format("* DCD has %u atoms in %u frames with a timestep of %f.\n") % dcd.natoms() % dcd.nframes() % dcd.timestep();
  if (dcd.hasCrystalParams()) {
    if (!scan)
      cout << "* DCD HAS box/crystal information.\n";
    else {
      cout << "Scanning trajectory for box information...\n";
      analyzeBoxes(dcd);
    }
  } else
    cout << "* DCD has no box/crystal information.\n";

  cout << "icntrl dump:\n";
  for (int i=0; i<20; ++i)
    cout << boost::format("\ticntrl[%d]\t= %d\n") % i % dcd.icntrl(i);

}
