## cwds related instructions

- Every .cxx file must start with `#include "sys.h"`, no exceptions except third party source code.
- All debug output is done using `Dout`, `DoutEntering` (used at the top of functions) and `DoutFatal(dc::core, ...)`
  Use the debug channel appropriate for the current section of the code, or `dc::notice` is none is specified yet.
- Always use `ASSERT` for assertions that test proper use of the code (they are debug mode only).
  `assert` is for fatal runtime errors due to circumstances outside the control of the author of the code that still must fire in release mode.
- If debug output and/or ASSERT is used in a file then that file must #include "debug.h". Put that include last, under all other includes.
