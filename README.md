# Arduino LED 呼吸灯

使用 `analogWrite()` 输出 PWM，通过逐步改变占空比，实现 LED 从熄灭到最亮、再从最亮到熄灭的循环呼吸效果。

## 项目结构

```text
BreathingLight/
├── BreathingLight.ino  # Arduino 主程序
└── README.md           # 接线和使用说明
```

## 硬件接线

以 Arduino Uno/Nano 为例：

| LED 引脚 | 连接方式 |
| --- | --- |
| 正极（长脚） | 串联 220Ω~1kΩ 限流电阻后接 D9 |
| 负极（短脚） | 接 GND |

D9 是 PWM 引脚。若需更换引脚，请修改源码中的 `LED_PIN` 宏；Uno/Nano 可使用 D3、D5、D6、D9、D10、D11。不要直接将 LED 接到非 PWM 引脚，也不要省略限流电阻。

## 参数调整

在 `BreathingLight.ino` 顶部修改：

```cpp
#define LED_PIN 9
#define BREATH_STEP 2
#define STEP_DELAY_MS 10
```

- `LED_PIN`：LED 连接的 PWM 引脚。
- `BREATH_STEP`：每次亮度变化量，必须大于等于 1。数值越小，变化越细腻，但呼吸周期越长。
- `STEP_DELAY_MS`：每一步的延时，单位毫秒。数值越小，呼吸速度越快。

默认参数下，一个完整呼吸周期约为 2.56 秒。可近似按下式估算：

```text
周期（毫秒）≈ (2 × 255 ÷ BREATH_STEP) × STEP_DELAY_MS
```

## 编译与上传

1. 使用 Arduino IDE 1.8.x 或 2.x 打开 `BreathingLight.ino`。
2. 在“工具 → 开发板”中选择实际开发板，例如 Arduino Uno 或 Arduino Nano。
3. 在“工具 → 端口”中选择开发板对应的串口。
4. 点击“验证”进行编译，确认无误后点击“上传”。
5. 上传完成后，外接 LED 将持续执行完整的呼吸循环。

程序仅使用 Arduino 核心 API，无需安装第三方库。
