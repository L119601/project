/*
 * LED 呼吸灯
 * 适用开发板：Arduino Uno、Nano 等支持 analogWrite() 的 Arduino 开发板
 *
 * 接线方式：
 *   LED 正极（长脚） -> 220Ω~1kΩ 限流电阻 -> 数字引脚 D9（PWM）
 *   LED 负极（短脚） -> GND
 *
 * 如需更换引脚，修改 LED_PIN 即可。Uno/Nano 可选 PWM 引脚：
 *   3、5、6、9、10、11
 */

// ------------------------- 可调整参数 -------------------------

#define LED_PIN 9           // LED 连接的 PWM 引脚
#define BREATH_STEP 2       // 每次亮度变化量，必须 >= 1；越小越细腻、周期越长
#define STEP_DELAY_MS 10    // 每一步的停留时间（毫秒）；越小呼吸越快

// Arduino Uno/Nano 的 analogWrite() 占空比范围为 0~255。
const int PWM_MAX = 255;

// 将亮度限制在有效 PWM 范围后输出，确保端点值准确。
void setLedBrightness(int brightness) {
  brightness = constrain(brightness, 0, PWM_MAX);
  analogWrite(LED_PIN, brightness);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);  // 将 LED 引脚配置为输出
  setLedBrightness(0);       // 上电时先熄灭 LED
}

void loop() {
  // 渐亮：从熄灭变化到最亮。
  for (int brightness = 0; brightness <= PWM_MAX; brightness += BREATH_STEP) {
    setLedBrightness(brightness);
    delay(STEP_DELAY_MS);
  }
  setLedBrightness(PWM_MAX);  // 补偿步长不能整除 255 的情况，确保到达最亮。

  // 渐暗：从最亮变化到熄灭。
  for (int brightness = PWM_MAX; brightness >= 0; brightness -= BREATH_STEP) {
    setLedBrightness(brightness);
    delay(STEP_DELAY_MS);
  }
  setLedBrightness(0);        // 确保完整呼吸周期结束时 LED 完全熄灭。

  // loop() 结束后会自动再次执行，形成连续的呼吸循环。
}
