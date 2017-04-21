# Gluten GUI System (WIP)

Gluten provides not only a powerful GUI library but also a complete inbuilt layout designer.

  - **Virtually no dependencies** - Gluten requires only X11 *or* SDL.
  - **Inbuilt Editor** - Press *F12* during execution to open up the built in designer.
  - **Easy to integrate** - Only a single .h and .c file need to be added to your project.
  - **Standard C** - Compiles with any ANSI C89 compiler or newer.
  - **Extremely portable** - Compiles on any platform providing *X11* or *SDL*.
  - **Extensible** - Using the underlying component entity system, new widgets can be easily added.

# Add a GUI in just a few lines of code!

```c
/* Provides the entire functionality of the GUI library and tools */
#include <gluten.h>

int main(int argc, char *argv[])
{
  /* Load layout if exists or an empty form ready to be populated */
  GnInit(argc, argv, "example.gluten");
  
  /* Process GUI events until window is closed */
  GnRun();

  /* No cleanup is required */
  return 0;
}
```
