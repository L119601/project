/*
 * LED 呼吸灯
 * 支持开发板：Arduino Uno / Nano（AVR）、ESP32
 *
 * 接线方式（LED 需串联 220Ω~1kΩ 限流电阻）：
 *   AVR  ：D9    -> 电阻 -> LED 正极；LED 负极 -> GND
 *   ESP32：GPIO2 -> 电阻 -> LED 正极；LED 负极 -> GND
 *          （多数 ESP32 开发板的板载 LED 就接在 GPIO2）
 *
 * 修改引脚：改下方 LED_PIN 宏。
 *   AVR   可用 PWM 引脚：3、5、6、9、10、11
 *   ESP32 可用任意输出 GPIO，但 GPIO6~GPIO11 连接内部 Flash，禁止使用
 */

// ------------------------- 可调整参数 -------------------------

#if defined(ARDUINO_ARCH_ESP32)
#define LED_PIN 2           // ESP32：板载 LED 通常为 GPIO2，也可改为 4、5、18、19 等
#else
#define LED_PIN 9           // Uno/Nano：D9（硬件 PWM）
#endif

#define BREATH_STEP 2       // 每次亮度变化量，必须 >= 1；越小越细腻、周期越长
#define STEP_DELAY_MS 10    // 每一步的停留时间（毫秒）；越小呼吸越快

// PWM 参数：ESP32 由 LEDC 硬件外设产生，AVR 由 Timer 硬件产生，均为 8 位。
#define PWM_FREQ_HZ 5000    // PWM 频率，远高于人眼闪烁感知范围
#define PWM_BITS 8          // 8 位分辨率，占空比范围 0~255
#define PWM_CHANNEL 0       // LEDC 通道号，仅 ESP32 Core 2.x 需要

const int PWM_MAX = (1 << PWM_BITS) - 1;

// 初始化 PWM 输出。ESP32 Core 3.x 与 2.x 的 LEDC 接口不同，这里做兼容处理。
void pwmBegin() {
#if defined(ARDUINO_ARCH_ESP32)
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(LED_PIN, PWM_FREQ_HZ, PWM_BITS);   // 3.x：按引脚绑定
#else
  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_BITS);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);          // 2.x：按通道绑定
#endif
#else
  pinMode(LED_PIN, OUTPUT);
#endif
}

// 输出占空比（0~PWM_MAX）。
void pwmWrite(int duty) {
#if defined(ARDUINO_ARCH_ESP32)
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(LED_PIN, duty);
#else
  ledcWrite(PWM_CHANNEL, duty);
#endif
#else
  analogWrite(LED_PIN, duty);
#endif
}

// 将亮度等级（0~PWM_MAX）转换为实际占空比后输出。
// 人眼对亮度的感知是非线性的，直接把 PWM 值线性递增会出现
// 低亮度段“跳变”、高亮度段停留过久的问题。这里用平方曲线做校正，
// 使视觉上的明暗变化均匀，呼吸过渡更自然。
void setLedBrightness(int level) {
  level = constrain(level, 0, PWM_MAX);
  int duty = (int)((long)level * level / PWM_MAX);  // 等效 gamma≈2.0，端点仍为 0 和最大值
  pwmWrite(duty);
}

void setup() {
  pwmBegin();                // 初始化 PWM 输出通道
  setLedBrightness(0);       // 上电时先熄灭 LED
}

void loop() {
  // 渐亮：从熄灭变化到最亮。
  for (int level = 0; level <= PWM_MAX; level += BREATH_STEP) {
    setLedBrightness(level);
    delay(STEP_DELAY_MS);
  }
  setLedBrightness(PWM_MAX);  // 补偿步长不能整除最大值的情况，确保到达最亮。

  // 渐暗：从最亮变化到熄灭。
  for (int level = PWM_MAX; level >= 0; level -= BREATH_STEP) {
    setLedBrightness(level);
    delay(STEP_DELAY_MS);
  }
  setLedBrightness(0);        // 确保完整呼吸周期结束时 LED 完全熄灭。

  // loop() 结束后会自动再次执行，形成连续的呼吸循环。
}
