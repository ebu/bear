/* Copyright Institute of Sound and Vibration Research - All rights reserved */

#include "udp_sender.hpp"

#include <libpml/empty_parameter_config.hpp>

#include <boost/array.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/bind/bind.hpp>
#ifndef VISR_DISABLE_THREADS
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>
#endif

#include <ciso646>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace visr
{
namespace rcl
{

class UdpSender::Impl
{
public:
  explicit Impl( UdpSender & parent,
                 std::size_t sendPort,
                 std::string const & receiverAddress,
                 std::size_t receiverPort,
                 UdpSender::Mode mode);

  ~Impl();

  void process(UdpSender::MessageInput & messageInput );
private:
    /**
    * Callback function triggered by the ASIO library.
    * @throw std::runtime_error if \p error is set or the number of transferred bytes differs from the expected value.
    */
    void handleSentData(const boost::system::error_code& error,
        std::size_t numBytesTransferred);

    Mode mMode;

    /**
    * Pointer to the either internally or externally provided externally provided boost::asio::io_service object.
    */
    boost::asio::io_service* mIoService;

    /**
    * An actual io_service object owned by this component, which is allocated in the modes Synchronous or Asynchronous,
    * but not for ExternalServiceObject.
    */
    std::unique_ptr<boost::asio::io_service> mIoServiceInstance;

    std::unique_ptr<boost::asio::ip::udp::socket> mSocket;

    boost::asio::ip::udp::endpoint mRemoteEndpoint;

    std::unique_ptr<boost::asio::io_service::work> mIoServiceWork;

    /**
    * Internal queue of messages received asynchronously. They will be copied into the output
    *  MessageQueue in the process() function. An object is instantiated only in the asynchronous mode.
    */
    std::deque< pml::StringParameter > mInternalMessageBuffer;

#ifndef VISR_DISABLE_THREADS
    std::unique_ptr< boost::thread > mServiceThread;

    boost::mutex mMutex;
#endif
};

UdpSender::UdpSender( SignalFlowContext const & context,
                      char const * name,
                      CompositeComponent * parent,
                      std::size_t sendPort,
                      std::string const & receiverAddress,
                      std::size_t receiverPort,
                      UdpSender::Mode mode )
 : AtomicComponent( context, name, parent )
 , mImpl( new Impl( *this, sendPort, receiverAddress, receiverPort, mode ) )
 , mMessageInput( "messageInput", *this, pml::EmptyParameterConfig() )
{
}

void UdpSender::process()
{
  mImpl->process(mMessageInput);
}

UdpSender::~UdpSender() = default;

UdpSender::Impl::Impl(UdpSender & parent,
    std::size_t sendPort,
    std::string const & receiverAddress,
    std::size_t receiverPort,
    UdpSender::Mode mode)
{
    using boost::asio::ip::udp;
    mMode = mode;
    mIoServiceInstance.reset(new boost::asio::io_service());
    mIoService = mIoServiceInstance.get();

    if( mMode == Mode::Synchronous )
    {
      mIoServiceWork.reset();
    }
    else
    {
      mIoServiceWork.reset(new  boost::asio::io_service::work(*mIoService));
    }

    udp::resolver resolver(*mIoService);
    udp::resolver::query query(udp::v4(),
        receiverAddress,
        std::to_string(receiverPort),
        udp::resolver::query::flags::passive); /* Override the default values for the flag parameter which includes "address_configured"
                                               that requires a network conection apart from loopback device. */
    mRemoteEndpoint = *resolver.resolve(query);
    // Debug output:
    // std::cout << "Remote endpoint: " << mRemoteEndpoint.address().to_string() << ":" << mRemoteEndpoint.port() << std::endl;
    if (mRemoteEndpoint.port() != static_cast<unsigned short>(receiverPort))
    {
        throw std::logic_error("");
    }

    udp::endpoint localEndpoint(udp::v4(), static_cast<unsigned short>(sendPort));
    mSocket.reset(new udp::socket(*mIoService, localEndpoint));

#ifdef VISR_DISABLE_THREADS
    throw std::invalid_argument( "UdpSender: Asynchronous mode is not supported because threads are disabled." );
#else
    if (mMode == Mode::Asynchronous)
    {
        mServiceThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, mIoService)));
    }
#endif
}


UdpSender::Impl::~Impl()
{
  if( mIoServiceInstance.get() != nullptr )
  {
    mIoServiceInstance->stop();
  }
#ifndef VISR_DISABLE_THREADS
  if( mServiceThread.get() != nullptr  )
  {
    mServiceThread->join();
  }
#endif
}

void UdpSender::Impl::process( UdpSender::MessageInput & messageInput )
{
  switch( mMode )
  {
  case Mode::Synchronous:
    // we don't operate an internal message queue in this mode.
    while( not messageInput.empty() )
    {
      pml::StringParameter const & nextMsg = messageInput.front( );
      // boost::system::error_code err;
      std::size_t bytesSent = mSocket->send_to( boost::asio::buffer(nextMsg.str(), nextMsg.size()), mRemoteEndpoint );
      if( bytesSent != nextMsg.size() )
      {
        throw std::runtime_error( "Number of sent bytes differs from message size." );
      }
      messageInput.pop( );
    }
    break;
  case Mode::Asynchronous:
#ifdef VISR_DISABLE_THREADS
    // Shouldn't happen because this is already checked in the constructor.
    throw std::logic_error( "UdpSender: Logic error: Asynchronous mode not supported because threads are disables." );
#else
    {
      boost::lock_guard<boost::mutex> lock( mMutex );
      bool const transmissionPending = not mInternalMessageBuffer.empty();
      while( not messageInput.empty() )
      {
        char const * nextMsg = messageInput.front().str();
        mInternalMessageBuffer.push_back( pml::StringParameter( nextMsg ) );
        messageInput.pop();
      }
      if( not mInternalMessageBuffer.empty() and not transmissionPending )
      {
        pml::StringParameter const & nextMsg = mInternalMessageBuffer.front( );
        mSocket->async_send_to( boost::asio::buffer( nextMsg.str(), nextMsg.size( ) ),
                                mRemoteEndpoint,
                                boost::bind( &UdpSender::Impl::handleSentData, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred ) );
      }
    }
#endif
  }
}

void UdpSender::Impl::handleSentData(const boost::system::error_code& error,
    std::size_t numBytesTransferred)
{
    if (error != boost::system::errc::success)
    {
        throw std::runtime_error("UdpSender: Asynchronous send operation resulted in an error.");
    }
#ifndef VISR_DISABLE_THREADS
    boost::lock_guard<boost::mutex> lock(mMutex);
#endif
    assert(not mInternalMessageBuffer.empty());
    pml::StringParameter & currMsg = mInternalMessageBuffer.front();
    if (currMsg.size() != numBytesTransferred)
    {
        throw std::runtime_error("UdpSender: Asynchronous send operation transmitted less bytes than the message length.");
    }
    mInternalMessageBuffer.pop_front();
    if (not mInternalMessageBuffer.empty())
    {
        pml::StringParameter const & nextMsg = mInternalMessageBuffer.front();
        mSocket->async_send_to(boost::asio::buffer(nextMsg.str(), nextMsg.size()),
            mRemoteEndpoint,
            boost::bind(&UdpSender::Impl::handleSentData, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
}

} // namespace rcl
} // namespace visr
