# 个人游戏引擎

这是一个模块化的 C++ 游戏引擎，旨在帮助学习和实现基本的游戏引擎系统。它包括多个模块，如渲染、资源管理、UI 等，并使用 Vulkan 作为主要的渲染 API。该引擎的结构被拆分为多个静态库和动态链接库（DLL），通过 `xmake` 管理。

## 功能
- **模块化架构**：将引擎拆分成多个独立模块，提高了可维护性和可扩展性。
- **全局接口**：通过 `engine.dll` engine.dll 管理Pimpl导出接口， `singleton.dll` 保证全局变量指向的内存唯一。 
- **渲染**：使用 Vulkan 实现，且有抽象层支持未来扩展到 OpenGL、DirectX 和 Metal 等其他渲染 API。
- **资源管理**：包括基础的资源管理、加载和序列化功能。
- **反射序列化**：通过代码生成实现反射功能，支持 JSON, YAML 的序列化与反序列化
- **UI**：初步集成了 NoesisGUI 和 ImGui 进行 XAML UI 元素的渲染，未来计划使用 ImGui 和 NoesisGUI 开发编辑器和游戏UI。

## 安装
安装 [LLVM](https://releases.llvm.org/) 、[xmake](https://github.com/xmake-io/xmake)

`xmake project -k vsxmake2022 -m "debug;release"`

`如果证书报错 git config --global http.schannelCheckRevoke false`

## 项目结构
- **engine.dll**：核心 DLL，负责整合所有模块并管理全局状态和模块生命周期。
- **core.lib, render.lib, asset.lib, app.lib, ui.lib**：提供不同功能的静态库（渲染、资源加载、UI 处理等），链接到 `engine.dll`。
- **singleton.dll**：确保全局变量在跨模块时唯一，并集中管理。
- **zlib.lib**：提供压缩、反射和内存管理优化。
- **vulkan.dll**：包含 Vulkan 特定的渲染逻辑，和 `engine.dll`、`editor.dll` 进行交互。
- **editor.dll**：使用 ImGui 和 NoesisGUI 提供编辑器界面（仍在开发中）。
- **zworld.exe**：目标游戏执行文件，当前实现了 Vulkan 渲染三角形，并展示了 NoesisGUI 的 XAML UI。

### 依赖项目
[代码生成工具](https://github.com/ouczbs/xmake.repo)

[Xmake库](https://github.com/ouczbs/xmake.repo.git)

[项目原地址](http://175.24.226.114:3000/ouczbs/zengine.git)
### 模块工作方式
- **全局变量**：所有全局变量都包含在 `engine.dll` 中，其他模块（如 `core.lib`、`render.lib` 等）可以通过指针引用访问。这减少了重复性并确保模块间的一致性。
- **模块化逻辑**：不是通过 DLL 接口导出所有函数，而是只导出必要的函数，最大限度地减少了冗余，确保代码简洁可维护。
- **生命周期管理**：`core` 模块负责其他模块的初始化、生命周期和销毁，确保它们按正确顺序加载，资源安全清理。

## 详细模块描述

### 1. singleton.dll
确保跨模块的全局变量唯一性，使用指针引用进行管理。每个模块都获取这些变量的指针副本，但它们指向相同的内存位置。

- **全局变量管理**：保证跨模块的共享资源保持一致，避免重复或误管理。
- **跨模块访问**：模块可以访问共享变量，而不必担心重复问题。

### 2. zlib.lib
提供多种实用工具，包括反射和高级内存管理。

- **PMR 内存管理**：使用多态内存资源 (`pmr`) 优化内存分配和释放策略。
- **反射**：实现基础的运行时反射，用于数据序列化和反序列化（JSON、YAML）。
- **模板元编程**：通过模板提供高效的编译时计算和类型操作。

### 3. core.lib
处理引擎的核心功能，包括模块管理、序列化和文件挂载。

- **模块管理**：负责模块的初始化、生命周期和销毁，确保按正确顺序初始化。
- **序列化**：使用 `zlib.lib` 的反射功能实现 JSON 和 YAML 序列化/反序列化。
- **文件挂载**：提供将资源从各种源（如本地文件系统或打包文件）挂载和加载的功能。

### 4. asset.lib
管理游戏资源，包括加载、缓存和序列化。

- **资源管理**：高效管理纹理、模型、声音等资源的生命周期。
- **资源加载**：支持从多种格式加载资源，如图像和 3D 模型。
- **序列化**：实现资源的保存和加载功能。

### 5. render.lib
该模块抽象了渲染硬件接口（RHI），并处理渲染管线管理，包括支持 Vulkan。

- **RHI 抽象**：当前仅实现 Vulkan API，但系统设计支持未来扩展到 OpenGL、DirectX 和 Metal 等渲染 API。
- **FrameGraph**：引入 `FrameGraph` 概念来管理渲染通道和资源依赖，优化渲染管线。
- **材质系统**：简单的材质系统，处理渲染用的材质属性和着色器。
- **GPU 动态缓冲区**：实现了 GPU 端的动态缓冲区，供 NoesisGUI 渲染 UI 元素时使用。
- **Vulkan 集成**：完全集成 Vulkan，处理命令缓冲区、资源管理和同步。

### 6. ui.lib
处理 UI 集成，最初关注于 NoesisGUI。

- **NoesisGUI 集成**：提供一个基础系统，通过 NoesisGUI 渲染 XAML 文件，支持游戏和编辑器中的 UI 渲染。
- **UI 渲染**：管理 UI 元素的渲染及与 Vulkan 的交互。
- **未来扩展**：计划扩展该模块，支持其他 UI 框架（如 ImGui）来处理更复杂的编辑器和游戏内 UI。

### 7. app.lib
包含整个项目的全局设置和事件系统。

- **项目设置**：管理引擎和游戏的全局配置和设置。
- **事件系统**：实现事件驱动系统，处理各种引擎和游戏事件，使不同模块解耦。

### 8. editor.dll
一个进行中的编辑器模块，用于管理游戏开发过程。

- **编辑器 UI**：最终将使用 ImGui 和 NoesisGUI 创建游戏和资产管理的编辑器界面。
- **编辑器工具**：包含早期功能，用于构建场景编辑器、资产查看器等开发工具。

### 9. vulkan.dll
管理 Vulkan 特定的渲染逻辑。

- **Vulkan RHI 实现**：包含所有 Vulkan 特定的代码，管理命令缓冲区、着色器和同步。
- **NoesisGUI 渲染**：将 NoesisGUI 集成到 Vulkan 管道中，进行 UI 元素渲染。

### 10. zworld.exe (目标游戏)
目标游戏的执行文件，当前处于早期阶段，使用 Vulkan 实现了基本渲染。

- **Vulkan 渲染三角形**：演示如何使用 Vulkan 渲染一个简单的三角形到 ImGui 的 image 控件中。
- **NoesisGUI 集成**：通过 Vulkan 和 NoesisGUI 渲染一个简单的 XAML UI 文件，展示游戏中的 UI 渲染。

## 未来计划
- **更多渲染 API**：扩展渲染系统，支持 OpenGL、DirectX 和 Metal，实现跨平台兼容性。
- **游戏逻辑与玩法**：将游戏逻辑、物理和 AI 系统整合进引擎，创建完整的游戏环境。
- **高级资源管理**：添加更多的资源格式，并优化资源加载管道以提高性能。
- **编辑器开发**：继续开发编辑器，为场景管理、资产创建和调试提供工具。
