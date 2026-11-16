# Arduino LED 呼吸灯

通过 PWM 逐步改变占空比，实现 LED 从熄灭到最亮、再从最亮到熄灭的循环呼吸效果。
同一份代码同时支持 **Arduino Uno / Nano（AVR）** 与 **ESP32**。

## 项目结构

```text
BreathingLight/
├── BreathingLight.ino  # Arduino 主程序
└── README.md           # 接线和使用说明
```

## 硬件接线

LED 必须串联 220Ω~1kΩ 限流电阻，切勿直连电源与地。

| 平台 | 默认引脚 | 接线 |
| --- | --- | --- |
| Arduino Uno / Nano | D9（PWM） | D9 → 电阻 → LED 长脚；LED 短脚 → GND |
| ESP32 | GPIO2 | GPIO2 → 电阻 → LED 长脚；LED 短脚 → GND |

说明：

- Uno/Nano 的 PWM 引脚为 D3、D5、D6、D9、D10、D11，接非 PWM 引脚将退化为开关效果。
- ESP32 大多数开发板的板载 LED 就在 GPIO2，可不接外部 LED 直接观察。
- **ESP32 的 GPIO6~GPIO11 连接内部 Flash，严禁使用**，否则会导致程序无法启动。

## 参数调整

在 `BreathingLight.ino` 顶部修改：

```cpp
#define BREATH_STEP 2       // 每次亮度变化量
#define STEP_DELAY_MS 10    // 每一步的停留时间（毫秒）
```

- `LED_PIN`：已按平台给出默认值，需要换引脚时直接改该宏。
- `BREATH_STEP`：必须大于等于 1。数值越小变化越细腻，呼吸周期越长。
- `STEP_DELAY_MS`：数值越小呼吸速度越快。

默认参数下一个完整呼吸周期约 2.56 秒，可近似按下式估算：

```text
周期（毫秒）≈ (2 × 255 ÷ BREATH_STEP) × STEP_DELAY_MS
```

## 实现要点

- **PWM 输出**：AVR 使用 `analogWrite()`；ESP32 使用 LEDC 硬件外设（`ledcAttach` / `ledcWrite`），并同时兼容 ESP32 Core 2.x 与 3.x 两套接口。
- **亮度校正**：人眼对亮度的感知是非线性的，代码用平方曲线（等效 gamma≈2.0）把线性等级映射为占空比，避免低亮度段跳变、高亮度段停留过久，使呼吸过渡更自然。
- **端点保护**：每轮结束显式写入 0 和最大值，抵消步长不能整除 255 带来的取整误差。

## 编译与上传

### Arduino Uno / Nano

1. 用 Arduino IDE 打开 `BreathingLight.ino`。
2. 开发板选择 `Arduino Uno` 或 `Arduino Nano`（克隆 Nano 若上传失败，处理器选 `ATmega328P (Old Bootloader)`）。
3. 选择对应串口，点击上传。

### ESP32

1. 首次使用需在 Arduino IDE 的“开发板管理器”中安装 `esp32` 支持包（Espressif Systems）。
2. 开发板选择 `ESP32 Dev Module`。
3. 选择对应串口（`CP210x` 或 `CH340`），点击上传。
4. 若提示等待连接超时，上传时按住开发板的 `BOOT` 键，出现 `Connecting...` 后松开。

程序仅使用 Arduino 核心与 ESP32 自带 LEDC 接口，无需安装第三方库。
