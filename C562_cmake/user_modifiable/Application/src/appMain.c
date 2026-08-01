#include "main.h"
#include "stm32c5xx_hal_gpio.h"
void appMain() {

  while (1) {
    if (HAL_GPIO_ReadPin(HAL_GPIOC, HAL_GPIO_PIN_13) == HAL_GPIO_PIN_SET) {
      HAL_GPIO_TogglePin(HAL_GPIOA, HAL_GPIO_PIN_5);
      HAL_Delay(1000);
    }
  }
}
