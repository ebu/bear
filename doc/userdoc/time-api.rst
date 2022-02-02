.. Copyright Andreas Franck 2018 - All rights reserved.
.. Copyright University of Southampton 2018 - All rights reserved.

.. _visr_component_api_time_api:

Time in VISR and time component API
-----------------------------------

* The VISR framework is primarily a processing framework for blockwise signal processing
  governed by an audio interface.
  That means that the execution is governed by the sample clock of the audio.
  This forms a time reference, which is referred to as the :code:`sample_clock` in the following.

* Processing is performed in *iterations*, where each iteration processes a fixed number of :code:`periodSize` samples.




Timing guarantuees in VISR
""""""""""""""""""""""""""
The VISR API is defined independent of an actual implementation.
That is, it does not take into account whether it is exexuted in a sequential
single-processor runtime environment or in a parallel execution environment,
potentially with distributed memory.

The following guarantees are made:

* Audio signals processed in one iteration are received in the same
  iteration by any component that receives these audio signals.
  This assumes that components follow a DAG (directed acyclic graph) ordering.
  That mean that delay-free loops of audio signals are not permitted.

  An API to specify delays in audio signal components or signal paths is in
  preparation. This would break delay-free cycles (under certain conditions) .

* Parameter messages are transmitted the receiver within the same iteration
  of the signal flow. This implies that messages sent to multiple receivers are
  received in the same iteration. As in case of the audio signals, this implies
  that there are no cycles of components with (delay-free) audio and parameter
  connections.

  An API to specify a delayed transmission for a parameter connection will be
  added to the VISR component API. Note that this delay will be in addition to
  a delayed delivery time specified for a parameter transmission (see below).

  Note: Parameter messages are always delivered at the beginning of a
  :code:`process()` call. That is, delays for sending a parameter message
  will always be multiples of the period size.
  However, it is 

Use cases
"""""""""

This is a collection of cases that use timing information within rendering signal flows. 

*Using elapsed time in a component*
  The behaviour of a component can depend on on the elapsed time, for example:

  - The output (audio) signal depends on the elapsed time.
    
  - An action is performed a certain amount of time after an initial event.

  This can be generally accomplished by implementing a cycle counter within a
  component, which is updated and checked in each :code:`process()` call.
  Using a centralised time provider can result in conciser code.

*Communicating time information between components*
  Components might need to signal time information to other components, e.g., to
  trigger actions at a given time instant. This is not possible without a time
  API (unless implicit assumptions are made, such as block counters in different
  components will have the same value).

*Passing parameter data with with a arrival time stamps*
  By default, transmission of parameter data is immediate. Attaching delivery
  time information to parameter messages would mean that the receipt could be
  synchronised to the internal time reference. For example, this would guarantee that
  a parameter message sent to multiple receivers is received at the 
  It might also result in clearer component implementations, because it avoids the
  logic to either store a message until the intended send time, or to keep a received
  message until the intended arrival time.

*Rendering of content with an internal time representation*
  Example: ADM files.

  - The resolution of the time represention might be higher than the period size,
    or even the duration of a sample.
  
*Aligning rendering to an external time reference*
  For example, use external time code information to control file-based playback
  of ADM contents.

  - This requires a non-monotonic clock,  which changes discontinuously and also 'back in time'.


The :code:`visr::Time` API
""""""""""""""""""""""""""

The class :code:`visr::Time`, which is accessed through the method :code:`Component::time()`.

.. doxygenclass:: visr::Time
   :project: visr
   :members:

.. doxygenfunction:: visr::Component::time
   :project: visr

Parameter communication with timing information
"""""""""""""""""""""""""""""""""""""""""""""""

Timing information can be used to transmit parameter messages for with a prescribed arrival time.
This is different from the default behaviour, which assumes instantaneous delivery.

Support for timing information is specific to the API of each communication protocol.
Not all communication protocols support timing information, either because it is not
implemented or because the use of timing information is not sensible for this communication semantics.
For example, :code:`pml::SharedMemoryProtocol` models direct, immediate access to a shared memory resource.
This is incompatible with a delayed transmission using receive time stamp.

Protocol :code:`pml::DoubleBufferingProtocol`
'''''''''''''''''''''''''''''''''''''''''''''
   
