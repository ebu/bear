/*
 * Copyright (c) 2014-2019 Institute of Sound and Vibration Research,
 * University of Southampton and VISR contributors --- All rights reserved.
 * Copyright (c) 2014-2019 Andreas Franck a.franck@soton.ac.uk --- All rights reserved.
 */

#ifndef VISR_TIME_HPP_INCLUDED
#define VISR_TIME_HPP_INCLUDED

#include "export_symbols.hpp"

#include <cstddef>
#include <memory>

namespace visr
{
// Forward declarations
namespace impl
{
class TimeImplementation;
class ComponentImplementation;
} // namespace impl

/**
 * Facade class to access time (clock) information from the VISR component API.
 * The Time class is to be accessed through references returned by the method
 * \p Component::time(), Time objects are not supposed to be created (or
 * copied) by users of the API.
 */
class VISR_CORE_LIBRARY_SYMBOL Time
{
public:
  /**
   * Deleted standard constructors and assingment operators.
   * Time objects are not supposed to be constructed within the VISR component API,
   * but to be used only through references returrned by the Component::time() method.
   */
  //@{
  Time() = delete;
  Time( Time const & ) = delete;
  Time( Time && ) = delete;
  Time & operator=( Time const & ) = delete;
  Time & operator=( Time && ) = delete;
  //@}

  /**
   * Floating-point type for representing continuous time stamps or durations (in seconds)
   */
  using ContinuousTimeType = double;

  /**
   * Integer-valued type to represent times counted in discrete quantities, e.g.,
   * number of samples or blocks.
   */
  using IntegerTimeType = std::size_t;

  /**
   * Return the number of audio samples (per channel) processed since the start of the rend ering.
   * This number refers to the state at the start of the current iteration (i.e., \p process() call),
   * and is therefore a multiple of period size of the signal flow.
   * The sample counter represents a monotonic clock because it cannot be reset.
   */
  IntegerTimeType sampleCount() const;

  /**
   * Return the number of processed blocks since the start of the rendering.
   * Return the number of audio blocks processed since the start of the rendering.
   * The block counter represents a monotonic clock because it cannot be reset.
   */
  IntegerTimeType blockCount() const;
    
  /**
   * Return the rendering time (as a floating-point value in seconds).
   * The rendering time is a monotonic clock counting from the start of rendering.
   * It counts the number of frames (sample for each channel) processed.
   * The returned time point corresponds to the start of the current iteration
   * (i.e., \p process() call )
   */
  ContinuousTimeType renderingTime() const;

private:
  // Liefetime management and access to the implementation object is managed
  // by ComponentImplementation.
  friend class impl::ComponentImplementation;
    
  /**
   * Private constructor that accepts an implementation object.
   * This constructor is supposed to be called from ComponentImplementation (a friend class),
   * which is responsible for creating and passing the \p impl object.
   * This ensures that all components in a hierarchical signal flow share the same implementation object.
   */
  Time( std::shared_ptr<impl::TimeImplementation> impl );

  /**
   * Explicitly instantiated private destructor.
   * The dtor is proivate because lifetime management is performed exclusively through the
   * impl::ComponentImplementation class (which is a friend class)
   */
  ~Time();
    
  /**
   * Return the implementation object, non-const version.
   * This method is private and to be used by friend classes, e.g., ComponentImplementation.
   */
  impl::TimeImplementation & implementation();

  /**
   * Return the implementation object, non-const version.
   * This method is private and to be used by friend classes, e.g., ComponentImplementation.
   */
  impl::TimeImplementation const & implementation() const;
    
  /**
   * Private implementation object following the pImpl idiom.
   * Moreover, through the way it is instantiated in ComponentImplementation, an implementation object is shared by all
   * components of a (potentially hierarchical) signal flow graph, which means that they access the same time information without
   * without additional effort by the runtime system.
   */
  std::shared_ptr<impl::TimeImplementation> mImpl;
};

} // namespace visr

#endif // #ifndef VISR_TIME_HPP_INCLUDED
