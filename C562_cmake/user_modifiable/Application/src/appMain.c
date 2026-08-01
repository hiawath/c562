#include "main.h"


void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti, hal_exti_trigger_t trigger)
{
    if (hexti == mx_gpio_default_exti13_gethandle())
    {
        // PC13 버튼 눌림 처리
        HAL_GPIO_TogglePin(PA5_PORT, PA5_PIN);  // 예: LED 토글
    }
}

void appMain() {
  

  while (1) {
    HAL_Delay(100);
  }
}
