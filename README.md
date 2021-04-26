# TextLayoutSampler
Utility to display text via multiple Windows API's simultaneously to compare differences in rendering, font selection, and shaping.

Supports:
- Arbitrary number of text objects with different text, font, direction, language, and rendering settings.
- DirectWrite, Direct2D, GDI, GDI+
- Requires Windows 7+. Certain features like Direct2D SVG rendering requires Windows 10 RS4.

C++, compiled with Visual Studio Professional 2019 16.9.3 (Community Edition should work too). VS 2017 is too old, as it lacks template constraints.
Note the project now uses C++ modules.
To use normal header files, USE_CPP_MODULES=0 in the project properties preprocessor definitions, because modules tend to crash the compiler.

![Image of TextLayoutSampler](TextLayoutSampler.png)
