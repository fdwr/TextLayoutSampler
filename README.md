# TextLayoutSampler
This utility displays text drawn via multiple Windows API's simultaneously, to compare differences in rendering, font selection, and glyph shaping.

![Image of TextLayoutSampler](TextLayoutSampler.png)

## Supports:
- DirectWrite, Direct2D, GDI, GDI+ API's
- Various attributes: weight, width, slope, family, direction, locale, font size, text color, back color, rotation, pixel zoom...
- Arbitrary number of text objects with different attributes.
- Requires Windows 7+. Certain features like Direct2D SVG rendering requires Windows 10 RS4+.

## Building:
- Open TextLayoutSampler.sln in Visual Studio Professional/Community 2019 16.9.3. VS 2017 is too old, as it lacks template constraints.
- If C++ modules crash the compiler, use normal header files via USE_CPP_MODULES=0 in the project properties preprocessor definitions.
