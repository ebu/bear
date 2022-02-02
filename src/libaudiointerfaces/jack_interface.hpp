/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#ifndef VISR_LIBRRL_JACK_INTERFACE_HPP_INCLUDED
#define VISR_LIBRRL_JACK_INTERFACE_HPP_INCLUDED

#include "audio_interface.hpp"
#include "export_symbols.hpp"

#include <libvisr/constants.hpp>

#include <boost/property_tree/ptree.hpp>

#include <memory>
#include <string>
#include <vector>

namespace visr
{
namespace audiointerfaces
{
  class VISR_AUDIOINTERFACES_LIBRARY_SYMBOL  JackInterface: public audiointerfaces::AudioInterface
  {
  public:
    /**
     * Structure to hold all configuration arguments for a PortAudioInterface instance.
     */
    struct Config
    {
    public:

      Config(std::string cliName, std::string servName, boost::property_tree::ptree portsConfig,  bool autoConnect = false);

      void loadPortConfig(boost::optional<boost::property_tree::ptree> tree,
                          std::vector< std::string > &portNames,
                          std::vector< std::string > & extPortNames,
                          std::size_t numPorts,
                          bool autoConn,
                          std::string const & portType );
      std::string mClientName;
      std::string mInExtClientName;
      std::string mOutExtClientName;
      bool mInAutoConnect;
      bool mOutAutoConnect;
      
      std::string mServerName;
      
      boost::property_tree::ptree mPortJSONConfig;
      
      std::vector< std::string > mCapturePortNames;
      
      std::vector< std::string > mPlaybackPortNames;

      /**
       * Whether to display verbose messages when connecting ports.
       */
      bool mVerbosePortConnections;

    };
    
    using Base = AudioInterface;
    
    explicit JackInterface(  Configuration const & baseConfig, std::string const & config);
    
    virtual ~JackInterface() override;
    
    /* virtual */ void start() override;
    
    /* virtual */ void stop() override;
    
    /*virtual*/ bool registerCallback( AudioCallback callback, void* userData ) override;
    
    /*virtual*/ bool unregisterCallback( AudioCallback audioCallback ) override;

    /**
     * Return the number of input channels to the interface.
    */
    /*virtual*/ std::size_t numberOfCaptureChannels() const override;

    /**
     * Return the number of output channels to the interface.
     */
    /*virtual*/ std::size_t numberOfPlaybackChannels() const override;

    /**
     * Return the configured period (block size).
     */
    /*virtual*/ std::size_t period() const override;

    /**
     * Return the configured sampling frequency (in Hz)
     */
    /*virtual*/ std::size_t samplingFrequency() const override;
    
  private:
    /**
     * Private implementation class to avoid dependencies to the Portaudio library in the public interface.
     */
    class Impl;
    
    
    //            JackInterface::Config parseSpecificConf( std::string const & config );
    
    /**
     * Private implementation object according to the "pointer to implementation" (pimpl) idiom.
     */
    std::unique_ptr<Impl> mImpl;
  };
  
} // namespace rrl
} // namespace visr

#endif // #ifndef VISR_LIBRRL_JACK_INTERFACE_HPP_INCLUDED
