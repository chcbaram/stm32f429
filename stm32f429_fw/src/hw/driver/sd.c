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
static volatile bool is_rx_done = false;
static volatile bool is_tx_done = false;
static SD_HandleTypeDef uSdHandle;
static DMA_HandleTypeDef hdma_sdio_rx;
static DMA_HandleTypeDef hdma_sdio_tx;


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
#if 1
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


  gpioInit();

  is_init = false;

  return ret;
}


bool sdReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  uint32_t pre_time;

#if 0
  if(HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    while(sdIsBusy() == true);
    ret = true;
  }
#else

  is_rx_done = false;
  if(HAL_SD_ReadBlocks_DMA(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {

    pre_time = millis();
    while(is_rx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    pre_time = millis();
    while(sdIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_rx_done = false;
        break;
      }
    }
    ret = is_rx_done;
  }
#endif

  return ret;
}

bool sdWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;
  uint32_t pre_time;

#if 0
  if(HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }
  else
  {
    printf("sd write fail 0x%X\n", uSdHandle.ErrorCode);
  }
#else
  is_tx_done = false;
  if(HAL_SD_WriteBlocks_DMA(&uSdHandle, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {
    pre_time = millis();
    while(is_tx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    pre_time = millis();
    while(sdIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_tx_done = false;
        break;
      }
    }
    ret = is_tx_done;
  }
#endif
  return ret;
}

bool sdEraseBlocks(uint32_t start_addr, uint32_t end_addr)
{
  bool ret = false;

  if(HAL_SD_Erase(&uSdHandle, start_addr, end_addr) == HAL_OK)
  {
    ret = true;
  }
  else
  {
    printf("sd erase fail\n");
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

bool sdIsReady(uint32_t timeout)
{
  uint32_t pre_time;

  pre_time = millis();

  while(millis() - pre_time < timeout)
  {
    if (sdIsBusy() == false)
    {
      return true;
    }
  }

  return false;
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
  //printf("sd tx isr\n");
  is_tx_done = true;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  //printf("sd rx isr\n");
  is_rx_done = true;
}

void SDIO_IRQHandler(void)
{
   HAL_SD_IRQHandler(&uSdHandle);
   //printf("sd isr\n");
 }

void DMA2_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_sdio_rx);
}

void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_sdio_tx);
  //printf("dma tx isr\n");
}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(sdHandle->Instance==SDIO)
  {
  /* USER CODE BEGIN SDIO_MspInit 0 */

    __HAL_RCC_DMA2_CLK_ENABLE();

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


    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);


    /* SDIO DMA Init */
    /* SDIO_RX Init */
    hdma_sdio_rx.Instance = DMA2_Stream3;
    hdma_sdio_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdio_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdio_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmarx,hdma_sdio_rx);



    /* DMA2_Stream3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);


    /* SDIO_TX Init */
    hdma_sdio_tx.Instance = DMA2_Stream6;
    hdma_sdio_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_sdio_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdio_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdio_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdio_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdio_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdio_tx.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_sdio_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdio_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdio_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdio_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdio_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmatx,hdma_sdio_tx);

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
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
  else if (cmdifGetParamCnt() == 1 && cmdifHasString("read", 0) == true)
  {
    uint8_t sd_buf[512];

    if (is_init == true)
    {
      if (sdReadBlocks(0, sd_buf, 1, 1000) == true)
      {
        for (int i=0; i<512; i++)
        {
          cmdifPrintf("%d : 0x%X\n", i, sd_buf[i]);
        }
      }
      else
      {
        cmdifPrintf("sd read fail\n");
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
    cmdifPrintf( "sd read \n");
  }
}
#endif /* _USE_HW_CMDIF_SD */

#endif /* _USE_HW_SD */
