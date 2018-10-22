#include "mountpoint_helpers.h"

// PDTK
#include "posix_helpers.h"
#include "hashing.h"
#include <specialized/fstable.h>

#ifndef MOUNT_TABLE_FILE
#define MOUNT_TABLE_FILE  "/etc/mtab"
#endif

char* procfs_path = nullptr;
char* sysfs_path  = nullptr;
char* devfs_path  = nullptr;
char* scfs_path   = nullptr;

inline void assign_data(char*& target, const char* str) noexcept
{
  size_t length = ::strnlen(str, PATH_MAX) + 1;
  target = static_cast<char*>(::malloc(length));
  std::strncpy(target, str, length);
}

inline int init_paths(void) noexcept
{
  std::set<struct fsentry_t> table;
  if(parse_table(table, MOUNT_TABLE_FILE) != posix::success_response)
    return posix::error_response;

  for(const fsentry_t& entry : table)
  {
    switch(hash(entry.filesystems))
    {
      case "proc"_hash:
      case "procfs"_hash:
        assign_data(procfs_path, entry.path);
        break;
      case "devtmpfs"_hash:
      case "devfs"_hash:
        assign_data(devfs_path, entry.path);
        break;
      case "sysfs"_hash:
        assign_data(sysfs_path, entry.path);
        break;
      case "scfs"_hash:
        assign_data(scfs_path, entry.path);
        break;
    }
  }
  return posix::success_response;
}

static int path_init_ok = init_paths();