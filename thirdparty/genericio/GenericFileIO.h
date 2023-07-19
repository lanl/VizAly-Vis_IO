#ifndef __GENERIC_FILE_IO
#define __GENERIC_FILE_IO

#include <string>

namespace gio {
class GenericFileIO {
public:
  virtual ~GenericFileIO() {}

public:
  virtual void open(const std::string &FN, bool ForReading = false, bool MustExist = false) = 0;
  virtual void setSize(size_t sz) = 0;
  virtual void read(void *buf, size_t count, off_t offset,
                    const std::string &D) = 0;
  virtual void write(const void *buf, size_t count, off_t offset,
                     const std::string &D) = 0;

protected:
  std::string FileName;
};
}

#endif //__GENERIC_FILE_IO
