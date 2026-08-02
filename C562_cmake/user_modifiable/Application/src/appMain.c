#include "main.h"


/* ------------------------------------------------------------------------ */
/* 각 인터럽트 라인별 상태 플래그 (ISR에서 최소한의 작업만 하고,
   실제 처리는 메인 루프에서 하는 게 안전한 패턴)                          */
/* ------------------------------------------------------------------------ */
static volatile uint8_t button_pressed_flag = 0; // PC13
static volatile uint8_t sensor_event_flag = 0;   // PA0
static volatile uint8_t limit_switch_flag = 0;   // PB1

static void Error_Handler(void) {
  __disable_irq();
  while (1) {
    /* 필요하면 LED 깜빡임 등으로 에러 표시 */
  }
}
/* ------------------------------------------------------------------------ */
/* 라인별 개별 콜백 함수                                                    */
/* ------------------------------------------------------------------------ */
static void Button_PC13_Callback(hal_exti_handle_t *hexti,
                                 hal_exti_trigger_t trigger) {
  (void)hexti;
  (void)trigger;
  button_pressed_flag = 1;
}

static void Sensor_PA0_Callback(hal_exti_handle_t *hexti,
                                hal_exti_trigger_t trigger) {
  (void)hexti;
  (void)trigger;
  sensor_event_flag = 1;
}

static void LimitSwitch_PB1_Callback(hal_exti_handle_t *hexti,
                                     hal_exti_trigger_t trigger) {
  (void)hexti;
  (void)trigger;
  limit_switch_flag = 1;
}

/* ------------------------------------------------------------------------ */
/* 초기화 — 각 라인에 콜백 등록                                             */
/* ------------------------------------------------------------------------ */
void appInit(void) {
  hal_status_t status;

  status = HAL_EXTI_RegisterTriggerCallback(mx_gpio_default_exti13_gethandle(),
                                            Button_PC13_Callback);
  if (status != HAL_OK) {
    Error_Handler(); // 프로젝트에 정의된 에러 핸들러
  }

  status = HAL_EXTI_RegisterTriggerCallback(mx_gpio_default_exti0_gethandle(),
                                            Sensor_PA0_Callback);
  if (status != HAL_OK) {
    Error_Handler();
  }

  status = HAL_EXTI_RegisterTriggerCallback(mx_gpio_default_exti1_gethandle(),
                                            LimitSwitch_PB1_Callback);
  if (status != HAL_OK) {
    Error_Handler();
  }
}

/* ------------------------------------------------------------------------ */
/* 메인 루프 — 플래그 확인 후 실제 처리                                      */
/* ------------------------------------------------------------------------ */
void appMain(void) {

  appInit();

  while (1) {
    if (button_pressed_flag) {
      button_pressed_flag = 0;
      HAL_GPIO_TogglePin(PA5_PORT, PA5_PIN); // LED 토글
      HAL_UART_Transmit(mx_usart2_uart_gethandle(), "hello\r\n", 8 , 100);
    }

    if (sensor_event_flag) {
      sensor_event_flag = 0;
      // 센서 이벤트 처리
    }

    if (limit_switch_flag) {
      limit_switch_flag = 0;
      // 리미트 스위치 처리 (예: 모터 정지)
    }
  }
}
