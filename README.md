# Contador de Flexiones

![Platform](https://img.shields.io/badge/Platform-STM32-blue)
![Framework](https://img.shields.io/badge/Framework-STM32Cube_HAL-brightgreen)
![MCU](https://img.shields.io/badge/MCU-STM32F103C6T6-orange)
![License](https://img.shields.io/badge/License-MIT-green)

Sistema embebido para contar flexiones utilizando sensor de distancia **VL53L0X** y visualización en **display de 7 segmentos multiplexado de 3 dígitos**. Proyecto desarrollado para el microcontrolador STM32F103C6T6 (Cortex-M3).

## 📋 Características

- ✅ Sensor de distancia VL53L0X (I2C)
- ✅ Display de 7 segmentos multiplexado (3 dígitos)
- ✅ 4 pulsadores para menú de configuración
- ✅ Buzzer de retroalimentación
- ✅ Multitarea no bloqueante (sin RTOS)
- ✅ Bajo consumo de RAM (~2KB)

## 🛠️ Tecnologías

| Tecnología | Versión | Descripción |
|------------|---------|-------------|
| **STM32CubeMX** | 6.12.0 | Generador de configuración de pines |
| **STM32Cube HAL** | 1.8.5 | Capa de abstracción de hardware |
| **ARM GCC Toolchain** | 14.3.1 | Compilador para STM32 |
| **VS Code** | - | Editor de desarrollo |
| **CMake** | 3.22+ | Sistema de construcción |
| **VL53L0X API** | 1.0 | Librería oficial de ST adaptada |

## 🏗️ Arquitectura del Proyecto
```
ContadorFlexionesOficial/
├── Core/ # Código generado por STM32CubeMX
├── Drivers/ # Librerías HAL y CMSIS de ST
├── MyLib/ # Librerías personalizadas
│ ├── inc/ # Archivos de cabecera (.h)
│ └── src/ # Implementaciones (.c)
├── CMakeLists.txt # Configuración de compilación
└── *.ioc # Proyecto de STM32CubeMX
```

## 🔌 Configuración de Pines

| Periférico | Pines | Configuración |
|------------|-------|---------------|
| Display (segmentos) | PA1, PA2, PA3, PA4, PA5, PA6, PA7 | GPIO_Output Push-Pull |
| Display (dígitos) | PA10, PA11, PA12 | GPIO_Output Push-Pull |
| Sensor VL53L0X | PB6 (SCL), PB7 (SDA) | I2C1 - Standard Mode (100kHz) |
| Pulsadores | PB12, PB13, PB14, PB15 | GPIO_Input con Pull-up interno |
| Buzzer | PA9 | GPIO_Output |

## ⚡ Instalación y Compilación

### Prerrequisitos

```bash
# Herramientas necesarias
- STM32CubeMX
- VS Code con extensiones:
  * STM32CubeIDE for VS Code
  * CMake Tools
- ARM GCC Toolchain
```
## Compilación
```
# Configurar el proyecto
cmake -B build

# Compilar
cmake --build build

# Generar archivo .hex para programación
arm-none-eabi-objcopy -O ihex build/Debug/ContadorFlexionesOficial.elf build/Debug/ContadorFlexionesOficial.hex
```
## 🔄 Versionado
Versión actual: 0.1.0

Estado: Desarrollo inicial - Funcionalidad básica implementada

## 👤 Autor
Rodrigo Calle Condori

GitHub: https://github.com/Rotronica


## 📄 Licencia
```
MIT License

Copyright (c) 2026 Rodrigo Calle Condori

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```