# sm64pc
本项目是 [n64decomp/sm64](https://github.com/n64decomp/sm64) 的 OpenGL 移植版本。

我们欢迎贡献代码与 bug 报告，但请切记，**不得上传任何被版权保护（来自 ROM 文件）的资源**。
提交前请运行 `./extract_assets.py --clean && make clean` 或 `make distclean` 来清除所有从 ROM 文件中提取的内容。
本移植是基于 [Emill](https://github.com/Emill) 的工作 [n64-fast32-engine](https://github.com/Emill/n64-fast3d-engine/) 才得以实现的。

## 主要功能

 * 原生渲染。现在不用任何模拟器就可以运行 马力欧64 了。 
 * 长宽比和分辨率可以自由改变。本游戏目前可以在几乎任何窗口尺寸下正确渲染。
 * 原生 xinput 手柄支持。在 Linux 下，已经确认 PS4 手柄可以即插即用。
 * 支持模拟量视点控制、鼠标控制视点。（请使用 `make BETTERCAMERA=1` 编译）
 * 可取消可视距离限制。（请使用 `make NODRAWINGDISTANCE=1` 编译）
 * 游戏内操作设定功能，目前在 `testing` 分支下可用。
 * 使用 `--skip-intro` 命令行选项跳过碧奇公主与 Lakitu 的片头剧情。目前在 `testing` 及 `skip-intro` 分支下可用。

## 编译方法
关于如何编译，请参考 [wiki](https://github.com/sm64pc/sm64pc/wiki)。

**请勿在 Linux 或者 WSL 下使用 `WINDOWS_BUILD=1` 参数尝试编译 Windows 版本，这样无法编译成功。请参考 Wiki。
