.. Copyright Andreas Franck 2018 - All rights reserved.
.. Copyright University of Southampton 2018 - All rights reserved.

.. _visr_component_api_parameter_subsystem:

The parameter subsystem
-----------------------

In VISR, *parameters* denote data that being is interchanged between components
and/or the exterior of a signal flow that is not audio.
As described in the tutorial :ref:`visr_tutorial_subsec_visr_framework_ports`,
parameter data is much more diverse than audio data.
For that reason, two main design principles in VISR are:

* The parameter system is extensible, that means that new types can be added to the framework
  without changing the core framework. In particular, parameter types can be defined in external libraries.
* Parameter communication is type-safe. In C++, most parameter ports are configured to accept a specific
  parameter type, and the API for these ports are tailored to access and accept this specific type.
  Morover, it is possible to specify so-called *parameter configurations* that define certain characteristics
  of the parameter data exchanged by a parameter port. For example, ports accepting matrices are configured
  with a :code:`pml::MatrixParameterConfiguration` object, which causes the port to accept only matrices of a given dimension.

Implementing parameter types in C++
"""""""""""""""""""""""""""""""""""

Implementing a parameter typoe in C++ requires three distinct steps:

* Implementing the parameter class.
* Making the type known to the compiler such that it can be used within parameter ports in a typesafe way.
* Registering parameter types in the runtime system.

These steps are described in detail below.

The type :code:`visr::ParameterType`
''''''''''''''''''''''''''''''''''''

Parameter types must be unique within the VISR framework. For this reason they
are identified by a unique identifier of type :code:`visr::ParameterType`.
This is essentially an integer value, and the implementers are required to ensure that they are unique.

We suggest to use the function :code:`visr::detail::compileTimeHashFNV1`, which creates an (almost
certainly) unique identifier from the class name.

.. doxygenfunction:: visr::detail::compileTimeHashFNV1
   :project: visr

Note that this this is a C++ :code:`constexpr` function, which means that it can also be used at compile time,
for example for use in template parameters.


The base class  :code:`visr::ParameterBase`
'''''''''''''''''''''''''''''''''''''''''''

All parameter types must derive from the base class :code:`visr::ParameterBase`.
This is an abstract base class, and derived classes must implement the virtual interface.

.. doxygenclass:: visr::ParameterBase
   :project: visr
   :members:

That is, implementers of a parameter type must override:

* A  method :code:`type()` returning the parameter type identifier of the object.
* A :code:`clone()` method, which returns a copy of the object itself.
  This must be a complete (deep) copy, as parameter objects will be copied
  and transferred by the runtime system during rendering.
* An :code:`assign( visr::ParameterBase const & rhs )` method that accepts a
  :code:`visr::ParameterBase` object and fills the present content with its
  content, i.e., a virtual assignment operator. The class is responsible for
  checking whether the type of :code:`rhs` matches its parameter type and the
  configuration (e.g., matrix dimension).

Moreover, class implementers need to define a constructor that takes a
:code:`const ParameterConfigBase &` reference and creates an object according to
this parameter specification. Typically, this will require a downcast to the specific
config type supported by this parameter class, and will cause an exception when invoked
with a non-matching config type.

In addition, parameter classes will need to implement an API to access and set
their data content.

The helper template :code:`visr::TypedParameterBase`
''''''''''''''''''''''''''''''''''''''''''''''''''''

This template is provided to ease the implementation of new parameter types.
It implements the curiously recurring template pattern
`CRTP <https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern>`_
to automatically generate most of the boilerplate code needed when a parameter type is
created by deriving from :code:`visr::ParameterBase`.

.. doxygenclass:: visr::TypedParameterBase
   :project: visr

The :code:`visr::TypedParameterBase` implements the :code:`type()` and
:code:`clone()`, and :code:`assign()` methods. The latter is implemented through a downcast (which throws
an exception if the type is not compatible), and invoking the (explicit or implicit) assignment operator.

In addition, it provides a static (class) method :code:`staticType`, which returns the unique type id
of the parameter type.
This is the same id that would returned when the (virtual) method :code:`clone()` is invoked for an
instance of this parameter type.

Thus, a minimal example for a new parameter might look like this:

.. code:: c++

   class MyParameter: public TypedParameterBase< MyParameter, SomeParameterConfig,
      detail::compileTimeHashFNV1("MyParameterName") >
   {
   public:
     MyParameter( SomeParameterConfig const & config );

     MyParameter( ParameterConfigBase const & config )
      : MyParameter( dynamic_cast<SomeParameterConfig const &>( config )

     // parameter-specific implementation
   };

The second constructor :code:`MyParameter( ParameterConfigBase const & config )`
is a technical necessity (there is no straightforward way to do that in the
:code:`TypedParameterBase` class.)

Declaring parameter types for use in parameter ports
''''''''''''''''''''''''''''''''''''''''''''''''''''

In order to create type-safe parameter ports, parameter types must be made
known to C++.
This is done through the macro :code:`DEFINE_PARAMETER_TYPE`, defined in
:code:`libvisr/parameter_base.hpp`.

.. code:: c++

   #define DEFINE_PARAMETER_TYPE( ParameterClassType, ParameterId, ParameterConfigType)

A typical use looks like:

.. code:: c++

   DEFINE_PARAMETER_TYPE( MyParameter, MyParameter::staticType(), SomeParameterConfig )

which uses the :code:`staticType` facility of :code:`TypedParameterBase`.
This definition needs to be placed at global scope (outside any namespaces) and must be visible
at the point of definition of :code:`ParameterInput` and :code:`ParameterOutput` ports using our parameter type.


Registering parameter types
'''''''''''''''''''''''''''

Parameter types must be registered in the runtime system before they
can be used for rendering.
This is done through static template methods of :code:`visr::ParameterFactory`:

.. doxygenfunction:: visr::ParameterFactory::registerParameterType(ParameterType const&)
   :project: visr

.. doxygenfunction:: visr::ParameterFactory::registerParameterType()
   :project: visr

The first overload accepts an arbitrary parameter type (derived from :code:`visr::ParameterBase`) and a type id :code:`type`.
The latter overload works only with parameter types based on the helper template :code:`TypedParameterBase`.

.. note:: In the future, a third, non-templated of :code:`visr::ParameterFactory::registerParameterType()`
   will be provided, which accepts a function object parameter that creates
   parameter type objects based on a parameter configuration object.
   However, this is not intended to be used by standard users, but for
   specialised uses such as implementing the Python bindings for the parameter
   subsystem.

:code:`visr::ParameterFactory::registerParameterType` can be called multiple times for the same type id
without error, and the latest call replaces previous registrations.

The :code:`visr::ParameterFactory` class also provides fuctions for unregistering of parameter types.

.. doxygenfunction:: visr::ParameterFactory::unregisterParameterType(ParameterType const&)
   :project: visr

.. doxygenfunction:: visr::ParameterFactory::unregisterParameterType()
   :project: visr

The latter overload is to be used with parameter types derived from :code:`TypedParameterBase`,
because it deduces the type id from that template.

Note that unregistering a type works regardless how often the corresponding
:code:`visr::ParameterFactory::registerParameterType()` function has been called.
That is, no reference counting is taking place.

.. note:: Consider whether such a reference counting mechanism, where the number of unregister
   calls has to match the number of register calls, would be useful for general applications.

In most standard applications, unregistering of types is not important, because it is automatically performed when the application terminates.

The template class :code:`visr::ParameterRegistrar` is provided to simplify the registration/ unregistering of parameter types.
It is to be used with types derived from :code:`TypedParameterBase`, because it uses the information encoded into these templates.

To register a class, instanstiate a variable of the :code:`visr::ParameterRegistrar` template parameterised by the type of this class.

.. code:: c++

   ParameterRegistrar< ParameterClass1 > myVariable;

Typically, the registrar valiable is created as a static variable within a library or an application,
or as a static local variable within an initialisation function (see :code:`visr::pml::initialiseParameterLibrary()`).
This ensures that the parameters are unregistered if the respective library is unloaded or the program terminates.

.. note:: Be aware that the initialisation of static instances works differently (i.e., it does not happen) when the
   instance is contained in a static library.

To reduce the amount of code (and required variables), multiple parameter types
can be registered with a single :code:`visr::ParameterRegistrar` instance. This
works because :code:`visr::ParameterRegistrar` is a variadic template.
The syntax is:

.. code:: c++

   ParameterRegistrar< ParameterClass1, ..., ParameterClassN > myVariable;

Extending the parameter subsystem in Python
"""""""""""""""""""""""""""""""""""""""""""

Creating and using the new parameters Python works essentially in the same
way as in C++, except for the obvious differences between the two languages.
* The :code:`DEFINE_PARAMETER_TYPE` step is not necessary, because Python is not a compiled
language with strong, static typing and/or templates.
* Python parameter types can not be used (more precisely: they could be used only in an
opaque way, e.g.) in C++ components. This is because it would require a C++ class definition
(e.g., a header file) to do so.

Creating new parameter types
''''''''''''''''''''''''''''

New parameter types are created by implementing a Python class that inherits from
:code:`visr.ParameterBase`.

This is a simple example (taken from :code:`src/python/visr/test/testclasses/my_parameter1.py`)

.. code:: python

   import visr
   import pml

   class MyParameter1(visr.ParameterBase):
       staticType = visr.ParameterBase.createTypeId("MyParameter1")
       def __init__( self, val ):
           super(MyParameter1, self).__init__()
           self.val = val
       def value(self):
           return self.val
       def set( self, val ):
           self.val = val
       def type( self ):
           return MyParameter1.staticType
       def clone( self ):
           return MyParameter1( self.value() )
       def assign( self, rhs ):
           self.val = rhs.value()
       def construct( paramConfig ): # Static create function
           if not isinstance( paramConfig, pml.VectorParameterConfig):
               raise ValueError( "MyParam expects a VectorParameterConfig object!" )
           return MyParameter1( np.ones(paramConfig.numberOfElements ) )

The main aspects are:
* :code:`MyParameter1` inherits from :code:`visr.ParameterBase`.
* The class attribute :code:`staticType` stores the unique identifier. Using this name is
not necessary but a convention.
However, following this convention makes types consistent with the builtin types and
enables the use of a simplified API (for example for registering parameters.
:code:`staticType` is essentially an integer that must be unique across all parameter
types used by the VISR runtime at the same time.
As in C++ we suggest to create a this number using a hash function. For this reason
we provide the convenience function :code:`visr.ParameterBase.createTypeId()`,
which calls the :code:`visr::detail::compileTimeHashFNV1` function.
* The methods :code:`value()` and :code:`set()` are the type-specific methods
access the contents of the parameter.
* The methods :code:`type()`, :code:`clone()`, and :code:`assign()` are pure virtual methods of :code:`ParameterBase` and must be
implemented by any derived class.
* The class method :code:`construct()` is required by the runtime system. Using this name and implementing
it as a static class method is just a convention.
It enables, however, the use of a simplified API, for example for registering parameter types.


Registering parameter types
'''''''''''''''''''''''''''

As in C++, parameter types must be registered with the parameter subsystem in order to use them for rendering.
This is achieved through static methods (class methods) of :code:`visr.ParameterFactory`.

.. code:: python

   import visr

   visr.ParameterFactory.registerParameterType(MyParameter1.staticType,
                                               MyParameter1 )
   # Alternative syntax, assumes that the parameter type id is stored in a
   # class attribute "staticType"
   visr.ParameterFactory.registerParameterType(MyParameter1)

Both methods require that the class (:code:`MyParameter1` in the example) provides
a static (class) method :code:`construct` that takes a parameter config object of
the correct type and returns a newly created parameter object.

.. note:: Consider creating an additional variant of :code:`registerParameter()`
   that accepts a free function to construct a new parameter object.

Parameter types can be removed from the runtime system through the corresponding
unregister functions.

.. code:: python

   import visr

   visr.ParameterFactory.unregisterParameterType(MyParameter1.staticType )
   # Alternative syntax, assumes that the parameter type id is stored in a
   # class attribute "staticType"
   visr.ParameterFactory.unregisterParameterType(MyParameter1)

While the first variant accepts a parameter type id, the second variant takes a
Python class object and infers the type id from the :code:`staticType` attribute.

To simplify the registration process, :code:`visr.ParameterRegistrar` variables can
be used that resemble the :code:`ParameterRegistrar` templates in the C++ API.

.. code:: python

   import visr

   myParameterRegistrar = visr.ParameterRegistrar( MyParameter1.staticType )

   # Variant that accepts a class object and infers the type id from itself
   # "staticType" attribute.
   myParameterRegistrar = visr.ParameterRegistrar( MyParameter1 )

If the variable gets garbage collected because it is deleted or goes out of scope,
the corresponding parameter type will be unregistered. However, due to the
unspecified timing of Python's garbage this might not be reliable or, at the very
least, portable across different implementations of Python (for example, cpython
vs. PyPy).

Typically, :code:`ParameterRegistrar` instances would be created

.. todo:: Provide example for using ParameterRegistrar in module initialisation
   code.

Using user-defined parameters
'''''''''''''''''''''''''''''

User-defined parameters can be used in the same way as built-in VISR parameter.

This is an example of a simple component (taken from
:code:`$VISR/src/python/visr/test/testclasses/my_parameter_receiver.py`)

.. code:: python

   import numpy as np

   import visr
   import pml

   from .my_parameter1 import MyParameter1

   class MyParameterReceiver( visr.AtomicComponent ):
       """ Sender for MyParameter1 parameter data  """
       def __init__( self, context, name, parent, width ):
           super(MyParameterReceiver, self).__init__( context, name, parent )
           self.counter = 0
           self.width = width
           self.input = visr.ParameterInput( "paramIn", self,
             MyParameter1.staticType,
             pml.DoubleBufferingProtocol.staticType,
             pml.VectorParameterConfig(width) )
       def process( self ):
           if self.input.protocol.changed():
               msg = self.input.protocol.data().value()
               print( "MyParameterReceiver: Received message \"\"."
                     % str(msg) )
               self.input.protocol.resetChanged()
           ++self.counter

That is, parameter types defined in Python are used exactly as built-in or
user-defined C++ types. The type definition must be visible to the definition of
the component (which is done through importing the type in the example.)

Creating custom parameter types in Python
'''''''''''''''''''''''''''''''''''''''''

.. todo:: Provide an example for a custom config type (and ink to the
   corresponding unit test file).
