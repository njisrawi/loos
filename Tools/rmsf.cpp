/*
  rmsf.cpp

  Compute the root mean square fluctuations (generally for CA's)
*/

/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod Romo
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


#include <loos.hpp>
#include <cmath>

using namespace std;
using namespace loos;


namespace opts = loos::OptionsFramework;
namespace po = loos::OptionsFramework::po;



string fullHelpMessage(void) {
  string msg =
    "\n"
    "SYNOPSIS\n"
    "\tCalculate root mean squared fluctuations for a selection\n"
    "\n"
    "DESCRIPTION\n"
    "\n"
    "\tThis tool calculates the root mean squared fluctuations for each atom in a selection.\n"
    "\n"
    "EXAMPLES\n"
    "\n"
    "\trmsf model.pdb simulation.dcd >rmsf.asc\n"
    "This example calculates the RMSF for the default selection (all alpha-carbons)\n"
    "\n"
    "\trmsf --range 0:99 model.pdb simulation.dcd >rmsf.asc\n"
    "This example calculates the RMSF for all alpha-carbons, using the first 100 frames\n"
    "from the trajectory\n"
    "\trmsf --range 0:2:999 --selection 'name =~ \"^(C|O|N|CA)$\"' \\\n"
    "\t  model.pdb simulation.dcd >rmsf.asc\n"
    "This example calculates the RMSF over backbone atoms using the first 1,000 frames and\n"
    "skipping every other frame.\n"
    "\n"
    "POTENTIAL COMPLICATIONS\n"
    "\n"
    "This tool assumes that you have already aligned the trajectory.  If you\n"
    "haven't done so, you will need to use the aligner tool to do so.\n"
    "\n";

  return(msg);
}


int main(int argc, char *argv[]) {
  
  string hdr = invocationHeader(argc, argv);

  opts::BasicOptions* bopts = new opts::BasicOptions(fullHelpMessage());
  opts::BasicSelection* sopts = new opts::BasicSelection("name == 'CA'");
  opts::TrajectoryWithFrameIndices* tropts = new opts::TrajectoryWithFrameIndices;

  opts::AggregateOptions options;
  options.add(bopts).add(sopts).add(tropts);
  if (!options.parse(argc, argv))
    exit(-1);
  
  cout << "# " << hdr << endl;

  AtomicGroup model = tropts->model;
  pTraj traj = tropts->trajectory;

  AtomicGroup subset = selectAtoms(model, sopts->selection);
  vector<uint> indices = tropts->frameList();


  vector<AtomicGroup> frames;
  for (vector<uint>::iterator i = indices.begin(); i != indices.end(); ++i) {
    traj->readFrame(*i);
    traj->updateGroupCoords(subset);
    AtomicGroup frame = subset.copy();
    frames.push_back(frame);
  }

  AtomicGroup avg = averageStructure(frames);
  uint n = avg.size();
  uint m = frames.size();

  vector<double> rmsf(n, 0.0);
  for (uint i = 0; i < m; i++)
    for (uint j = 0; j < n; j++) {
      double d = frames[i][j]->coords().distance2(avg[j]->coords());
      rmsf[j] += d;
    }

  for (uint i = 0; i < n; i++)
    rmsf[i] = sqrt(rmsf[i] / m);

  cout << "# atomid\tresid\tRMSF\n";
  for (uint i = 0; i < n; i++)
    cout << boost::format("%10d %6d   %f\n") % avg[i]->id() % avg[i]->resid() % rmsf[i];

}
