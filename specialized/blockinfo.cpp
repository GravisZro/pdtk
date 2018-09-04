#include "blockinfo.h"

// https://stackoverflow.com/questions/19747663/where-are-ioctl-parameters-such-as-0x1268-blksszget-actually-specified

#if defined(__linux__)

#elif defined(__minix) // MINIX
#error Detection of block partitions is not implemented in PDTK for MINIX!  Please submit a patch!

#elif defined(__QNX__) // QNX
#error Detection of block partitions is not implemented in PDTK for QNX!  Please submit a patch!

#elif defined(__hpux) // HP-UX
#error Detection of block partitions is not implemented in PDTK for HP-UX!  Please submit a patch!

#elif defined(_AIX) // IBM AIX
#error Detection of block partitions is not implemented in PDTK for IBM AIX!  Please submit a patch!

#elif defined(__APPLE__) // Darwin
#error Detection of block partitions is not implemented in PDTK for Darwin!  Please submit a patch!

#elif defined(__sun) && defined(__SVR4) // Solaris / OpenSolaris / OpenIndiana / illumos
#error Detection of block partitions is not implemented in PDTK for Solaris!  Please submit a patch!

#elif defined(__FreeBSD__) || defined(__DragonFly__) // FreeBSD and DragonFly BSD
#error Detection of block partitions is not implemented in PDTK for FreeBSD!  Please submit a patch!

#elif defined(__OpenBSD__) // OpenBSD
#error Detection of block partitions is not implemented in PDTK for OpenBSD!  Please submit a patch!

#elif defined(__NetBSD__) // NetBSD
#error Detection of block partitions is not implemented in PDTK for NetBSD!  Please submit a patch!

#elif defined(BSD)
#error Unrecognized BSD derivative!

#elif defined(__unix__)
#error Unrecognized UNIX variant!

#else
#error This platform is not supported.
#endif