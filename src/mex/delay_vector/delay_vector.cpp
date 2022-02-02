/* Copyright Institute of Sound and Vibration Research - All Rights reserved */

#include <libsignalflows/delay_vector.hpp>

#include <libmexsupport/export_symbol.hpp>
#include <libmexsupport/mex_wrapper.hpp>

#include <libvisr/constants.hpp>

// for the moment, do some basic tests in the main mex function
#include <libefl/basic_vector.hpp>
#include <librcl/delay_vector.hpp>

#include <mex.h> 
#include <matrix.h>

#include <ciso646>
#include <string>

static char const * usage()
{
  return "Usage: output = delay_vector( input [, parameterMessages], blockLength, samplingFrequency )";
}

VISR_MEXSUPPORT_EXPORT_SYMBOL
void mexFunction(int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[] )
{
  using namespace visr;
  try
  {
    if( (nrhs < 3) or( nrhs > 4 ) )
    {
      mexErrMsgIdAndTxt( "visr:mexInterface:inputArgumentCount", usage() );
    }
    if( nlhs != 1 )
    {
      mexErrMsgIdAndTxt( "visr:mexInterface:outputArgumentCount", usage() );
    }

    if( mxIsComplex( prhs[0] ) or not( mxIsSingle( prhs[0] ) or mxIsDouble( prhs[0] ) ) )
    {
      mexErrMsgIdAndTxt( "visr:mexInterface:ArgumentType",
        "The parameter \"message\" mut be a matrix of type single or double" );
    }

    bool const hasParameterArg = (nrhs == 4);

    std::size_t const periodSizeParamIdx = hasParameterArg ? 2 : 1;
    std::size_t const samplingFreqParamIdx = hasParameterArg ? 3 : 2;

    if( (not mxIsNumeric( prhs[periodSizeParamIdx] ) or( mxGetN( prhs[periodSizeParamIdx] ) != 1 )
      or( mxGetM( prhs[periodSizeParamIdx] ) != 1 )) )
    {
      mexErrMsgIdAndTxt( "visr:mexInterface:argumentType",
        "The parameter \"period\" mut be a scalar numerical value" );
    }

    if( (not mxIsNumeric( prhs[samplingFreqParamIdx] ) or( mxGetN( prhs[samplingFreqParamIdx] ) != 1 )
      or( mxGetM( prhs[samplingFreqParamIdx] ) != 1 )) )
    {
      mexErrMsgIdAndTxt( "visr:mexInterface:argumentType",
        "The parameter \"samplingFrequency\" mut be a scalar numerical value" );
    }

    std::size_t const periodSize
      = static_cast<ril::SamplingFrequencyType>(mxGetScalar( prhs[periodSizeParamIdx] ));

    ril::SamplingFrequencyType const samplingFrequency
      = static_cast<ril::SamplingFrequencyType>(mxGetScalar( prhs[samplingFreqParamIdx] ));

    const std::size_t cNumberOfChannels = 4;
    const std::size_t cInterpolationLength = 4 * periodSize;

    const rcl::DelayVector::InterpolationType intType = rcl::DelayVector::InterpolationType::Linear;

    signalflows::DelayVector flow( cNumberOfChannels, cInterpolationLength, intType, periodSize, samplingFrequency );

    efl::BasicVector<ril::SampleType> initialDelays( cNumberOfChannels, ril::cVectorAlignmentSamples );
    initialDelays.fillValue( static_cast<visr::SampleType>(0.0002) );

    efl::BasicVector<ril::SampleType> initialGains( cNumberOfChannels, ril::cVectorAlignmentSamples );
    initialGains.fillValue( static_cast<visr::SampleType>(0.735) );

    flow.setDelay( initialDelays );
    flow.setGain( initialGains );

    mexsupport::MexWrapper mexWrapper( flow, prhs[0], plhs[0],
     hasParameterArg ? prhs[1] : nullptr );

    mexWrapper.process();
  }
  catch( std::exception const & e )
  {
    mexErrMsgIdAndTxt( "visr:mexInterface:runtimeError", e.what() );
  }
}
