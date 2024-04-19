/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include <libpythoncomponents/wrapper.hpp>

#include <libvisr/constants.hpp>
#include <libvisr/signal_flow_context.hpp>

#include <librrl/audio_signal_flow.hpp>

#include <librrl/integrity_checking.hpp>

#include <libefl/basic_matrix.hpp>

#include <libpythonsupport/initialisation_guard.hpp>


#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

namespace visr
{
namespace pythoncomponents
{
namespace test
{

// Wrap a Python component in a Python file (as opposed to a multi-file package)
BOOST_AUTO_TEST_CASE( WrapSingleFileModule )
{
  pythonsupport::InitialisationGuard::initialise();

  std::string moduleName = "pythonAtoms";

  boost::filesystem::path basePath{CMAKE_CURRENT_SOURCE_DIR};
  boost::filesystem::path const modulePath = basePath / "python";
  
  const std::size_t blockSize{64};
  const SamplingFrequencyType samplingFrequency{48000};
  std::size_t const numBlocks{16};

  SignalFlowContext ctxt( blockSize, samplingFrequency );

  // Instantiate the atomic component (implemented in Python)
  // by a mixture or positional and keyword constructor arguments
  Wrapper pyAtom1( ctxt, "PythonAtom", nullptr,
                   moduleName.c_str(),
                   "PythonAdder",
                   "3,", "{'width':5}",
                   modulePath.c_str() );

  std::stringstream errMsg;
  bool res = rrl::checkConnectionIntegrity( pyAtom1, true, errMsg );
  BOOST_CHECK_MESSAGE( res, errMsg.str() );

  rrl::AudioSignalFlow flow( pyAtom1 );

  std::size_t const numInputChannels = 15;
  std::size_t const numOutputChannels = 5;

  std::vector<SampleType*> inputPtr( numInputChannels, nullptr );
  std::vector<SampleType*> outputPtr( numOutputChannels, nullptr );
  
  efl::BasicMatrix<SampleType> inputData( numInputChannels, blockSize* numBlocks );
  // TODO: Fill the input data with something useful

  efl::BasicMatrix<SampleType> outputData( numOutputChannels, blockSize* numBlocks );

  for( std::size_t blockIdx(0); blockIdx < numBlocks; ++blockIdx )
  {
    for( std::size_t idx(0); idx < numInputChannels; ++idx )
    {
      inputPtr[idx] = inputData.row(idx) + blockIdx*blockSize;
    }
    for( std::size_t idx(0); idx < numOutputChannels; ++idx )
    {
      outputPtr[idx] = outputData.row(idx) + blockIdx*blockSize;
    }
    flow.process( &inputPtr[0], &outputPtr[0] );
  }
}

// Wrap a Python component contained in a multi-file package)
BOOST_AUTO_TEST_CASE( WrapMultiFilePackage )
{
  pythonsupport::InitialisationGuard::initialise();

  std::string moduleName = "testmodule";

  boost::filesystem::path basePath{CMAKE_CURRENT_SOURCE_DIR};
  boost::filesystem::path const modulePath = basePath / "python";

  const std::size_t blockSize{64};
  const SamplingFrequencyType samplingFrequency{48000};
  std::size_t const numBlocks{16};

  SignalFlowContext ctxt( blockSize, samplingFrequency );

  // Instantiate the atomic component (implemented in Python)
  // by a mixture or positional and keyword constructor arguments
  Wrapper pyAtom1( ctxt, "PythonAtom", nullptr,
                  moduleName.c_str(),
                  "Adder",
                  "3,", "{'width':5}",
                  modulePath.string().c_str() );

  std::stringstream errMsg;
  bool res = rrl::checkConnectionIntegrity( pyAtom1, true, errMsg );
  BOOST_CHECK_MESSAGE( res, errMsg.str() );

  rrl::AudioSignalFlow flow( pyAtom1 );

  std::size_t const numInputChannels = 15;
  std::size_t const numOutputChannels = 5;

  std::vector<SampleType*> inputPtr( numInputChannels, nullptr );
  std::vector<SampleType*> outputPtr( numOutputChannels, nullptr );

  efl::BasicMatrix<SampleType> inputData( numInputChannels, blockSize* numBlocks );
  // TODO: Fill the input data with something useful

  efl::BasicMatrix<SampleType> outputData( numOutputChannels, blockSize* numBlocks );

  for( std::size_t blockIdx(0); blockIdx < numBlocks; ++blockIdx )
  {
    for( std::size_t idx(0); idx < numInputChannels; ++idx )
    {
      inputPtr[idx] = inputData.row(idx) + blockIdx*blockSize;
    }
    for( std::size_t idx(0); idx < numOutputChannels; ++idx )
    {
      outputPtr[idx] = outputData.row(idx) + blockIdx*blockSize;
    }
    flow.process( &inputPtr[0], &outputPtr[0] );
  }
}

// Wrap a Python component contained in a nested multi-file package)
BOOST_AUTO_TEST_CASE(WrapNestedPackage)
{
  pythonsupport::InitialisationGuard::initialise();

  std::string moduleName = "subpackagemodule.level1.level2";

  boost::filesystem::path basePath{ CMAKE_CURRENT_SOURCE_DIR };
  boost::filesystem::path const modulePath = basePath / "python";

  const std::size_t blockSize{ 64 };
  const SamplingFrequencyType samplingFrequency{ 48000 };
  std::size_t const numBlocks{ 16 };

  SignalFlowContext ctxt(blockSize, samplingFrequency);

  // Instantiate the atomic component (implemented in Python)
  // by a mixture or positional and keyword constructor arguments
  Wrapper pyAtom1(ctxt, "PythonAtom", nullptr,
    moduleName.c_str(),
    "Level2Adder",
    "3,", "{'width':5}",
    modulePath.string().c_str());

  std::stringstream errMsg;
  bool res = rrl::checkConnectionIntegrity(pyAtom1, true, errMsg);
  BOOST_CHECK_MESSAGE(res, errMsg.str());

  rrl::AudioSignalFlow flow(pyAtom1);

  std::size_t const numInputChannels = 15;
  std::size_t const numOutputChannels = 5;

  std::vector<SampleType*> inputPtr(numInputChannels, nullptr);
  std::vector<SampleType*> outputPtr(numOutputChannels, nullptr);

  efl::BasicMatrix<SampleType> inputData(numInputChannels, blockSize* numBlocks);
  // TODO: Fill the input data with something useful

  efl::BasicMatrix<SampleType> outputData(numOutputChannels, blockSize* numBlocks);

  for (std::size_t blockIdx(0); blockIdx < numBlocks; ++blockIdx)
  {
    for (std::size_t idx(0); idx < numInputChannels; ++idx)
    {
      inputPtr[idx] = inputData.row(idx) + blockIdx * blockSize;
    }
    for (std::size_t idx(0); idx < numOutputChannels; ++idx)
    {
      outputPtr[idx] = outputData.row(idx) + blockIdx * blockSize;
    }
    flow.process(&inputPtr[0], &outputPtr[0]);
  }
}

// Wrap a Python component nested multi-file package specified by a namespaced name)
BOOST_AUTO_TEST_CASE(WrapNamespacedClass)
{
  pythonsupport::InitialisationGuard::initialise();

  std::string moduleName = "nestednamespacemodule";

  boost::filesystem::path basePath{ CMAKE_CURRENT_SOURCE_DIR };
  boost::filesystem::path const modulePath = basePath / "python";

  const std::size_t blockSize{ 64 };
  const SamplingFrequencyType samplingFrequency{ 48000 };
  std::size_t const numBlocks{ 16 };

  SignalFlowContext ctxt(blockSize, samplingFrequency);

  // Instantiate the atomic component (implemented in Python)
  // by a mixture or positional and keyword constructor arguments
  Wrapper pyAtom1(ctxt, "PythonAtom", nullptr,
    moduleName.c_str(),
    "level1.level2.Level2Adder",
    "3,", "{'width':5}",
    modulePath.string().c_str());

  std::stringstream errMsg;
  bool res = rrl::checkConnectionIntegrity(pyAtom1, true, errMsg);
  BOOST_CHECK_MESSAGE(res, errMsg.str());

  rrl::AudioSignalFlow flow(pyAtom1);

  std::size_t const numInputChannels = 15;
  std::size_t const numOutputChannels = 5;

  std::vector<SampleType*> inputPtr(numInputChannels, nullptr);
  std::vector<SampleType*> outputPtr(numOutputChannels, nullptr);

  efl::BasicMatrix<SampleType> inputData(numInputChannels, blockSize* numBlocks);
  // TODO: Fill the input data with something useful

  efl::BasicMatrix<SampleType> outputData(numOutputChannels, blockSize* numBlocks);

  for (std::size_t blockIdx(0); blockIdx < numBlocks; ++blockIdx)
  {
    for (std::size_t idx(0); idx < numInputChannels; ++idx)
    {
      inputPtr[idx] = inputData.row(idx) + blockIdx * blockSize;
    }
    for (std::size_t idx(0); idx < numOutputChannels; ++idx)
    {
      outputPtr[idx] = outputData.row(idx) + blockIdx * blockSize;
    }
    flow.process(&inputPtr[0], &outputPtr[0]);
  }
}

// Wrap a Python component nested multi-file package specified by a namespaced name,
// where one level of nesting is done in the nested module name, the other in the namespaced class name.
BOOST_AUTO_TEST_CASE(WrapNamespacedClassSplit)
{
  pythonsupport::InitialisationGuard::initialise();

  std::string moduleName = "nestednamespacemodule.level1";

  boost::filesystem::path basePath{ CMAKE_CURRENT_SOURCE_DIR };
  boost::filesystem::path const modulePath = basePath / "python";

  const std::size_t blockSize{ 64 };
  const SamplingFrequencyType samplingFrequency{ 48000 };
  std::size_t const numBlocks{ 16 };

  SignalFlowContext ctxt(blockSize, samplingFrequency);

  // Instantiate the atomic component (implemented in Python)
  // by a mixture or positional and keyword constructor arguments
  Wrapper pyAtom1(ctxt, "PythonAtom", nullptr,
    moduleName.c_str(),
    "     level2 .   Level2Adder", // Adds some arbitrary whitespace around argument, which should be ignored (as in Python)
    "3,", "{'width':5}",
    modulePath.string().c_str());

  std::stringstream errMsg;
  bool res = rrl::checkConnectionIntegrity(pyAtom1, true, errMsg);
  BOOST_CHECK_MESSAGE(res, errMsg.str());

  rrl::AudioSignalFlow flow(pyAtom1);

  std::size_t const numInputChannels = 15;
  std::size_t const numOutputChannels = 5;

  std::vector<SampleType*> inputPtr(numInputChannels, nullptr);
  std::vector<SampleType*> outputPtr(numOutputChannels, nullptr);

  efl::BasicMatrix<SampleType> inputData(numInputChannels, blockSize* numBlocks);
  // TODO: Fill the input data with something useful

  efl::BasicMatrix<SampleType> outputData(numOutputChannels, blockSize* numBlocks);

  for (std::size_t blockIdx(0); blockIdx < numBlocks; ++blockIdx)
  {
    for (std::size_t idx(0); idx < numInputChannels; ++idx)
    {
      inputPtr[idx] = inputData.row(idx) + blockIdx * blockSize;
    }
    for (std::size_t idx(0); idx < numOutputChannels; ++idx)
    {
      outputPtr[idx] = outputData.row(idx) + blockIdx * blockSize;
    }
    flow.process(&inputPtr[0], &outputPtr[0]);
  }
}

} // namespace test
} // namespace pythoncomponents
} // namespace visr
