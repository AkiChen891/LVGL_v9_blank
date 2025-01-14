# 为正点原子阿波罗STM32F429开发的基于LVGL v9.2的空白工程

这是一个空白工程，已经移植了LVGL v9.2版本。

## 系统要求
- IAR Embedded Workbench 8.40.2 或更高版本
- STM32F429IGT6
- 开发板：正点原子阿波罗 STM32F429
- 外接屏幕：本例程 使用1024*600 7inch RGB屏

## 依赖项
- STM32CubeMX：用于生成初始化代码。
- STM32 HAL 库：用于硬件抽象层的驱动。
- LVGL 库：用于显示界面。

## 文件结构
- `Core/`：核心驱动文件夹，包含必要的外设及系统驱动。
- `Drivers/`：系统驱动。
- `EWARM/`：与IAR有关的工程文件及依赖项。
- `lvgls/`：LVGL库文件。
