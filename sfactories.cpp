/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
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


#include <string>

#include <sfactories.hpp>
#include <sys/stat.h>


#include <boost/algorithm/string.hpp>

#include <AtomicGroup.hpp>
#include <pdb.hpp>
#include <psf.hpp>
#include <amber.hpp>

#include <Trajectory.hpp>
#include <dcd.hpp>
#include <amber_traj.hpp>

#if defined(HAS_NETCDF)
#include <amber_netcdf.hpp>
#endif

#include <amber_rst.hpp>
#include <ccpdb.hpp>
#include <charmm.hpp>
#include <tinkerxyz.hpp>
#include <tinker_arc.hpp>
#include <gro.hpp>
#include <xtc.hpp>
#include <trr.hpp>


#include <trajwriter.hpp>
#include <dcdwriter.hpp>
#include <xtcwriter.hpp>

namespace loos {

  std::string availableSystemFileTypes() {
    std::string types = "crd (CHARMM), gro (GROMACS), pdb (CHARMM/NAMD), prmtop (Amber), psf (CHARMM/NAMD), xyz (Tinker)";
    return(types);
  }


  pAtomicGroup createSystemPtr(const std::string& filename, const std::string& filetype) {

    pAtomicGroup pag;

    if (filetype == "pdb") {
      pPDB p(new PDB(filename));
      pag = p;
    } else if (filetype == "psf") {
      pPSF p(new PSF(filename));
      pag = p;
    } else if (filetype == "prmtop") { 
      pAmber p(new Amber(filename));
      pag = p;
    } else if (filetype == "xyz") {
      pTinkerXYZ p(new TinkerXYZ(filename));
      pag = p;
    } else if (filetype == "gro") {
      pGromacs p(new Gromacs(filename));
      pag = p;
    } else if (filetype == "crd") {
      pCHARMM p(new CHARMM(filename));
      pag = p;

    } else
      throw(std::runtime_error("Error- unknown system file type '" + filetype + "' for file '" + filename + "'. Try --help to see available types."));

    return(pag);
  }



  pAtomicGroup createSystemPtr(const std::string& filename) {

    boost::tuple<std::string, std::string> names = splitFilename(filename);
    if (boost::get<1>(names).empty())
      throw(std::runtime_error("Error- system filename must end in an extension or the filetype must be explicitly specified"));

    std::string filetype = boost::get<1>(names);
    boost::to_lower(filetype);
    return(createSystemPtr(filename, filetype));
  }


  AtomicGroup createSystem(const std::string& filename) {
    return(*(createSystemPtr(filename)));
  }

  AtomicGroup createSystem(const std::string& filename, const std::string& filetype) {
    return(*(createSystemPtr(filename, filetype)));
  }


  namespace internal {
    struct TrajectoryNameBindingType {
      std::string suffix;
      std::string type;
      Trajectory* (*creator)(const std::string& fname, const AtomicGroup& model);
    };

    TrajectoryNameBindingType trajectory_name_bindings[] = {
      { "dcd", "CHARMM/NAMD DCD", &DCD::create},
#if defined(HAS_NETCDF)
      { "nc", "Amber Trajectory (NetCDF format)", &AmberNetcdf::create},
      { "mdcrd", "Amber Trajectory (NetCDF or Amber format)", &AmberNetcdf::create},
      { "crd", "Amber Trajectory (NetCDF or Amber format)", &AmberNetcdf::create},
#else
      { "mdcrd", "Amber Trajectory", &AmberTraj::create},
      { "crd", "Amber Trajectory", &AmberTraj::create},
#endif
      { "rst", "Amber Restart", &AmberRst::create},
      { "rst7", "Amber Restart", &AmberRst::create},
      { "impcrd", "Amber Restart", &AmberRst::create},
      { "pdb", "Concatenated PDB", &CCPDB::create},
      { "arc", "Tinker ARC", &TinkerArc::create},
      { "xtc", "Gromacs XTC", &XTC::create},
      { "trr", "Gromacs TRR", &TRR::create},
      { "", "", 0}
    };
      
    

  }


  std::string availableTrajectoryFileTypes() {
    std::string types;
    for (internal::TrajectoryNameBindingType* p = internal::trajectory_name_bindings; p->creator != 0; ++p) {
      types += p->suffix + " = " + p->type + "\n";
    }

    return(types);

  }


  pTraj createTrajectory(const std::string& filename, const std::string& filetype, const AtomicGroup& g) {
    
    // First, check to make sure AtomicGroup has index information...
    if (!g.allHaveProperty(Atom::indexbit))
      throw(LOOSError("Model passed to createTrajectory() does not have atom index information."));

    if (filetype == "dcd") {
      pDCD pd(new DCD(filename));
      pTraj pt(pd);
      return(pt);
    } else if (filetype == "nc") {
#if defined(HAS_NETCDF)
	pAmberNetcdf pat(new AmberNetcdf(filename, g.size()));
	pTraj pt(pat);
	return(pt);
#else
	throw(std::runtime_error("Error- trajectory type is an Amber Netcdf file but LOOS was built without netcdf support."));
#endif
    } else if (filetype == "mdcrd"
	       || filetype == "crd") {

#if defined(HAS_NETCDF)      
      if (isFileNetCDF(filename)) {
        pAmberNetcdf pat(new AmberNetcdf(filename, g.size()));
        pTraj pt(pat);
        return(pt);
      }
#endif

      pAmberTraj pat(new AmberTraj(filename, g.size()));
      pTraj pt(pat);
      return(pt);

    } else if (filetype == "rst"
               || filetype == "rst7"
               || filetype == "inpcrd") {
      pAmberRst par(new AmberRst(filename, g.size()));
      pTraj pt(par);
      return(pt);
    } else if (filetype == "pdb") {
      pCCPDB ppdb(new CCPDB(filename));
      pTraj pt(ppdb);
      return(pt);
    } else if (filetype == "arc") {
      pTinkerArc pta(new TinkerArc(filename));
      pTraj pt(pta);
      return(pt);
    } else if (filetype == "xtc") {
      pXTC pxtc(new XTC(filename));
      pTraj pt(pxtc);
      return(pt);
    } else if (filetype == "trr") {
      pTRR ptrr(new TRR(filename));
      pTraj pt(ptrr);
      return(pt);

    } else
      throw(std::runtime_error("Error- unknown trajectory file type '" + filetype + "' for file '" + filename + "'.  Try --help to see available types."));
  }


  pTraj createTrajectory(const std::string& filename, const AtomicGroup& g) {
    boost::tuple<std::string, std::string> names = splitFilename(filename);

    if (boost::get<1>(names).empty())
      throw(std::runtime_error("Error- trajectory filename must end in an extension or the filetype must be explicitly specified"));

    std::string filetype = boost::get<1>(names);
    boost::to_lower(filetype);
    return(createTrajectory(filename, filetype, g));
  }


  namespace internal {
    struct OutputTrajectoryNameBindingType {
      std::string suffix;
      std::string type;
      TrajectoryWriter* (*creator)(const std::string& fname, const bool append);
    };

    OutputTrajectoryNameBindingType output_trajectory_name_bindings[] = {
      { "dcd", "NAMD DCD", &DCDWriter::create},
      { "xtc", "Gromacs XTC (compressed trajectory)", &XTCWriter::create},
      { "", "", 0}
    };
      
  }


  std::string availableOutputTrajectoryFileTypes() {
    std::string types;
    for (internal::OutputTrajectoryNameBindingType* p = internal::output_trajectory_name_bindings; p->creator != 0; ++p) {
      types += p->suffix + "\t" + p->type + "\n";
    }

    return(types);
  }


  TrajectoryWriter* createOutputTrajectory(const std::string& filename, const bool append) {
    boost::tuple<std::string, std::string> names = splitFilename(filename);
    std::string suffix = boost::get<1>(names);
    if (suffix.empty())
      throw(std::runtime_error("Error- output trajectory filename must end in an extension or the filetype must be explicitly specified"));

    for (internal::OutputTrajectoryNameBindingType* p = internal::output_trajectory_name_bindings; p->creator != 0; ++p) {
      if (p->suffix == suffix) {
        return((*(p->creator))(filename, append));
      }
    }
    throw(std::runtime_error("Error- unknown output trajectory file type '" + suffix + "' for file '" + filename + "'.  Try --help to see available types."));
    return(0);
  }

}



