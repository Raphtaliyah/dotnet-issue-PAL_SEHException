Example code to reproduce ``PAL_SEHException`` caused by ``SuppressGCTransition`` on ``DllImport`` in release mode with hosted CoreCLR.

The C# library has to be built in **release** mode.

The runtime path is set to ``runtime`` by default, it can be changed in ``main.cpp`` (``RUNTIME_PATH``).

Tested on Ubuntu 22.04 x64 with .NET 8.0.0-rc.2.23479.6 and built from ``9a6d53050f15071287e6d2b111be80af54e43689``.