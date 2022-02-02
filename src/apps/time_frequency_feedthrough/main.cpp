/* Copyright Institute of Sound and Vibration Research - All rights reserved */

// Enable native JACK interface instead of PortAudio
// TODO: Make this selectable via a command line option.
// #define NATIVE_JACK

#include <libefl/denormalised_number_handling.hpp>
#include <libaudiointerfaces/audio_interface_factory.hpp>

#ifdef NATIVE_JACK
#include <libaudiointerfaces/jack_interface.hpp>
#else
#include <libaudiointerfaces/portaudio_interface.hpp>
#endif
#include <librrl/audio_signal_flow.hpp>
#include <libaudiointerfaces/audio_interface.hpp>

#include <libsignalflows/time_frequency_feedthrough.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstdio> // for getc(), for testing purposes
#include <iostream>
#include <sstream>

int main( int argc, char const * const * argv )
{
    using namespace visr;
    
    try
    {
        efl::DenormalisedNumbers::State const oldDenormNumbersState
        = efl::DenormalisedNumbers::setDenormHandling();
        
        std::size_t const numberOfChannels = 2;
        std::size_t const period = 256;
        std::size_t const windowLength = 2 * period;
        std::size_t const hopSize = period;
        std::size_t const dftSize = 2 * windowLength;
        
        SamplingFrequencyType const samplingRate = 48000;
        visr::audiointerfaces::AudioInterface::Configuration baseConfig(numberOfChannels,numberOfChannels,samplingRate,period);
        
        std::string type;
        std::string specConf;
        
#ifdef NATIVE_JACK
        std::string pconfig = "{\"ports\":[ \n { \"captbasename\": \"input_\"}, \n { \"playbasename\": \"output_\"} ]}";
        specConf = "{\"clientname\": \"BaselineRenderer\", \"servername\": \"\", \"portsconfig\" : "+pconfig+"}";
        
        type = "Jack";
#else
        specConf = "{\"sampleformat\": 8, \"interleaved\": \"false\", \"hostapi\" : \"default\"}";
        type = "PortAudio";
#endif
        
        std::unique_ptr<audiointerfaces::AudioInterface> audioInterface
          = audiointerfaces::AudioInterfaceFactory::create( type, baseConfig, specConf);
        
        
        SignalFlowContext context( period, samplingRate );
        
        signalflows::TimeFrequencyFeedthrough flow( context,
                                                   "", nullptr,
                                                   numberOfChannels,
                                                   dftSize,
                                                   windowLength,
                                                   hopSize );
        
        rrl::AudioSignalFlow audioFlow( flow );
        
        audioInterface->registerCallback( &rrl::AudioSignalFlow::processFunction, &audioFlow );
        
        // should there be a separate start() method for the audio interface?
        audioInterface->start( );
        
        // Rendering runs until q<Return> is entered on the console.
        std::cout << "VISR time-frequency feedthrough renderer running. Press \"q<Return>\" or Ctrl-C to quit." << std::endl;
        char c;
        do
        {
            c = std::getc( stdin );
        }
        while( c != 'q' );
        
        audioInterface->stop( );
        
        audioInterface->unregisterCallback( &rrl::AudioSignalFlow::processFunction );
        
        efl::DenormalisedNumbers::resetDenormHandling( oldDenormNumbersState );
    }
    catch( std::exception const & ex )
    {
        std::cout << "Exception caught on top level: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
