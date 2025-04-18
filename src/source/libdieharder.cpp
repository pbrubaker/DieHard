/* -*- C++ -*- */

/**
 * @file   libdiehard.cpp
 * @brief  Replaces malloc and friends with DieHard versions.
 * @author Emery Berger <http://www.emeryberger.org>
 * @note   Copyright (C) 2005-2017 by Emery Berger, University of Massachusetts Amherst.
 */

// The undef below ensures that any pthread_* calls get strong
// linkage.  Otherwise, our versions here won't replace them.  It is
// IMPERATIVE that this line appear before any files get included.

#undef __GXX_WEAK__ 

#include <stdlib.h>
#include <new>

// The heap multiplier.

enum { Numerator = 8, Denominator = 7 };

// enum { Numerator = 2, Denominator = 1 };
// enum { Numerator = 3, Denominator = 1 };
// enum { Numerator = 4, Denominator = 1 };
// enum { Numerator = 8, Denominator = 1 };
// enum { Numerator = 16, Denominator = 1 };

#include "heaplayers.h"

#include "combineheap.h"
#include "diehard.h"
#include "largeheap.h"
#include "diehardheap.h"

/*************************  define the DieHard heap ************************/

class TheLargeHeap : public OneHeap<LargeHeap<MmapWrapper> > {
  typedef OneHeap<LargeHeap<MmapWrapper> > Super;
public:
  inline void * malloc(size_t sz) {
    auto ptr = Super::malloc(sz);
    return ptr;
  }
  inline auto free(void * ptr) {
    return Super::free(ptr);
  }
};


typedef
 ANSIWrapper<
  LockedHeap<PosixLockType,
	     //	     CombineHeap<DieHardHeap<Numerator, Denominator, 1048576, // 65536,
	     CombineHeap<DieHardHeap<Numerator, Denominator, 1048576, // 65536,
				     (DIEHARD_DIEFAST == 1),
				     (DIEHARD_DIEHARDER == 1)>,
			 TheLargeHeap> > >
TheDieHardHeap;

class TheCustomHeapType : public TheDieHardHeap {};

inline static TheCustomHeapType * getCustomHeap (void) {
  static char buf[sizeof(TheCustomHeapType)];
  static TheCustomHeapType * _theCustomHeap = 
    new (buf) TheCustomHeapType;
  return _theCustomHeap;
}

#if defined(_WIN32)
#pragma warning(disable:4273)
#endif

#include "printf.h"

#if !defined(_WIN32)
#include <unistd.h>

extern "C" {
  // For use by the replacement printf routines (see
  // https://github.com/emeryberger/printf)
  void _putchar(char ch) { ::write(1, (void *)&ch, 1); }
}
#endif

// Heap-Layers
#include "wrappers/generic-memalign.cpp"

extern "C" {

  void * xxmalloc (size_t sz) {
    auto ptr = getCustomHeap()->malloc (sz);
    // printf_("xxmalloc %lu = %p\n", sz, ptr);
    return ptr;
  }

  void xxfree (void * ptr) {
    getCustomHeap()->free (ptr);
  }

  void * xxmemalign(size_t alignment, size_t sz) {
    return generic_xxmemalign(alignment, sz);
  }
  
  size_t xxmalloc_usable_size (void * ptr) {
    return getCustomHeap()->getSize (ptr);
  }

  void xxmalloc_lock() {
    getCustomHeap()->lock();
  }

  void xxmalloc_unlock() {
    getCustomHeap()->unlock();
  }

}
