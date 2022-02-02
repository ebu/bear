/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBEFL_ALIGNED_ARRAY_HPP_INCLUDED
#define VISR_LIBEFL_ALIGNED_ARRAY_HPP_INCLUDED

#include "alignment.hpp"

#include <cstddef> // for std::size_t
#include <memory>
#include <new>
#include <stdexcept>
#include <utility> // for std::swap

// Workaround for the function std::align() missing in the GNU libstdc++ library
// (GCC <= 4.9). See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57350
// TODO: Remove as soon as feature has been implemented on all supported platforms
#ifdef __GLIBCXX__
#include <cstdint> // needed for the type std::uintptr_t used in the workaround implementation
#endif

namespace visr
{
namespace efl
{

/**
 * A template class to provide memory that is aligned in memory.
 * Alignment is an important property for speed. The aligement is given as a number of elements.
 * This means that the pointer to the aligned memory is divisable by alignmentElements*sizeof(T) .
 * @tparam ElementType The type of the array elements
 */
template< typename ElementType >
class AlignedArray
{
public:
  /**
   * Constructor, constructs an empty array and sets the alignment.
   * Note that this array cannot be dereferenced, and data() returns a null pointer.
   * @param alignmentElements The requested alignment of the array
   * which will be used if the array is resized to a non-zero length.
   * The alignment must be a one or an integral power of two. A value
   * of zero is supported, but transformed to 2 internally.
   * @throw std::invalid_argument If the alignment value is not an
   * integer power of 2 (or 0 or 1)
   */
  explicit AlignedArray( std::size_t alignmentElements );

  /**
   * Constructor, create an array with a specified number of elements and the given alignment.
   * @param length The requested length of the array, given as number of elements.
   * @param alignmentElements The requested alignment of the
   * array. The alignment must be a one or an integral power of two. A
   * value of zero is supported, but transformed to 2 internally.
   * @throw std::invalid_argument If the alignment value is not an
   * integer power of 2 (or 0 or 1)
   */
  explicit AlignedArray( std::size_t length, std::size_t alignmentElements );

  /**
   * Move constructor, takes over the contents of an rvalue \p rhs and leaves \p rhs in an empty, but valid state.
   */
  AlignedArray( AlignedArray< ElementType > && rhs );

  /**
   * Move assignment operator from a rvalue \p rhs.
   * @note The alignment of \p rhs must match the alignment of this object.
   * @throw std::logic_error if the alignment of \p rhs does not match this object.
   */
  AlignedArray<ElementType> & operator=( AlignedArray< ElementType > && rhs );

  ~AlignedArray();

  /**
   * Change the number of elements in the array.
   * The old content of the array is not retained, but the new content is initialized with arbitrary values.
   * @param newLength The allocated length of the array (in bytes)
   */
  void resize( std::size_t newLength );

  /**
   * Exchange the contents of this object with that of rhs.
   * Does not throw, thus enabling exception safety for classes using AlignedArray.
   */
  void swap( AlignedArray<ElementType>& rhs );

  /**
   * Return the size of the array (in elements).
   * This value may differ from the allocated length in order to ensure the
   * prescribed alignment.
   */
  std::size_t size() const { return mLength; }

  /**
   * Return the alignment of the the data structure (measured in number of elements)
   * as determined in the constructor.
   */
  std::size_t alignmentElements() const { return mAlignment; }

  /**
   * Return the alignment of the the data structure (measured in bytes)
   * as determined in the constructor.
   */
  std::size_t alignmentBytes( ) const { return mAlignment * sizeof(ElementType); }

  /**
   * Return a writable pointer to the data elements.
   */
  ElementType* data() { return mAlignedStorage; }

  /**
   * Return a constant pointer to the data elements.
   */
  ElementType const * data( ) const { return mAlignedStorage; }

  /**
   * Indexed access, return a modifiable reference to the element addressed by index.
   * @param index Zero-offset index into the array
   */
  ElementType& operator[]( std::size_t index ) { return mAlignedStorage[index]; }

  /**
   * Indexed access, return a constant reference to the element addressed by index.
   * @param index Zero-offset index into the array
   */
  ElementType const & operator[]( std::size_t index ) const { return mAlignedStorage[index]; }

  /**
   * Indexed access, return a modifiable reference to the element addressed by index.
   * The function variant provides range checking.
   * @param index Zero-offset index into the array
   * @throw std::out_of_range if index exceeds the allocated length of the array.
   */
  ElementType& at( std::size_t index )
  {
    if( index >= mLength )
    {
      throw std::out_of_range( "Array index exceeded" );
    }
    return operator[]( index );
  }

  /**
   * Indexed access, return a constant reference to the element addressed by index.
   * The function variant provides range checking.
   * @param index Zero-offset index into the array
   * @throw std::out_of_range if index exceeds the allocated length of the array.
   */
  ElementType const & at( std::size_t index ) const
  {
    if( index >= mLength ) {
      throw std::out_of_range( "Array index exceeded" );
    }
    return (*this)[index];
  }

private:
  /**
   * Default constructor (disabled)
   */
  AlignedArray() = delete;

  /**
   * Disabled copy constructor.
   */
  AlignedArray( AlignedArray< ElementType > const & ) = delete;

  /**
   * Internal implementation method to allocate the data memory with the alignment passed in the constructor.
   */
  void allocate( std::size_t length );

  /**
   * Internal implmenetation method to free the data memory.
   * Used by the resize() and the destructor.
   */
  void deallocate( );

  /**
   * The requested alignment of the array, given as the number of elements.
   */
  const std::size_t mAlignment;

  /**
   * The requested size of the array. Due to the mechanism used to achieve alignment,
   * the actual array might be larger than this length.
   */
  std::size_t mLength;

  /**
   * The pointer to the allocated memory.
   * Depending on the implementation, this pointer might be unaligned.
   */
  ElementType* mRawStorage;

  /**
   * The pointer to the aligned memory. This pointer points to a location
   * within the memory chunk designated by mRawStorage, but might differ from that one.
   * This pointer has no ownership over the memory, i.e., it must be used for freeing ressources.
   */
  ElementType* mAlignedStorage;
};

template< typename ElementType>
AlignedArray<ElementType>::AlignedArray( std::size_t alignmentElements )
  : mAlignment( alignmentElements == 0 ? 1 : alignmentElements )
 , mLength( 0 )
 , mRawStorage( nullptr )
 , mAlignedStorage( nullptr )
{
  if( !alignmentIsPowerOfTwo( mAlignment ) )
  {
    throw std::invalid_argument( "AlignedArray: alignment values must be integer powers of 2" );
  }
}

template< typename ElementType>
AlignedArray<ElementType>::AlignedArray( std::size_t length, std::size_t alignmentElements )
 : AlignedArray( alignmentElements )
{
  if( length > 0 )
  {
    allocate( length );
  }
  else
  {
    mLength = 0;
    mRawStorage = nullptr;
    mAlignedStorage = nullptr;
  }
}

template< typename ElementType>
AlignedArray<ElementType>::AlignedArray( AlignedArray< ElementType > && rhs )
 : AlignedArray( rhs.alignmentElements() )
{
  swap( rhs );
}

template< typename ElementType>
AlignedArray<ElementType> & AlignedArray<ElementType>::operator=( AlignedArray< ElementType > && rhs )
{
  if( mAlignment != rhs.mAlignment )
  {
    throw std::logic_error( "AlignedArray:: objects in move assignment must have the same alignment." );
  }
  swap( rhs );
  rhs.deallocate(); // Not strictly necessary
  return *this;
}

template< typename ElementType>
AlignedArray<ElementType>::~AlignedArray()
{
  deallocate();
}

template< typename ElementType>
void AlignedArray<ElementType>::resize( std::size_t newLength )
{
  deallocate();
  if( newLength > 0 )
  {
    allocate( newLength );
  }
}

/**
 * Swap the content with another AlignedArray object.
 * This method has strong exception safety and does not involve memory allocation or vector copy operations.
 * @param rhs The object to be swapped with.
 * @throw std::logic_error If the objects do not have the same alignment.
 */
template< typename ElementType>
void AlignedArray<ElementType>::swap( AlignedArray<ElementType>& rhs )
{
  if( mAlignment != rhs.mAlignment )
  {
    throw std::logic_error( "AlignedArray::swap(): objects to be swapped must have the same alignment." );
  }
  std::swap( mLength, rhs.mLength );
  std::swap( mRawStorage, rhs.mRawStorage );
  std::swap( mAlignedStorage, rhs.mAlignedStorage );
}

template< typename ElementType>
void AlignedArray<ElementType>::allocate( std::size_t length )
{
  // Special case if the guaranteed alignment of the memory allocation
  // is smaller than the size of the element. This might happen, for example,
  // with std::complex<> types.
  const std::size_t worstCaseLengthElements =
    (alignof(ElementType) == sizeof(ElementType))
    ? length + mAlignment - 1 : length + mAlignment;
  mRawStorage = new ElementType[worstCaseLengthElements];
  void* retPtr = static_cast<void*>(mRawStorage);
  std::size_t space = worstCaseLengthElements*sizeof(ElementType);
  // ignoring the return value is safe, because it is identical to the updated retPtr parameter.
  std::align( mAlignment*sizeof(ElementType), length*sizeof(ElementType), retPtr, space );
  if( retPtr == nullptr )
  {
    deallocate(); // Set the object into a defined, valid state.
    throw std::bad_alloc();
  }
  // todo: should we check the updated 'space' parameter?
  mAlignedStorage = static_cast<ElementType*>(retPtr);
  mLength = length;
}

template< typename ElementType>
void AlignedArray<ElementType>::deallocate()
{
  delete[] mRawStorage;
  mRawStorage = nullptr;
  mAlignedStorage = nullptr;
  mLength = 0;
}

} // namespace efl
} // namespace visr

#endif // #ifndef VISR_LIBEFL_ALIGNED_ARRAY_HPP_INCLUDED
