Notecard Auxiliary Wi-Fi
========================

NotecardAuxiliaryWiFi is a utility class to make external Wi-Fi functionality
available to the [Blues Wireless Notecard](https://shop.blues.io/collections/notecard).
The Notecard can then use the Wi-Fi radio to enable such functionality as Wi-Fi
triangulation.

Example sketches in can be found in the [examples directory](examples).

This library takes a dependency on the [note-arduino library][note-arduino],
which provides access to the `Notecard` class. The Notecard is used to submit
data to Notehub where the values are transformed into location information.

Installation
------------

1. Open the Arduino IDE and click **Tools > Manage Libraries...**.
2. Search for "Blues" in the input box and click the "Install" button next
   to the "Blues Wireless Notecard Auxiliary Wi-Fi" result.
3. Create a new sketch and select the **Sketch > Include Library > Contributed
Libraries > Blues Wireless Notecard Auxiliary Wi-Fi** menu option, to add the
following includes to your sketch:

```cpp
#include <Notecard.h>
#include <NotecardAuxiliaryWiFi.h>
```

Usage
-----

```cpp
#include <Notecard.h>
#include <NotecardAuxiliaryWiFi.h>

using namespace blues;

// Create an instance of the Notecard and NotecardAuxiliaryWiFi
Notecard notecard;
NotecardAuxiliaryWiFi aux_wifi(notecard);

notecard.begin();
aux_wifi.begin();

aux_wifi.updateTriangulationData();
```

Examples
--------

The [examples](examples/) directory contains examples for using this library
with:

- [Triangulate](examples/Triangulate/Triangulate.ino)

Contributing
------------

We love issues, fixes, and pull requests from everyone. Please run the
unit-tests ([as described in the following section](#tests)), prior
to submitting your PR. By participating in this project, you agree to abide by
the Blues Inc [code of conduct].

For details on contributions we accept and the process for contributing, see our
[contribution guide](CONTRIBUTING.md).

Tests
-----

The unit-tests are written using the
[Catch2](https://github.com/catchorg/Catch2) framework, and mocking is
implemented using the [FakeIt](https://github.com/eranpeer/FakeIt) framework.

### Running Tests

From the `test` directory in a terminal application, execute the following command:

```none
g++ ../src/NotecardAuxiliaryWiFi.cpp NotecardAuxiliaryWiFi.test.cpp cjson.mock.cpp -Wall -Wextra -Wpedantic -I . -I ../src/ -I Catch2/single_include/ -I FakeIt/single_header/catch/ -DMOCK && ./a.out || echo "Tests Result: $?"
```

### Success

Upon success, you will see a similar message to the following:

```none
===============================================================================
All tests passed (6 assertions in 6 test cases)
```

### Failure

When a test fails, you will see a message similar to the following:

```none
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a.out is a Catch v2.13.4 host application.
Run with -? for options

-------------------------------------------------------------------------------
NotecardAuxiliaryWiFi::updateTriangulationData() returns the value parsed from
the Notecard response
-------------------------------------------------------------------------------
NotecardAuxiliaryWiFi.test.cpp:57
...............................................................................

NotecardAuxiliaryWiFi.test.cpp:74: FAILED:
  CHECK( result == JNumberValueResult+1 )
with expansion:
  9.17f == 10.17f

===============================================================================
test cases: 6 | 5 passed | 1 failed
assertions: 6 | 5 passed | 1 failed

Tests Result: 1
```

### Adding a New Test

If wish to add new functionality to the `NotecardAuxiliaryWiFi` class, then
please also add the supporting unit-tests to ensure its functionality. If you
unfamiliar with the unit-testing and mocking frameworks, please use the links
above to help get started. If you are still having trouble, reach out to us in
the [developer forums](https://discuss.blues.io/). Happy hacking!

Learn more about Blues Wireless, the Notecard and Notehub
---------------------------------------------------------

- [blues.io][blues]
- [notehub.io][notehub]
- [wireless.dev](https://dev.blues.io)

License
-------

Copyright (c) 2021 Blues, Inc. Released under the MIT license. See
[LICENSE](LICENSE.mit) for details.

[blues]: https://blues.io
[notehub]: https://notehub.io
[note-arduino]: https://github.com/blues/note-arduino
[code of conduct]: https://blues.github.io/opensource/code-of-conduct
