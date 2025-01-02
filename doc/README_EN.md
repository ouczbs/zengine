# Personal Game Engine

This is a modular C++ game engine designed to help learn and implement essential game engine systems. It includes various modules for rendering, asset management, UI, and more, and uses Vulkan as the primary rendering API. The engine is structured into several libraries and dynamic link libraries (DLLs), all managed with `xmake`.

## Features
- **Modular Architecture**: The engine is broken into independent modules to improve maintainability and extensibility.
- **Rendering**: Implemented using Vulkan, with an abstraction layer for possible future expansion to OpenGL, DirectX, and Metal.
- **Asset Management**: Includes basic resource management, loading, and serialization.
- **UI**: Basic UI system integration with NoesisGUI for rendering XAML UI elements.
- **Global Variables**: Centralized management of global state in `engine.dll` to avoid inconsistencies across modules.
- **Efficient Memory Management**: Includes optimizations for memory allocation using `pmr` memory resources and template metaprogramming.
- **Editor Support**: Basic integration for editor functionality, with plans for full editor tools using ImGui and NoesisGUI.

## Project Structure
- **engine.dll**: The core DLL that integrates all the modules and manages global state and module lifecycle.
- **core.lib, render.lib, asset.lib, app.lib, ui.lib**: Static libraries providing different functionalities (rendering, asset loading, UI handling, etc.) linked into `engine.dll`.
- **singleton.dll**: Ensures that global variables across modules are unique and managed centrally.
- **zlib.lib**: Provides compression, reflection, and memory management optimizations.
- **vulkan.dll**: Contains the Vulkan-specific rendering logic, interfacing with `engine.dll` and `editor.dll`.
- **editor.dll**: Provides the editor interface using ImGui and NoesisGUI (still in early development).
- **zworld.exe**: The target game executable that currently renders a triangle with Vulkan and displays a NoesisGUI XAML UI.

### How the Modules Work
- **Global Variables**: All global variables are contained within `engine.dll`, which other modules (like `core.lib`, `render.lib`, etc.) can access via pointer references. This reduces duplication and ensures consistency across modules.
- **Modular Logic**: Instead of exporting all functions through DLL interfaces, only the necessary functions are exported, minimizing bloat and ensuring a clean and maintainable codebase.
- **Lifecycle Management**: The `core` module manages the initialization, lifecycle, and shutdown of other modules, ensuring they are loaded in the correct order and resources are cleaned up safely.

## Detailed Module Descriptions

### 1. singleton.dll
This module ensures that global variables across different modules are unique by using pointer references. Each module gets a copy of the pointer to these variables, but they all point to the same memory location.

- **Global Variable Management**: Guarantees that shared resources across modules remain consistent and avoid duplication or mismanagement.
- **Cross-Module Access**: Modules can access shared variables without the risk of duplication.

### 2. zlib.lib
Provides a variety of utilities, including compression support and advanced memory management.

- **PMR Memory Management**: Optimized memory allocation and deallocation strategies using polymorphic memory resources (`pmr`).
- **Reflection**: Implements basic runtime reflection for data serialization and deserialization (JSON, YAML).
- **Template Metaprogramming**: High-performance template-based utilities for compile-time calculations and type manipulation.

### 3. core.lib
Handles the core functionality of the engine, including module management, serialization, and file mounting.

- **Module Management**: Manages the initialization, lifecycle, and shutdown of modules, ensuring that they are initialized in the correct order.
- **Serialization**: Implements JSON and YAML serialization/deserialization using the reflection features of `zlib.lib`.
- **File Mounting**: Provides the functionality to mount and load resources from various sources, such as local file systems or packaged files.

### 4. asset.lib
Manages game assets, including their loading, caching, and serialization.

- **Resource Management**: Efficiently manages the lifecycle of assets like textures, models, and sounds.
- **Asset Loading**: Supports loading assets from various formats, such as images and 3D models.
- **Serialization**: Implements functionality to save and load assets to and from disk.

### 5. render.lib
This module abstracts the rendering hardware interface (RHI) and handles rendering pipeline management, including support for Vulkan.

- **RHI Abstraction**: While Vulkan is currently the only fully implemented API, the system is designed to support other rendering APIs (OpenGL, DirectX, Metal) in the future.
- **FrameGraph**: Introduces the `FrameGraph` concept to manage render passes and resource dependencies, optimizing the render pipeline.
- **Material System**: A simple material system that handles material properties and shaders for rendering.
- **GPU Dynamic Buffers**: Implements dynamic buffers for GPU-side storage, which is used by NoesisGUI to render UI elements.
- **Vulkan Integration**: Fully integrates Vulkan, handling command buffers, resource management, and synchronization.

### 6. ui.lib
Handles the integration of user interfaces, initially focused on NoesisGUI.

- **NoesisGUI Integration**: Provides a basic system to render XAML files using NoesisGUI, supporting UI rendering within the game and editor.
- **UI Rendering**: Manages the rendering of UI elements and their interaction with Vulkan.
- **Future Expansion**: Plans to extend the module to support other UI frameworks such as ImGui for more complex editor and in-game UIs.

### 7. app.lib
Contains global settings and event systems for the entire project.

- **Project Settings**: Manages global configurations and settings for the engine and game.
- **Event System**: Implements an event-driven system for handling various engine and game events, decoupling different parts of the engine.

### 8. editor.dll
A work-in-progress editor module for managing the game development process.

- **Editor UI**: Will eventually use ImGui and NoesisGUI to create an editor interface for game and asset management.
- **Editor Tools**: Contains early-stage functionality to build scene editors, asset viewers, and other development tools.

### 9. vulkan.dll
Manages Vulkan-specific rendering logic.

- **Vulkan RHI Implementation**: Contains all Vulkan-specific code for managing command buffers, shaders, and synchronization.
- **NoesisGUI Rendering**: Integrates NoesisGUI into the Vulkan pipeline to render UI elements.

### 10. zworld.exe (Target Game)
The executable for the target game, currently in its early stages, implements basic rendering using Vulkan.

- **Vulkan Triangle Rendering**: Demonstrates the use of Vulkan to render a simple triangle on screen.
- **NoesisGUI Integration**: Renders a simple XAML UI file using Vulkan and NoesisGUI, showcasing UI rendering in the game.

## Future Plans
- **More Rendering APIs**: Expand the rendering system to support OpenGL, DirectX, and Metal, allowing for cross-platform compatibility.
- **Game Logic and Gameplay**: Integrate game logic, physics, and AI systems into the engine to create a full-fledged game environment.
- **Advanced Asset Management**: Add more asset formats and optimize the resource loading pipeline for better performance.
- **Editor Development**: Continue development on the editor to provide tools for scene management, asset creation, and debugging.