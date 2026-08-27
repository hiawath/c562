#include "main.h"
#include <stdio.h>
#include <string.h>

#include "example.h"
#include "mx_system.h"

#include "mx_usart2.h" 
#include "mx_basic_stdio_app.h"

#define RX_BUF_SIZE 64

static uint8_t rx_buffer[RX_BUF_SIZE];
static uint8_t tx_buffer[RX_BUF_SIZE];   // 송신 전용 버퍼 추가

static volatile uint8_t uart_error_flag=0;
static volatile uint8_t rx_done_flag = 0;
static volatile uint32_t rx_received_size = 0;

static hal_uart_handle_t *huart2;
app_status_t ExecStatus = EXEC_STATUS_UNKNOWN; /* application status */
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
/* weak 함수 오버라이드 (RegisterCallback 대신)                              */
/* ------------------------------------------------------------------------ */
void HAL_UART_RxCpltCallback(hal_uart_handle_t *huart, uint32_t size_byte,
                              hal_uart_rx_event_types_t rx_event)
{
    if (huart == huart2)
    {
        (void)rx_event;
        /* 받은 즉시 tx_buffer로 복사 (콜백 안이라 최대한 짧게) */
        memcpy(tx_buffer, rx_buffer, size_byte);
        rx_received_size = size_byte;
        rx_done_flag = 1;
    }
}

void HAL_UART_ErrorCallback(hal_uart_handle_t *huart) {
  if (huart == huart2) {
    uart_error_flag = 1;
  }
}

static void error_handler(void)
{

  while (1)
  {
    /* Repeated flashing status LED (50ms on and 2sec off) when execution loop is exited */
    led_on(LED_0);
    HAL_Delay(50);
    led_off(LED_0);
    HAL_Delay(2000);
  }
} /* end error_handler */

/** brief:  Success notification
  * retval: None (infinite loop)
  */
static void success_handler(void)
{

  /* Report success: the status LED remains turned on */
  //led_on(LED_0);

  //while (1);
} 

/* ------------------------------------------------------------------------ */
/* 초기화 — 각 라인에 콜백 등록                                             */
/* ------------------------------------------------------------------------ */
system_status_t appInit(void) {
  hal_status_t status;
   
  mx_basic_stdio_init();

  huart2 = mx_usart2_uart_gethandle();
  if (huart2 == NULL) {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  if (HAL_UART_ReceiveToIdle_DMA(huart2, rx_buffer, RX_BUF_SIZE) != HAL_OK) {
    return SYSTEM_PERIPHERAL_ERROR;
  }

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

  return HAL_OK;
}



/* ------------------------------------------------------------------------ */
/* 메인 루프 — 플래그 확인 후 실제 처리                                      */
/* ------------------------------------------------------------------------ */
void appMain(void) {
  
  appInit();

  mx_system_init();

  ExecStatus =app_init();


  /* Run app_process if no error occurs  */
    if (ExecStatus != EXEC_STATUS_ERROR)
    {
      ExecStatus = app_process();
    }

    if (ExecStatus == EXEC_STATUS_OK)
    {
      ExecStatus = app_deinit();
    }

      /* Report the example status */
  if (ExecStatus == EXEC_STATUS_OK)
  {
    success_handler();
  }
  else
  {
    error_handler();
  }

  // 부팅 안내 메시지 LPDMA1(Tx)로 전송
  const char *init_msg = "STM32C562 USART2 LPDMA(Rx:0 / Tx:1) Ready!\r\n";
  HAL_UART_Transmit_DMA(mx_usart2_uart_gethandle(), (uint8_t *)init_msg,
                        strlen(init_msg));

                       
  while (1) {
    if (button_pressed_flag) {
      button_pressed_flag = 0;
      HAL_GPIO_TogglePin(PA5_PORT, PA5_PIN); // LED 토글
      HAL_UART_Transmit_DMA(huart2, "hello\r\n", 8);
      



    }


    if (sensor_event_flag) {
      sensor_event_flag = 0;
      // 센서 이벤트 처리
    }

    if (limit_switch_flag) {
      limit_switch_flag = 0;
      // 리미트 스위치 처리 (예: 모터 정지)
    }

          
    if (uart_error_flag) {
      uart_error_flag = 0;
      HAL_UART_ReceiveToIdle_DMA(huart2, rx_buffer, RX_BUF_SIZE);
    }

    if (rx_done_flag)
    {
        rx_done_flag = 0;
        HAL_UART_Transmit_DMA(huart2, tx_buffer, rx_received_size);  // tx_buffer에서 송신
        HAL_UART_ReceiveToIdle_DMA(huart2, rx_buffer, RX_BUF_SIZE);  // rx_buffer는 다시 수신 전용
    }
    
  }
}
