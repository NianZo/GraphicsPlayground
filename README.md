[![Actions Status](https://github.com/NianZo/GraphicsPlayground/actions/workflows/cmake.yml/badge.svg)](https://github.com/NianZo/GraphicsPlayground/actions)
[![Actions Status](https://github.com/NianZo/GraphicsPlayground/actions/workflows/ClangFormat.yml/badge.svg)](https://github.com/NianZo/GraphicsPlayground/actions)
[![Actions Status](https://github.com/NianZo/GraphicsPlayground/actions/workflows/ClangTidy.yml/badge.svg)](https://github.com/NianZo/GraphicsPlayground/actions)
[![codecov](https://codecov.io/gh/NianZo/GraphicsPlayground/branch/main/graph/badge.svg?token=ILR5XG9D4V)](https://codecov.io/gh/NianZo/GraphicsPlayground)
# GraphicsPlayground
GUI application to practice rendering techniques with Vulkan

## GUI Framework
This project uses Qt as its GUI framework. The Qt-Vulkan interop was a bit tricky to get right, especially since Qt has a very heavy-handed built in integration. I documented how I interfaced Qt with an 'external' Vulkan renderer here: https://www.niangames.com/articles/qt-vulkan-renderer

## Cross Platform Support
Qt does most of the heavy lifting when it comes to cross platform support of this application. There is one hiccup though. Vulkan seems to need a platform specific surface extension. For Linux, this is "VK_KHR_xcb_surface". For Windows, this would certainly be a win32 variant. Other than this quirk, the application SHOULD be completely platform agnostic. That being said, I haven't tested building or running on any platform other than Linux.
