#if !defined(LOOS_TRAJWRITER_HPP)
#define LOOS_TRAJWRITER_HPP

#include <iostream>
#include <string>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <loos_defs.hpp>
#include <AtomicGroup.hpp>


namespace loos {

  class TrajectoryWriter {
  public:

    struct WriteError : public std::exception {
      WriteError() : text("Error while writing trajectory") {}
      WriteError(const char* message) : text(message) {}
      
      virtual const char* what() const throw() { return(text); }
      
      const char* text;
    };
    

    TrajectoryWriter(const std::string& fname, const bool append = false)
      : appending_(false) {
      struct stat statbuf;

      if (append && !stat(fname.c_str(), &statbuf)) {
	openStream(fname, true);
	appending_ = true;
      } else
	openStream(fname);
    }


    virtual void writeFrame(const AtomicGroup& model) =0;

    virtual ~TrajectoryWriter() {
      if (delete_)
	delete stream_;
    }


  protected:
    std::iostream* stream_;
    bool appending_;
    bool delete_;

  private:

    void openStream(const std::string& fname, const bool append = false) {
      std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary;
      if (append)
        mode |= std::ios_base::in;
      else
        mode |= std::ios_base::trunc;

      stream_ = new std::fstream(fname.c_str(), mode);
      if (append)
	stream_->seekp(0, std::ios_base::end);
      if (!stream_->good())
        throw(std::runtime_error("Error while opening output trajectory file"));


      delete_ = true;
    }
    

  };



}




#endif