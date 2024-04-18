#include "stm8s.h"
#include <math.h>

// ==== TIMER ====
volatile uint32_t timer_s = 0;

INTERRUPT_HANDLER(TIM1_UPD_OVF_TRG_BRK_IRQHandler, 11)
{
  timer_s++;
  TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

void InitTimer()
{
  TIM1_TimeBaseInit(2000,TIM1_COUNTERMODE_UP, 1000, 0);
  TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
  TIM1_Cmd(ENABLE);
}

// ==== UART ====

void InitUART()
{
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
  UART1_Cmd(ENABLE);
}

uint8_t TxUart(const uint8_t *buff, uint16_t size)
{
  uint32_t safeTimer = timer_s + 2;
  FlagStatus stat;
  
  while (size--)
  {
    UART1_SendData8(*buff++);  
    do
    {
      stat = UART1_GetFlagStatus(UART1_FLAG_TXE);
      if (timer_s == safeTimer) return 0;
    }
    while (stat == RESET );
  }
  
  do
  {
    stat = UART1_GetFlagStatus(UART1_FLAG_TC);
    if (timer_s == safeTimer) return 0;
  }
  while (stat == RESET ); 
  
  return 1;
}

uint8_t RxUart(uint8_t *buff, uint16_t size)
{
  uint32_t safeTimer = timer_s + 2;
  FlagStatus stat;
  
  while (size--)
  {
    do
    {
      stat = UART1_GetFlagStatus(UART1_FLAG_RXNE);
      if (timer_s == safeTimer) return 0;
    }
    while (stat == RESET );
    
    *buff++ = UART1_ReceiveData8();    
  }  
  return 1;
}

// ==== I2C ====

void InitI2C()
{
  I2C_Init(100000, 1, I2C_DUTYCYCLE_2, I2C_ACK_NONE, I2C_ADDMODE_7BIT , 2 );
  I2C_Cmd(ENABLE);
}

// ==== INIT ====
void Init(void)
{
  // ==== CLK ====
  /*
  // Configure the Fcpu to DIV1
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);    
    // Configure the HSI prescaler to the optimal value 
    CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);        
    // Configure the system clock to use HSI clock source and to run at 16Mhz
    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);
*/
  
  enableInterrupts();
  InitTimer();
  InitUART();
  InitI2C();
  
   GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);
  
}



const double C = 423.8477;
const double k = -17.2023;
double y;

const uint8_t TxBuff[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
int main( void )
{
  Init();
  while(1)
  {
    uint32_t currentTim = timer_s;
    while (currentTim == timer_s) ;
    y = k*log(timer_s+100) + C;
    GPIO_WriteReverse(GPIOB, GPIO_PIN_5);
    TxUart(TxBuff, sizeof(TxBuff));
    
    
    
  }
}
