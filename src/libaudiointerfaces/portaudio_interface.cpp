/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "portaudio_interface.hpp"

// TODO: Eliminate this dependency!
#include <librrl/communication_area.hpp>

#include <libvisr/detail/compose_message_string.hpp>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <portaudio.h>

#include <ciso646> // should not be necessary in C++11, but MSVC is non-compliant here
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits> // for static type checking due to current limitations of the system.

namespace visr
{
namespace audiointerfaces
{
namespace // unnamed
{
  using namespace rrl;
  using TranslateSampleFormatMapType = std::map<PortaudioInterface::Config::SampleFormat::Type, PaSampleFormat >;
  TranslateSampleFormatMapType const cTranslateSampleFormatMap = {
    { PortaudioInterface::Config::SampleFormat::signedInt8Bit, paInt8 },
    { PortaudioInterface::Config::SampleFormat::unsignedInt8Bit, paUInt8 },
    { PortaudioInterface::Config::SampleFormat::signedInt16Bit, paInt16 },
    { PortaudioInterface::Config::SampleFormat::signedInt24Bit, paInt24 },
    { PortaudioInterface::Config::SampleFormat::signedInt32Bit, paInt32 },
    { PortaudioInterface::Config::SampleFormat::float32Bit, paFloat32 }
  };
  /**
   * Translate the format enumeration into the portaudio type.
   * @throw std::invalid_argument if the sample format does not exist in portaudio
   */
  PaSampleFormat translateSampleFormat( PortaudioInterface::Config::SampleFormat::Type format,
                                       bool interleaved )
  {
    auto const findIt = cTranslateSampleFormatMap.find( format );
    if( findIt == cTranslateSampleFormatMap.end() ) {
      throw std::invalid_argument( "The given sample format does not match a sample format in portaudio." );
    }
    if( interleaved )
    {
      return findIt->second;
    }
    else
    {
      return (findIt->second) bitor paNonInterleaved;
    }
  }
  
  
  using TranslateHostApiNameToTypeMapType = std::map< std::string, PaHostApiTypeId >;
  TranslateHostApiNameToTypeMapType const cTranslateHostApiNameToType =
  {
    { "default", paInDevelopment }, // we use this reserved type for the value "default"
    { "DirectSound", paDirectSound },
    { "MME", paMME },
    { "ASIO", paASIO },
    { "SoundManager", paSoundManager },
    { "CoreAudio", paCoreAudio },
    { "OSS", paOSS },
    { "ALSA", paALSA },
    { "AL", paAL },
    { "BeOS", paBeOS },
    { "WDMKS", paWDMKS },
    { "JACK", paJACK },
    { "WASAPI", paWASAPI },
    { "AudioScienceHPI", paAudioScienceHPI }
  };
  
  PaHostApiTypeId translateHostApiName( std::string const & apiName )
  {
    auto const findIt = cTranslateHostApiNameToType.find( apiName );
    if( findIt == cTranslateHostApiNameToType.end( ) )
    {
      throw std::invalid_argument( "The given API name is not a supported portaudio API." );
    }
    return findIt->second;
  }
  
} // unnamed namespace
  
  /******************************************************************************/
  /* Definition of the internal implementation class PortaudioInterface::Impl   */
  
  class PortaudioInterface::Impl
  {
    friend class PortaudioInterface;
  public:
    
    explicit Impl(Configuration const & baseConfig, std::string const & config );
    
    ~Impl( );
    
    void start();
    
    void stop();
    
    bool registerCallback( AudioCallback callback, void* userData );
    
    bool unregisterCallback( AudioCallback audioCallback );
  private:
    
    PortaudioInterface::Config parseSpecificConf( std::string const & config );
    
    
    static int sEngineCallback( const void *input,
                               void *output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData );
    
    int engineCallbackFunction( const void *input,
                               void *output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags );
    
    /**
     * Convert and transfer the audio samples from the input
     * of the Portaudio callback function to an array of sample buffers
     * where it is passed to the audio processing callback.
     */
    void transferCaptureBuffers( void const * input );
    
    /**
     * Transfer and convert the audio samples generated by the audio
     * processing callback to the output argument of the Portaudio callback
     * function.
     */
    void transferPlaybackBuffers( void * output );
    
    std::size_t const mNumCaptureChannels;
    
    std::size_t const mNumPlaybackChannels;
    std::size_t const mPeriodSize;
    std::size_t const mSampleRate;

    Config::SampleFormat::Type mSampleFormat;

    bool mInterleaved;
    
    std::string mHostApiName;
    
    PaStream * mStream;
    
    Base::AudioCallback mCallback;
    
    void* mCallbackUserData;
    
    std::unique_ptr<rrl::CommunicationArea<SampleType> > mCommunicationBuffer;
    
    /**
     * Buffer to hold the pointers to the sample vectors for the input
     * of the audio processor.
     * These samples are written from the capture ports of the sound
     * interface and then passed to the audio processing callback function.
     */
    std::vector< SampleType * > mCaptureSampleBuffers;
    
    /**
     * Buffer to hold the pointers to the sample vectors for the output
     * of the audio processor.
     * These samples are generated by the audio processing callback
     * function and then passed to the playback argument of the
     * portaudio callback function.
     */
    std::vector< SampleType * > mPlaybackSampleBuffers;
  };
  
  /******************************************************************************/
  /* Implementation of the PortaudioInterface::Impl class                       */

  namespace // unnamed
  {
    /**
     * Return a list of audio devices that have at least minChannels channels in the specified direction.
     */
    std::string listDeviceNames( PaHostApiIndex const hostApiIndex,
                                 bool isInput,
                                 std::size_t minChannels = 1)
    {
      // Prepare a list of devices that have at least one output channel in the given direction.
      std::stringstream deviceList;
      PaHostApiInfo const * hostApiInfo = Pa_GetHostApiInfo(hostApiIndex);
      PaDeviceIndex const numDevices = hostApiInfo->deviceCount;
      bool empty{ true };
      for (PaDeviceIndex idx(0); idx < numDevices; ++idx)
      {
        PaDeviceIndex const globalIdx = Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, idx);
        PaDeviceInfo const * info = Pa_GetDeviceInfo(globalIdx);
        if ((isInput and info->maxInputChannels >= 1) or ((not isInput) and info->maxOutputChannels >= 1))
        {
          if( empty )
          {
            empty = false;
          }
          else
          {
            deviceList << ", ";
          }
          deviceList << "\"" << info->name << "\"";
        }
      }
      return deviceList.str();
    }

    /**
     * Find a device index with a given name and direction.
     * If the parameter \p is left empty, return the default device for the given host api and direction.
     * @throw std::invalid_argument if no device with this name exists.
     * @throw std::invalid_argument if the device does not provide the requested number of channels.
     */
    PaDeviceIndex findDevice( std::string const & name,
                              PaHostApiIndex const hostApiIndex,
                              std::size_t minChannels,
                              bool isInput )
    {
      PaHostApiInfo const * hostApiInfo = Pa_GetHostApiInfo(hostApiIndex);
      PaDeviceIndex deviceIdx = paNoDevice;
      bool const useDefault = name.empty();
      if( useDefault )
      {
        deviceIdx = isInput ? hostApiInfo->defaultInputDevice 
                            : hostApiInfo->defaultOutputDevice;
      }
      else
      {
        PaDeviceIndex const numDevices = hostApiInfo->deviceCount;
        {
          for (PaDeviceIndex idx(0); idx < numDevices; ++idx)
          {
            PaDeviceIndex const globalIdx = Pa_HostApiDeviceIndexToDeviceIndex(hostApiIndex, idx);
            PaDeviceInfo const * info = Pa_GetDeviceInfo(globalIdx);
            // Compare only up to the length of the device name returned by PortAudio.
            // A device name is considered as matching if it's equal up to that length.
            // This is done because PortAudio sometimes truncates its device names.
            if (std::strncmp(name.c_str(), info->name, std::strlen(info->name) ) == 0)
            {
              deviceIdx = globalIdx;
              break;
            }
          }
        }
      }
      if( deviceIdx == paNoDevice )
      {
        // We list all devices that have at least 1 channel in the specified direction (minChannels=1)
        std::string const deviceNames = listDeviceNames(hostApiIndex, isInput, 1 /*minChannels*/);
        std::stringstream errMsg;
        errMsg << "PortAudioInterface: ";
        if( useDefault )
        {
          errMsg << "Default " << (isInput ? "input" : "output");
        }
        else
        {
          errMsg << (isInput ? "Input" : "Output")
                 << " device named \"" << name << "\"";
        }
        PaHostApiInfo const * hostApiInfo = Pa_GetHostApiInfo(hostApiIndex);
        errMsg << " not found for host API \"" << hostApiInfo->name << "\".\n";
        if( deviceNames.empty() )
        {
          errMsg << " No admissible devices found.";
        }
        else
        {
          errMsg << " Admissible devices are: " << deviceNames;
        }
        throw std::invalid_argument( errMsg.str() );
      }
      PaDeviceInfo const * deviceInfo = Pa_GetDeviceInfo(deviceIdx);
      std::size_t const numChannels = isInput ? deviceInfo->maxInputChannels : deviceInfo->maxOutputChannels;
      if (numChannels < minChannels)
      {
        std::stringstream errMsg;
        errMsg << "PortAudioInterface: PortAudio ";
        if( useDefault )
        {
          errMsg << "default ";
        }
        errMsg << (isInput ? "input" : "output") << "device ";
        if( not useDefault )
        {
          errMsg << "\"" << name << "\" ";
        }
        errMsg << " has too few " << (isInput ? "input" : "output")
               << " channels (" << numChannels << ", required: " << minChannels << ").";
        throw std::runtime_error( errMsg.str() );
      }
      return deviceIdx;
    }

  } // unnamed namespace

  PortaudioInterface::Impl::Impl( Configuration const & baseConfig, std::string const & conf )
  : mNumCaptureChannels( baseConfig.numCaptureChannels() )
  , mNumPlaybackChannels( baseConfig.numPlaybackChannels() )
  , mPeriodSize( baseConfig.periodSize() )
  , mSampleRate( baseConfig.sampleRate() )
  , mStream( 0 )
  , mCallback( nullptr )
  , mCallbackUserData( nullptr )
  , mCommunicationBuffer( new CommunicationArea<SampleType>(mNumCaptureChannels + mNumPlaybackChannels, mPeriodSize, cVectorAlignmentSamples ) )
  , mCaptureSampleBuffers( mNumCaptureChannels, nullptr )
  , mPlaybackSampleBuffers( mNumPlaybackChannels, nullptr )
  {
    
    PortaudioInterface::Config config = parseSpecificConf(conf);
    mSampleFormat = config.mSampleFormat;
    mInterleaved = config.mInterleaved;
    mHostApiName = config.mHostApi;
    
    // Initialize the buffer pointer arrays.
    for( std::size_t captureIndex( 0 ); captureIndex < mNumCaptureChannels; ++captureIndex )
    {
      mCaptureSampleBuffers[captureIndex] = mCommunicationBuffer->at( captureIndex );
    }
    for( std::size_t playbackIndex( 0 ); playbackIndex < mNumPlaybackChannels; ++playbackIndex )
    {
      mPlaybackSampleBuffers[playbackIndex] = mCommunicationBuffer->at( mNumCaptureChannels + playbackIndex );
    }
    
    PaError ret;
    // Initialise the portaudio library. Multiple calls of this constructur should be no problem, since the number of initialize and deInitialize must match.
    if( (ret = Pa_Initialize()) != paNoError )
    {
      char const * msg = Pa_GetErrorText( ret );
      throw std::runtime_error( visr::detail::composeMessageString( "Initialisation of PortAudio library failed: ", msg, "." ) );
    }
    
    PaHostApiTypeId apiType;
    try
    {
      apiType = translateHostApiName( mHostApiName );
    }
    catch( std::exception const & e )
    {
      throw std::invalid_argument( std::string("Error while resolving host API type: ") + e.what() );
    }
    
    bool const useDefaultHostApi( apiType == paInDevelopment );
    
    PaHostApiIndex const apiCount = Pa_GetHostApiCount( );
    PaHostApiIndex const hostApiIdx = useDefaultHostApi ?
    Pa_GetDefaultHostApi() : Pa_HostApiTypeIdToHostApiIndex( apiType );
    if( hostApiIdx < 0 )
    {
      throw std::logic_error( std::string("PortAudioInterface: Error while retrieving the host API index: ")
                             + Pa_GetErrorText( hostApiIdx ) );
    }
    // Check against logical errors (internal errors of the portaudio library?)
    if( hostApiIdx >= apiCount )
    {
      throw std::logic_error( "PortAudioInterface: The returned API index exceeds the number of APIs" );
    }

    PaDeviceIndex const inDeviceIdx = findDevice(config.mInputDeviceName,
      hostApiIdx,
      mNumCaptureChannels,
      true /*isInput*/);
    PaDeviceIndex const outDeviceIdx = findDevice(config.mOutputDeviceName,
      hostApiIdx,
      mNumPlaybackChannels,
      false /*isInput*/ );

    PaSampleFormat const cPaSampleFormat = translateSampleFormat( mSampleFormat, mInterleaved );
    
    // Create input and output parameters
    PaStreamParameters inputParameters, outputParameters;
    inputParameters.device = inDeviceIdx;
    inputParameters.channelCount = static_cast<int>(mNumCaptureChannels);
    inputParameters.sampleFormat = cPaSampleFormat;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inDeviceIdx )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;
    
    outputParameters.device = outDeviceIdx;
    outputParameters.channelCount = static_cast<int>(mNumPlaybackChannels);
    outputParameters.sampleFormat = cPaSampleFormat;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outDeviceIdx )->defaultLowInputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;
    ret = Pa_IsFormatSupported( &inputParameters, &outputParameters,
                               static_cast<double>(mSampleRate) );
    if( ret != paFormatIsSupported )
    {
      throw std::invalid_argument( std::string("The chosen stream format is not supported by the portaudio interface: ") + Pa_GetErrorText( ret ) );
    }
    ret = Pa_OpenStream( &mStream,
                        &inputParameters,
                        &outputParameters,
                        static_cast<double>(mSampleRate),
                        static_cast<unsigned long>(mPeriodSize), // cast to avoid compiler warning
                        paNoFlag, // TODO: decide whether we want to optionally disable clipping or dithering
                        &PortaudioInterface::Impl::sEngineCallback,
                        this );
    if( ret != paNoError )
    {
      throw std::runtime_error( std::string("PortaudioInterface: Opening of audio stream failed: ")
                               + Pa_GetErrorText( ret ) );
    }
  }
  
  PortaudioInterface::Impl::~Impl()
  {
    PaError ret;
    // Shutdown the portaudio library.
    if( (ret = Pa_Terminate( )) != paNoError )
    {
      std::cerr << "Termination of PortAudio library failed." << std::endl;
    }
  }
  
  
  PortaudioInterface::Config PortaudioInterface::Impl::parseSpecificConf( std::string const & config )
  {
    std::stringstream stream(config.empty() ? "{}" : config ); // Cope with empty optional configs (the default)
    //            std::cout<< "STREAM: "<<stream<<std::endl;
    boost::property_tree::ptree tree;
    try
    {
      read_json( stream, tree );
    }
    catch( std::exception const & ex )
    {
      throw std::invalid_argument( std::string( "Error while parsing a json portaudio configuration string: ") + ex.what() );
    }

    std::string const sampleFormat = tree.get<std::string>("sampleformat", "float32Bit" );
    bool const interleaved = tree.get<bool>("interleaved", false );
    std::string const mHostApi = tree.get<std::string>("hostapi", "default" );

    Config ret(sampleFormat, interleaved, mHostApi);
    ret.mInputDeviceName = tree.get<std::string>("inputDevice", std::string() );
    ret.mOutputDeviceName = tree.get<std::string>("outputDevice", std::string() );
    return ret;
  }

  void PortaudioInterface::Impl::start()
  {
    // Todo: do we need an internal state representation and checking?
    PaError const ret = Pa_StartStream( mStream );
    if( ret != paNoError )
    {
      throw std::runtime_error( "PortaudioInterface: Opening of audio stream failed." );
    }
  }
  
  void PortaudioInterface::Impl::stop()
  {
    // Todo: do we need an internal state representation and checking?
    // Note: Consider whether PaAbortStream or Pa_CloseStream would make sense in some circumstances.
    PaError ret = Pa_StopStream( mStream );
    if( ret != paNoError )
    {
      throw std::runtime_error( "PortaudioInterface: Stopping of audio stream failed." );
    }
    ret = Pa_CloseStream( mStream );
    if( ret != paNoError )
    {
      throw std::runtime_error( "PortaudioInterface: Closing of audio stream failed." );
    }
  }
  
  bool PortaudioInterface::Impl::registerCallback( AudioCallback callback, void* userData )
  {
    mCallback = callback;
    mCallbackUserData = userData;
    return true;
  }
  
  bool PortaudioInterface::Impl::unregisterCallback( AudioCallback callback )
  {
    if( mCallback == callback )
    {
      mCallback = nullptr;
      mCallbackUserData = nullptr;
      return true;
    }
    else
    {
      return false;
    }
  }
  
  /*static*/ int
  PortaudioInterface::Impl::sEngineCallback( const void *input,
                                            void *output,
                                            unsigned long frameCount,
                                            const PaStreamCallbackTimeInfo *timeInfo,
                                            PaStreamCallbackFlags statusFlags,
                                            void *userData )
  {
    Impl* me = reinterpret_cast<Impl*>( userData );
    int const retValue = me->engineCallbackFunction( input, output, frameCount, timeInfo, statusFlags );
    return retValue;
  }
  
  int
  PortaudioInterface::Impl::engineCallbackFunction( void const *input,
                                                   void *output,
                                                   unsigned long frameCount,
                                                   const PaStreamCallbackTimeInfo *timeInfo,
                                                   PaStreamCallbackFlags statusFlags )
  {
    if( mCallback )
    {
      transferCaptureBuffers( input );
      bool res;
      try
      {
        (*mCallback)(mCallbackUserData, &mCaptureSampleBuffers[0], &mPlaybackSampleBuffers[0], res);
      }
      catch( std::exception const & e )
      {
        std::cerr << "Error during execution of audio callback: " << e.what() << std::endl;
        return paAbort;
      }
      transferPlaybackBuffers( output );
      // TODO: Replace by sophisticated return value depending on the status of the called functions.
      return paContinue;
    }
    else
    {
      return paContinue; // no registered callback function is no error. We should think about clearing the output buffers.
    }
  }
  
  void PortaudioInterface::Impl::transferPlaybackBuffers( void * output )
  {
    static_assert( std::is_same<SampleType, float >::value, "At the moment, only float is allowed as sample type." );
    if( (mSampleFormat != Config::SampleFormat::float32Bit ) /*or mInterleaved*/  )
    {
      throw std::invalid_argument( "At the moment, the portaudio interface supports only the sample type float32 in non-interleaved mode." );
    }
    for( std::size_t channelIndex( 0 ); channelIndex < mNumPlaybackChannels; ++channelIndex )
    {
      SampleType const * inputPtr = mPlaybackSampleBuffers[channelIndex];
      const std::size_t outStride = (mInterleaved ? mNumPlaybackChannels : 1);
      // Note: depending in the 'interleaved' mode the portaudio buffer variables are either arrays of samples or
      // pointer arrays to sample vectors.
      float * outputPtr = mInterleaved ?
      reinterpret_cast<float *>(output) + channelIndex :
      reinterpret_cast<float * const * >(output)[channelIndex];
      for( std::size_t sampleIdx( 0 ); sampleIdx < mPeriodSize; ++sampleIdx, ++inputPtr )
      {
        *outputPtr = *inputPtr;
        outputPtr += outStride;
      }
    }
  }
  
  void PortaudioInterface::Impl::transferCaptureBuffers( void const * input )
  {
    static_assert(std::is_same<SampleType, float >::value, "At the moment, only float is allowed as sample type.");
    if( (mSampleFormat != Config::SampleFormat::float32Bit) /*or mInterleaved*/ )
    {
      throw std::invalid_argument( "At the moment, the portaudio interface supports only the sample type float32 in un-interleaved mode." );
    }
    for( std::size_t channelIndex( 0 ); channelIndex < mNumCaptureChannels; ++channelIndex )
    {
      SampleType * outputPtr = mCaptureSampleBuffers[channelIndex];
      const std::size_t inStride = (mInterleaved ? mNumCaptureChannels : 1);
      // Note: depending in the 'interleaved' mode the portaudio buffer variables are either arrays of samples or
      // pointer arrays to sample vectors.
      float const * inputPtr = mInterleaved ?
      reinterpret_cast<float const *>(input) + channelIndex :
      reinterpret_cast<float const * const * >(input)[channelIndex];
      for( std::size_t sampleIdx( 0 ); sampleIdx < mPeriodSize; ++sampleIdx, ++outputPtr )
      {
        *outputPtr = *inputPtr;
        inputPtr += inStride;
      }
    }
  }
  
  /******************************************************************************/
  /* PortAudioInterface implementation                                          */
  
  PortaudioInterface::PortaudioInterface( Configuration const & baseConfig, std::string const & config )
  : mImpl( new Impl( baseConfig, config ) )
  {
  }

  PortaudioInterface::~PortaudioInterface( )
  {
    // nothing to be done, as all cleanup is performed in the implementation object.
  }

  void PortaudioInterface::start()
  {
    mImpl->start();
  }

  void PortaudioInterface::stop()
  {
    mImpl->stop();
  }

  /*virtual*/ bool
  PortaudioInterface::registerCallback( AudioCallback callback, void* userData )
  {
    return mImpl->registerCallback( callback, userData );
  }
  
  /*virtual*/ bool
  PortaudioInterface::unregisterCallback( AudioCallback callback )
  {
    return mImpl->unregisterCallback( callback );
  }

  std::size_t PortaudioInterface::numberOfCaptureChannels() const
  {
    return mImpl->mNumCaptureChannels;
  }

  std::size_t PortaudioInterface::numberOfPlaybackChannels() const
  {
    return mImpl->mNumPlaybackChannels;
  }

  std::size_t PortaudioInterface::period() const
  {
    return mImpl->mPeriodSize;
  }

  std::size_t PortaudioInterface::samplingFrequency() const
  {
    return mImpl->mSampleRate;
  }

  PortaudioInterface::Config::Config( std::string sampleFormat, bool interleaved, std::string mHostApi)
  : mInterleaved(interleaved)
  , mHostApi(mHostApi)
  {
    
    mSampleFormat = translateToSampleFormat(sampleFormat);
  }

} // namespace rrl
} // namespace visr
