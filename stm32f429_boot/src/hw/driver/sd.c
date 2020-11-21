/*
 * sd.c
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */




#include "sd.h"


#ifdef _USE_HW_SD
#include "gpio.h"
#include "cmdif.h"


//-- Internal Variables
//
static bool is_init = false;
static bool is_detected = false;
static SD_HandleTypeDef uSdHandle;




//-- External Variables
//


//-- Internal Functions
//
#ifdef _USE_HW_CMDIF
void sdCmdifInit(void);
void sdCmdif(void);
#endif

//static void sdInitHw(void);


//-- External Functions
//




bool sdInit(void)
{
  HAL_StatusTypeDef status;


  /* uSD device interface configuration */
  uSdHandle.Instance = SDIO;

  uSdHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
  uSdHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
  uSdHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  uSdHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
  uSdHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV;


  if (gpioPinRead(_PIN_GPIO_SDCARD_DETECT) != _DEF_HIGH)
  {
    logPrintf("sdCard     \t\t: not connected\r\n");
    return false;
  }
  else
  {
    logPrintf("sdCard     \t\t: connected\r\n");
  }

  is_detected = true;

  HAL_SD_DeInit(&uSdHandle);
  status = HAL_SD_Init(&uSdHandle);
  if(status != HAL_OK)
  {
    logPrintf("sdCard     \t\t: fail, %d\r\n", status);
    return false;
  }
  else
  {
#if 0
    /* Enable wide operation */
    if(HAL_SD_ConfigWideBusOperation(&uSdHandle, SDIO_BUS_WIDE_4B) != HAL_OK)
    {
      logPrintf("sdCard 4bit \t\t: fail\r\n");
      return false;
    }
#endif
  }

  logPrintf("sdCard     \t\t: OK\r\n");
  is_init = true;


#ifdef _USE_HW_CMDIF
  static bool is_cmd_init = false;

  if (is_cmd_init == false)
  {
    sdCmdifInit();
    is_cmd_init = true;
  }
#endif

  return is_init;
}

bool sdDeInit(void)
{
  bool ret = true;


  uSdHandle.Instance = SDIO;

  if(HAL_SD_DeInit(&uSdHandle) != HAL_OK)
  {
    ret = false;
  }

  HAL_NVIC_DisableIRQ(SDIO_IRQn);
  __HAL_RCC_SDIO_CLK_DISABLE();


  is_init = false;

  return ret;
}


bool sdReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;


  if(HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    while(sdIsBusy() == true);
    ret = true;
  }

  return ret;
}

bool sdWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;


  if(HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }

  return ret;
}

bool sdEraseBlocks(uint32_t start_addr, uint32_t end_addr)
{
  bool ret = false;


  if(HAL_SD_Erase(&uSdHandle, start_addr, end_addr) == HAL_OK)
  {
    ret = true;
  }

  return ret;
}

bool sdIsBusy(void)
{
  bool is_busy;


  if (HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER )
  {
    is_busy = false;
  }
  else
  {
    is_busy = true;
  }

  return is_busy;
}

bool sdIsDetected(void)
{
  bool ret = false;

#if 0
  if (gpioPinRead(_PIN_GPIO_SDCARD_DETECT) == _DEF_LOW)
  {
    ret = true;
  }
#else
  ret = is_detected;
#endif

  return ret;
}

bool sdGetInfo(sd_info_t *p_info)
{
  bool ret = false;
  sd_info_t *p_sd_info = (sd_info_t *)p_info;

  HAL_SD_CardInfoTypeDef card_info;


  if (is_init == true)
  {
    HAL_SD_GetCardInfo(&uSdHandle, &card_info);

    p_sd_info->card_type          = card_info.CardType;
    p_sd_info->card_version       = card_info.CardVersion;
    p_sd_info->card_class         = card_info.Class;
    p_sd_info->rel_card_Add       = card_info.RelCardAdd;
    p_sd_info->block_numbers      = card_info.BlockNbr;
    p_sd_info->block_size         = card_info.BlockSize;
    p_sd_info->log_block_numbers  = card_info.LogBlockNbr;
    p_sd_info->log_block_size     = card_info.LogBlockSize;
    p_sd_info->card_size          =  (uint32_t)((uint64_t)p_sd_info->block_numbers * (uint64_t)p_sd_info->block_size / (uint64_t)1024 / (uint64_t)1024);
    ret = true;
  }

  return ret;
}

void HAL_SD_AbortCallback(SD_HandleTypeDef *hsd)
{
  printf("sd abort isr\n");
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  printf("sd tx isr\n");
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  printf("sd rx isr\n");
}

void SDIO_IRQHandler(void)
{
   HAL_SD_IRQHandler(&uSdHandle);
   printf("sd isr\n");
 }

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(sdHandle->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspInit 0 */

  /* USER CODE END SDIO_MspInit 0 */
    /* SDIO clock enable */
    __HAL_RCC_SDIO_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**SDIO GPIO Configuration
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10     ------> SDIO_D2
    PC11     ------> SDIO_D3
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN SDIO_MspInit 1 */

  /* USER CODE END SDIO_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspDeInit 0 */

  /* USER CODE END SDIO_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDIO_CLK_DISABLE();

    /**SDIO GPIO Configuration
    PC8     ------> SDIO_D0
    PC9     ------> SDIO_D1
    PC10     ------> SDIO_D2
    PC11     ------> SDIO_D3
    PC12     ------> SDIO_CK
    PD2     ------> SDIO_CMD
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

  /* USER CODE BEGIN SDIO_MspDeInit 1 */

  /* USER CODE END SDIO_MspDeInit 1 */
  }
}


#ifdef _USE_HW_CMDIF
void sdCmdifInit(void)
{
  if (cmdifIsInit() == false)
  {
    cmdifInit();
  }
  cmdifAdd("sd", sdCmdif);
}

void sdCmdif(void)
{
  bool ret = true;
  sd_info_t sd_info;


  if (cmdifGetParamCnt() == 1 && cmdifHasString("info", 0) == true)
  {
    cmdifPrintf("sd init      : %d\n", is_init);
    cmdifPrintf("sd connected : %d\n", sdIsDetected());

    if (is_init == true)
    {
      if (sdGetInfo(&sd_info) == true)
      {
        cmdifPrintf("  card_type            : %d\n", sd_info.card_type);
        cmdifPrintf("  card_version         : %d\n", sd_info.card_version);
        cmdifPrintf("  card_class           : %d\n", sd_info.card_class);
        cmdifPrintf("  rel_card_Add         : %d\n", sd_info.rel_card_Add);
        cmdifPrintf("  block_numbers        : %d\n", sd_info.block_numbers);
        cmdifPrintf("  block_size           : %d\n", sd_info.block_size);
        cmdifPrintf("  log_block_numbers    : %d\n", sd_info.log_block_numbers);
        cmdifPrintf("  log_block_size       : %d\n", sd_info.log_block_size);
        cmdifPrintf("  card_size            : %d MB, %d.%d GB\n", sd_info.card_size, sd_info.card_size/1024, ((sd_info.card_size * 10)/1024) % 10);
      }
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "sd info \n");
  }
}
#endif /* _USE_HW_CMDIF_SD */

#endif /* _USE_HW_SD */
