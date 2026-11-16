# 环境配置与调试说明

说明 LED 呼吸灯项目所需的环境配置、必须安装的驱动，以及代码生成与调试的完整步骤。

## 一、环境配置

### 1. 硬件

| 项目 | 内容 |
| --- | --- |
| 开发板（实测） | ESP32-D0WD-V3，rev 3.1，双核 240 MHz |
| USB 转串口芯片 | CH340（VID_1A86 / PID_7523） |
| LED 引脚 | ESP32 用 GPIO2；Arduino Uno/Nano 用 D9 |
| 限流电阻 | 220Ω~1kΩ，串联在引脚与 LED 长脚之间 |

接线：GPIO2 → 电阻 → LED 长脚，LED 短脚 → GND。

> ESP32 的 GPIO6~GPIO11 连接内部 Flash，禁止使用。

### 2. 需要的软件与插件

| 名称 | 版本 | 下载方式 |
| --- | --- | --- |
| Arduino CLI | 1.6.0 | 下载压缩包后解压即可：`https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Windows_64bit.zip` |
| AVR 平台及依赖 | 1.8.8 | `arduino-cli core install arduino:avr` |
| ESP32 平台及依赖 | 3.3.12 | 先添加板卡地址，再执行 `arduino-cli core install esp32:esp32` |

ESP32 板卡地址：`https://espressif.github.io/arduino-esp32/package_esp32_index.json`

安装平台时会自动下载的插件：

- **AVR**（约 71 MB）：`avr`、`avr-gcc`、`avrdude`、`arduinoOTA`、`ctags`、`serial-discovery`、`serial-monitor`、`mdns-discovery`、`dfu-discovery`
- **ESP32**（约 1899 MB）：`esp32-core`、`esp-x32`、`esp-rv32`、`esptool_py`、`esp32-libs`、`esp32s2-libs`、`esp32s3-libs`、`esp32c3-libs`、`esp32c5-libs`、`esp32c6-libs`、`esp32h2-libs`、`esp32p4-libs`、`esp32p4_es-libs`、`xtensa-esp-elf-gdb`、`riscv32-esp-elf-gdb`、`openocd-esp32`、`mklittlefs`、`dfu-util`

> ESP32 平台会一次性拉取全部芯片库与两套编译器，无法只装经典 ESP32 的部分。

### 3. 必需的三项配置

**① 数据目录必须可写**
系统默认的 `Arduino15` 目录在受限环境下会出现 `Access is denied`，需要为 CLI 指定一个可写的数据目录与下载目录。

```yaml
directories:
  data: <可写的数据目录>
  downloads: <可写的下载目录>
  user: <可写的用户库目录>
board_manager:
  additional_urls:
    - https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

**② 构建目录必须显式指定**
否则 CLI 会尝试写入系统默认 sketches 目录并报权限错误，编译时必须加 `--build-path` 指向可写目录。

**③ 网络下载通道**
GitHub 直连实测仅约 26 KB/s，安装 1.9 GB 工具链会长时间卡住；使用 GitHub 加速代理可提升到 0.6~1.4 MB/s。下载脚本需支持断点续传与重试，且必须保证**单实例运行**，多进程同时写同一文件会导致包损坏。

## 二、需要下载的驱动

### CH340 串口驱动（CH341SER）

| 项目 | 内容 |
| --- | --- |
| 驱动名称 | CH341SER（WCH 官方，兼容 CH340 / CH341） |
| 获取方式 | 从 SparkFun 镜像下载 `CH341SER.EXE` |
| 安装后表现 | 设备名变为 `USB-SERIAL CH340 (COMx)`，可正常分配串口 |
| 未安装表现 | 设备管理器显示 `USB Serial`，错误码 28，系统中没有对应串口，任何工具都无法烧录 |

该安装包本质是 WinRAR 自解压文件，可静默解压后由系统接口安装，无需手动点击 INSTALL：

```powershell
# 1. 静默解压出驱动文件（含 INF、SYS、CAT）
Start-Process "CH341SER.EXE" -ArgumentList "-s","-d<目标目录>" -Wait

# 2. 以管理员身份安装驱动并重新扫描设备
pnputil /add-driver "<目标目录>\CH341SER.INF" /install
pnputil /scan-devices
```

驱动安装需要管理员权限，普通会话需通过 UAC 提权。

## 三、生成代码的步骤

1. **建立项目文件夹**
   文件夹名必须与 `.ino` 文件名一致，否则 Arduino 无法识别为 sketch。

2. **在文件头写明硬件信息**
   注明适用开发板、接线方式、可用的 PWM 引脚，方便后续维护。

3. **定义可调参数**
   `LED_PIN`（引脚）、`BREATH_STEP`（每次亮度变化量）、`STEP_DELAY_MS`（每步延时）、`PWM_FREQ_HZ` / `PWM_BITS`（PWM 参数）。

4. **按平台封装 PWM 接口**
   AVR 使用 `analogWrite()`；ESP32 使用 LEDC 外设（Core 3.x 用 `ledcAttach` / `ledcWrite`，2.x 用 `ledcSetup` + `ledcAttachPin`），用条件编译统一成一个输出函数。

5. **实现亮度映射**
   人眼对亮度感知非线性，直接把 PWM 值线性递增会出现低亮度跳变、高亮度停留过久。将亮度等级做平方校正（等效 gamma≈2.0）后再输出。

6. **编写 `setup()`**
   初始化 PWM 通道，并把 LED 置为熄灭状态。

7. **编写 `loop()`**
   渐亮循环 0→最大值，再渐暗循环最大值→0，循环体结束后自动进入下一轮。

8. **补齐端点值**
   步长无法整除最大值时，每轮结束显式写入最大值和 0，消除取整误差。

## 四、调试代码的步骤

1. **先编译，不接板子**
   分别验证多个目标平台，确认语法与资源占用正常。

   ```powershell
   arduino-cli compile --fqbn esp32:esp32:esp32 --build-path <构建目录> <项目目录>
   arduino-cli compile --fqbn arduino:avr:uno  --build-path <构建目录> <项目目录>
   ```

   实测占用：ESP32 263,547 B（20%）、Uno 1,398 B（4%）均为正常。

2. **确认驱动与串口**
   设备管理器应出现 `USB-SERIAL CH340 (COMx)`；若只有错误码 28，先装驱动。

3. **让板子进入下载模式**
   按住 `BOOT`（部分板丝印为 `IO0`）→ 点一下 `EN` / `RST` → 松开 `BOOT`。
   若板子自动下载电路正常，此步可跳过；若报 `Wrong boot mode detected`，说明必须手动操作。

4. **烧录并核对校验**
   工具会依次写入 bootloader、分区表、boot_app0、应用固件四个分区，每个分区都应出现 `Hash of data verified`，最后出现 `Hard resetting via RTS pin`，程序随即运行，无需再按复位键。

5. **观察实际现象**
   本程序不输出串口信息，串口监视器无内容属正常。判断依据是 LED 是否呈现"暗 → 亮 → 暗"的平滑循环，周期约 2.56 秒。

6. **常见问题排查**

   | 现象 | 原因 | 处理 |
   | --- | --- | --- |
   | 没有串口 | 驱动未安装（错误码 28） | 安装 CH341SER |
   | `Wrong boot mode detected` | 未进入下载模式 | 手动 BOOT + EN |
   | `Access is denied` | 数据目录或构建目录不可写 | 指定可写目录 |
   | 包下载卡在 0 字节 | GitHub 直连限速 | 使用加速代理并断点续传 |
   | 串口监视器空白 | 程序未实现串口输出 | 属正常现象 |
   | LED 不亮 | 板载 LED 不在 GPIO2 | 外接 LED 到 GPIO2 再测 |
