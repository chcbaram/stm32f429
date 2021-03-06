/*
 * usb.c
 *
 *  Created on: 2018. 3. 16.
 *      Author: HanCheol Cho
 */


#include "usb.h"






static bool is_init = false;
static enum UsbMode is_usb_mode = USB_NON_MODE;

USBD_HandleTypeDef USBD_Device;
extern PCD_HandleTypeDef hpcd;


extern USBD_DescriptorsTypeDef MSC_Desc;
extern USBD_StorageTypeDef USBD_DISK_fops;


bool usbInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;


  __HAL_RCC_GPIOB_CLK_ENABLE();


  /* Configure DM DP Pins - For OTG */
  GPIO_InitStruct.Pin = (GPIO_PIN_14 | GPIO_PIN_15);
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);


  return true;
}

bool usbBegin(enum UsbMode usb_mode)
{
  is_init = true;

  if (usb_mode == USB_CDC_MODE)
  {
    /* Init Device Library */
    USBD_Init(&USBD_Device, &VCP_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

    /* Add CDC Interface Class */
    USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);

    is_usb_mode = USB_CDC_MODE;
  }
#if HW_USE_MSC == 1
  else if (usb_mode == USB_MSC_MODE)
  {
    /* Init Device Library */
    USBD_Init(&USBD_Device, &MSC_Desc, 0);

    /* Add Supported Class */
    USBD_RegisterClass(&USBD_Device, USBD_MSC_CLASS);

    /* Add Storage callbacks for MSC Class */
    USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops);

    /* Start Device Process */
    USBD_Start(&USBD_Device);


    is_usb_mode = USB_MSC_MODE;
  }
#endif
  else
  {
    is_init = false;
  }

  return is_init;
}

enum UsbMode usbGetMode(void)
{
  return is_usb_mode;
}

void usbDeInit(void)
{
  if (is_init == true)
  {
    USBD_DeInit(&USBD_Device);
  }
}

#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd);
}
