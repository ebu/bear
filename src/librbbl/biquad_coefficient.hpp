/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_RBBL_BIQUAD_COEFFICIENT_HPP_INCLUDED
#define VISR_RBBL_BIQUAD_COEFFICIENT_HPP_INCLUDED

#include <boost/property_tree/ptree_fwd.hpp>

#include "export_symbols.hpp"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace rbbl
{

/**
 * Data type for describing Biquad IIR coefficients, also known as second-order sections.
 * @tparam CoeffType Type of the contained elements.
 */
template< typename CoeffType >
class VISR_RBBL_LIBRARY_SYMBOL BiquadCoefficient
{
public:
  /**
   * The number of coefficients to describe one biquad section.
   */
  static const std::size_t cNumberOfCoeffs = 5;

  /**
   * Create an default object corresponding to a flat EQ.
   */
  BiquadCoefficient()
  {
    mCoeffs.fill( static_cast<CoeffType>(0.0) );
    mCoeffs[0] = static_cast<CoeffType>(1.0);
  }

  BiquadCoefficient( BiquadCoefficient<CoeffType> const & rhs ) = default;

  /**
   * Create a BiquadCoefficient objects from JSON and XML representations,
   * modeling the 'named constructor' idiom.
   */
  //@{
  static BiquadCoefficient fromJson( boost::property_tree::ptree const & tree );

  static BiquadCoefficient fromJson( std::basic_istream<char> & stream );

  static BiquadCoefficient fromJson( std::string const & str );

  static BiquadCoefficient fromXml( boost::property_tree::ptree const & tree );

  static BiquadCoefficient fromXml( std::basic_istream<char> & stream );

  static BiquadCoefficient fromXml( std::string const & str );
//@}

  BiquadCoefficient( CoeffType b0, CoeffType b1, CoeffType b2,
		     CoeffType a1, CoeffType a2 )
  {
    mCoeffs[0] = b0;
    mCoeffs[1] = b1;
    mCoeffs[2] = b2;
    mCoeffs[3] = a1;
    mCoeffs[4] = a2;
  }

  BiquadCoefficient( std::initializer_list< CoeffType > const & coeffs )
  {
    if( coeffs.size() != mCoeffs.size() )
    {
      throw std::invalid_argument( "BiquadCoefficient: Initialisation vector has wrong number of elements." );
    }
    std::copy( coeffs.begin(), coeffs.end(), mCoeffs.begin() );
  }

  // BiquadCoefficient( const BiquadCoefficient & rhs ) = default;

  BiquadCoefficient & operator=( BiquadCoefficient const & rhs )
  {
    mCoeffs = rhs.mCoeffs;
    return *this;
  }

  CoeffType const * data() const { return &mCoeffs[0]; }

  CoeffType * data() { return &mCoeffs[0]; }

  CoeffType const & operator[]( std::size_t idx ) const
  {
    return mCoeffs[ idx ];
  }

  CoeffType & operator[]( std::size_t idx )
  {
    return mCoeffs[ idx ];
  }

  CoeffType const & at( std::size_t idx ) const
  {
    return mCoeffs.at( idx );
  }

  CoeffType & at( std::size_t idx )
  {
    return mCoeffs.at( idx );
  }


  CoeffType const & b0() const { return mCoeffs[0]; }

  CoeffType const & b1() const { return mCoeffs[1]; }

  CoeffType const & b2() const { return mCoeffs[2]; }

  CoeffType const & a1() const { return mCoeffs[3]; }

  CoeffType const & a2() const { return mCoeffs[4]; }


  CoeffType& b0() { return mCoeffs[0]; }

  CoeffType& b1() { return mCoeffs[1]; }

  CoeffType& b2() { return mCoeffs[2]; }

  CoeffType& a1() { return mCoeffs[3]; }

  CoeffType& a2() { return mCoeffs[4]; }

  void loadJson( boost::property_tree::ptree const & tree );

  void loadJson( std::basic_istream<char> & );

  void loadJson( std::string const & str );

  void loadXml( boost::property_tree::ptree const & tree );

  void loadXml( std::basic_istream<char> & );

  void loadXml( std::string const & str );

  /**
   * Write a biquad coefficient set into Json and XML representations.
   */
  //@{
  void writeJson( boost::property_tree::ptree & tree ) const;

  void writeJson( std::basic_ostream<char> & stream ) const;

  void writeJson( std::string & str ) const;

  void writeXml( boost::property_tree::ptree & tree ) const;

  void writeXml ( std::basic_ostream<char> & stream ) const;

  void writeXml ( std::string & str ) const;

  //@}
private:
  /**
   * The internal data representation.
   */
  std::array<CoeffType,5> mCoeffs;
};

template< class CoeffType >
class VISR_RBBL_LIBRARY_SYMBOL BiquadCoefficientList
{
public:
  using Container = typename std::vector< BiquadCoefficient< CoeffType > >;
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  /**
   * Default constructor, creates an empty list of biquad parameters.
   */
  BiquadCoefficientList() = default;

  explicit BiquadCoefficientList( const std::size_t initialSize )
    : mBiquads( initialSize )
  {
  }

  /**
   * Default copy constructor (required for use within aSTL data structure as, for instance, in BiquadCoefficientList.
   */
  BiquadCoefficientList( BiquadCoefficientList const & rhs ) = default;

  BiquadCoefficientList( std::initializer_list<BiquadCoefficient<CoeffType> > const & initList )
    : mBiquads( initList )
  {
  }

  static BiquadCoefficientList fromJson( boost::property_tree::ptree const & tree );

  static BiquadCoefficientList fromJson( std::basic_istream<char> & stream );

  static BiquadCoefficientList fromJson( std::string const & str );

  static BiquadCoefficientList fromXml( boost::property_tree::ptree const & tree );

  static BiquadCoefficientList fromXml( std::basic_istream<char> & stream );

  static BiquadCoefficientList fromXml( std::string const & str );

  /**
   * Assign the content from another BiquadCoefficientList with consistent size.
   * @throw std::invalid_argument If \p rhs has a different size.
   */
  BiquadCoefficientList & operator=(BiquadCoefficientList const & rhs)
  {
    if( rhs.size() != size() )
    {
      std::invalid_argument( "BiquadCoefficientList: Size of argument to be assigned does not match." );
    }
    mBiquads = rhs.mBiquads;
    return *this;
  }

  std::size_t size() const { return mBiquads.size(); }

  void resize( std::size_t newSize )
  {
    mBiquads.resize( newSize, BiquadCoefficient<CoeffType>() );
  }

  BiquadCoefficient<CoeffType> const & operator[]( std::size_t idx ) const { return mBiquads[idx]; }
  BiquadCoefficient<CoeffType> & operator[]( std::size_t idx ) { return mBiquads[idx]; }

  BiquadCoefficient<CoeffType> const & at( std::size_t idx ) const { return mBiquads.at( idx ); }
  BiquadCoefficient<CoeffType> & at( std::size_t idx ) { return mBiquads.at( idx ); }

  iterator begin() { return mBiquads.begin(); }
  iterator end() { return mBiquads.end(); }

  const_iterator begin() const { return mBiquads.begin(); }
  const_iterator end() const { return mBiquads.end(); }

  /**
   * DE-serialization from text formats.
   */
  //@{
  /**
   * Initialise from a JSON representation, provided as a Boost property tree ptree object.
   */
  void loadJson( boost::property_tree::ptree const & tree );

  /**
  * Initialise from a JSON representation, provided as an input stream containing JSON text data.
  */
  void loadJson( std::basic_istream<char> & stream );

  /**
  * Initialise from a JSON representation, provided as a string containing JSON text data.
  */
  void loadJson( std::string const & str );

  /**
   * Initialise from an XML representation, provided as a Boost property tree ptree object.
   */
  void loadXml( boost::property_tree::ptree const & tree );

  /**
   * Initialise from an XML representation, provided as an input stream containing JSON text data.
   */
  void loadXml( std::basic_istream<char> & stream );

  /**
   * Initialise from an XML representation, provided as a string containing JSON text data.
   */
  void loadXml( std::string const & str );
  //@}

  /**
   * Serialisation into textual formats.
   */
  //@{
  /**
   * Write to a boost property tree object (ptree) that can be serialised to an XML document (or part thereof)
   * @note the ptree representations for XML and JSON differ slightly, so different implementations are needed.
   */
  void writeJson( boost::property_tree::ptree & tree ) const;

  /**
   * Write an XML representation to an output stream.
   */
  void writeJson( std::basic_ostream<char> & stream ) const;

  /**
   * Write an XML representation into a string.
   */
  void writeJson( std::string & str ) const;

  /**
  * Write to a boost property tree object (ptree) that can be serialised to an XML document (or part thereof)
  * @note the ptree representations for XML and JSON differ slightly, so different implementations are needed.
  */
  void writeXml( boost::property_tree::ptree & tree ) const;

  /**
  * Write an XML representation to an output stream.
  */
  void writeXml( std::basic_ostream<char> & stream ) const;

  /**
  * Write an XML representation into a string.
  */
  void writeXml( std::string & str ) const;
  //@}


private:
  Container mBiquads;
};

template<typename CoeffType >
class VISR_RBBL_LIBRARY_SYMBOL BiquadCoefficientMatrix
{
public:
  explicit BiquadCoefficientMatrix( std::size_t numberOfFilters, std::size_t numberOfBiquads );

explicit BiquadCoefficientMatrix( std::initializer_list<
                                  std::initializer_list<
                                  BiquadCoefficient<CoeffType> > > const & initList );

  ~BiquadCoefficientMatrix();

  static BiquadCoefficientMatrix fromJson( boost::property_tree::ptree const & tree );

  static BiquadCoefficientMatrix fromJson( std::basic_istream<char> & stream );

  static BiquadCoefficientMatrix fromJson( std::string const & str );

  static BiquadCoefficientMatrix fromXml( boost::property_tree::ptree const & tree );

  static BiquadCoefficientMatrix fromXml( std::basic_istream<char> & stream );

  static BiquadCoefficientMatrix fromXml( std::string const & str );

  std::size_t numberOfFilters() const { return mRows.size(); }
  std::size_t numberOfSections() const { return mRows.empty() ? 0 : mRows[0].size(); }

  void resize( std::size_t numberOfFilters, std::size_t numberOfBiquads );
  BiquadCoefficientList<CoeffType> const & operator[]( std::size_t rowIdx ) const { return mRows[rowIdx]; }
  BiquadCoefficientList<CoeffType> & operator[]( std::size_t rowIdx ) { return mRows[rowIdx]; }

  BiquadCoefficient<CoeffType> const & operator()( std::size_t rowIdx, std::size_t colIdx ) const { return mRows[rowIdx][colIdx]; }
  BiquadCoefficient<CoeffType> & operator()( std::size_t rowIdx, std::size_t colIdx ) { return mRows[rowIdx][colIdx]; }

  /**
   * Set the biquad sections for a complete filter specification (a row in the matrix)
   * If \p newFilter has fewer sections than the matrix, the rest is filled with default values.
   * @throw std::out_of_range If \p filterIdx exceeds the number of biquad sections.
   * @throw std::invalid_argument If \p nnewFilters has more elements than the column number of the matrix.
   */
  void setFilter( std::size_t filterIdx, BiquadCoefficientList<CoeffType> const & newFilter );

  /**
  * Deserialisation from text formats.
  */
  //@{
  /**
  * Initialise from a JSON representation, provided as a Boost property tree ptree object.
  */
  void loadJson( boost::property_tree::ptree const & tree );

  /**
  * Initialise from a JSON representation, provided as an input stream containing JSON text data.
  */
  void loadJson( std::basic_istream<char> & stream );

  /**
  * Initialise from a JSON representation, provided as a string containing JSON text data.
  */
  void loadJson( std::string const & str );

  /**
  * Initialise from an XML representation, provided as a Boost property tree ptree object.
  */
  void loadXml( boost::property_tree::ptree const & tree );

  /**
  * Initialise from an XML representation, provided as an input stream containing JSON text data.
  */
  void loadXml( std::basic_istream<char> & stream );

  /**
  * Initialise from an XML representation, provided as a string containing JSON text data.
  */
  void loadXml( std::string const & str );
  //@}

  /**
  * Serialisation into textual formats.
  */
  //@{
  /**
  * Write to a boost property tree object (ptree) that can be serialised to an XML document (or part thereof)
  * @note the ptree representations for XML and JSON differ slightly, so different implementations are needed.
  */
  void writeJson( boost::property_tree::ptree & tree ) const;

  /**
  * Write an XML representation to an output stream.
  */
  void writeJson( std::basic_ostream<char> & stream ) const;

  /**
  * Write an XML representation into a string.
  */
  void writeJson( std::string & str ) const;

  /**
  * Write to a boost property tree object (ptree) that can be serialised to an XML document (or part thereof)
  * @note the ptree representations for XML and JSON differ slightly, so different implementations are needed.
  */
  void writeXml( boost::property_tree::ptree & tree ) const;

  /**
  * Write an XML representation to an output stream.
  */
  void writeXml( std::basic_ostream<char> & stream ) const;

  /**
  * Write an XML representation into a string.
  */
  void writeXml( std::string & str ) const;
  //@}

private:
  using ContainerType = std::vector< BiquadCoefficientList<CoeffType> >;

  ContainerType mRows;
};

} // namespace rbbl
} // namespace visr

#endif // VISR_RBBL_BIQUAD_COEFFICIENT_HPP_INCLUDED
