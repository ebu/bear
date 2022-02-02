/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "initialisation_guard.hpp"

#include <cassert>

// Use the pybind11 header to include Python.h (instead of doing this directly)
// This is to overcome the pecularities of Python.h on Windows, 
// which enforces linking to a debug version of the python library.
// The pybind11 header already contains the magic to circumvent this.
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace visr
{

namespace pythonsupport
{

class InitialisationGuard::Internal
{
public:
  Internal()
  {
    pybind11::initialize_interpreter();

#if PY_VERSION_HEX < ((3 << 24) | (9 << 16))
    // does nothing in 3.9 and will be removed in 3.11
    PyEval_InitThreads();
#endif
    // After this call, the GIL is held by the calling thread.
    // We must release it because other threads might be created that might need the GIL 
    // to create their internal thread state.
    mState = PyEval_SaveThread();
#if PY_VERSION_HEX >= ((3 << 24) | (4 << 16))
    assert( PyGILState_Check()  == 0 );
#endif
    mInitialised = true;
  }

  ~Internal()
  {
    mInitialised = false;
    PyEval_RestoreThread( mState );
    pybind11::finalize_interpreter();
  }

  bool initialised() const
  {
    return mInitialised;
  }

private:
  bool mInitialised = false;

  /**
   * Thread state immediately after construction. This state is returned by the call to release the thread state.
   * Check whether we can safely restore that state during finalisation (we need to avoid deadlocking at this point,
   * but we also need to be sure that there are no stray calls of Python code).
   */
  PyThreadState* mState;
};

/*static*/ bool InitialisationGuard::initialise()
{
  static Internal sInternal;
  return sInternal.initialised();
}

} // namespace pythonsupport
} // namespace visr
