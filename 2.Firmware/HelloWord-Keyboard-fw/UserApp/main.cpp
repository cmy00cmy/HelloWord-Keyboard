#include "common_inc.h"
#include "configurations.h"
#include "HelloWord/hw_keyboard.h"
#include "../HelloWord/hw_keyboard.h"


/* Component Definitions -----------------------------------------------------*/
KeyboardConfig_t config;
HWKeyboard keyboard(&hspi1);


/* Main Entry ----------------------------------------------------------------*/
void Main()
{
    EEPROM eeprom;
    eeprom.Pull(0, config);
    if (config.configStatus != CONFIG_OK)
    {
        // Use default settings
        config = KeyboardConfig_t{
            .configStatus = CONFIG_OK,
            .serialNum=123,
            .keyMap={}
        };
        memset(config.keyMap, -1, 128);
        eeprom.Push(0, config);
    }

    // Keyboard Report Start
    HAL_TIM_Base_Start_IT(&htim4);


    while (true)
    {
        switch (keyboard.mode){
            case 0:
                break;
            case 1:
                /*---- This is a demo RGB effect ----*/
                static uint32_t t = 1;
                static bool fadeDir = true;

                fadeDir ? t++ : t--;
                if (t > 250) fadeDir = false;
                else if (t < 1) fadeDir = true;

                for (uint8_t i = 0; i < HWKeyboard::LED_NUMBER; i++)
                    keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{(uint8_t) t, 50, 0});
                /*-----------------------------------*/
                break;
            case 2:
                for (uint8_t i = 0; i < HWKeyboard::LED_NUMBER; i++)
                    keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{100, 100, 100});
                break;
            default:
                break;
        }





        // Send RGB buffers to LEDs
        keyboard.SyncLights();
    }
}

/* Event Callbacks -----------------------------------------------------------*/
extern "C" void OnTimerCallback() // 1000Hz callback
{
    keyboard.ScanKeyStates();  // Around 40us use 4MHz SPI clk
    keyboard.ApplyDebounceFilter(100);
    //keyboard.Remap(keyboard.FnPressed() ? 2 : 1);  // When Fn pressed use layer-2
    keyboard.Remap(1);  // When Fn pressed use layer-2
    if (keyboard.KeyPressed(HWKeyboard::FN))
    {
        if(keyboard.KeyPressed(HWKeyboard::BACKSPACE)){
            if(keyboard.mode > 1){
                keyboard.mode = 0;
                for (uint8_t i = 0; i < HWKeyboard::LED_NUMBER; i++){
                    keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{(uint8_t) 0, 0, 0});
                }
                keyboard.SyncLights();

            }
            else{
                keyboard.mode++;
            }
        }
        uint8_t rbg = keyboard.colorB + keyboard.colorG + keyboard.colorR;
        if(keyboard.KeyPressed(HWKeyboard::UP_ARROW)){
            static bool fadeDir = true;

            fadeDir ? rbg++ : rbg--;
            if (rbg > 750) fadeDir = false;
            else if (rbg < 1) fadeDir = true;

            keyboard.colorB = rbg % 250;
            keyboard.colorG = (rbg - keyboard.colorB <= 0 ? 0: rbg - keyboard.colorB) % 250;
            keyboard.colorR = rbg - keyboard.colorG < 0 ? 0 : rbg - keyboard.colorB - keyboard.colorG < 0;
            for (uint8_t i = 0; i < HWKeyboard::LED_NUMBER; i++){
                keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{(uint8_t) keyboard.colorR, keyboard.colorG, keyboard.colorB});
            }
            keyboard.SyncLights();

        }
    }
    /*if (keyboard.KeyPressed(HWKeyboard::LEFT_CTRL) &&
        keyboard.KeyPressed(HWKeyboard::A))
    {
        // do something...

        // or trigger some keys...
        keyboard.Press(HWKeyboard::DELETE);
    }*/

    /*----  ----*/
    keyboard.SetRgbBufferByID(keyboard.GetTouchBarState(), HWKeyboard::Color_t{250, 0, 0});

/*    if (keyboard.KeyPressed(HWKeyboard::VOLUME_UP))
    {
        keyboard.Press(HWKeyboard::VOLUME_UP);
    }
    if (keyboard.KeyPressed(HWKeyboard::VOLUME_DOWN))
    {
        keyboard.Press(HWKeyboard::VOLUME_DOWN);
    }*/

    // Report HID key states
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS,
                               keyboard.GetHidReportBuffer(1),
                               HWKeyboard::KEY_REPORT_SIZE);
}


extern "C"
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi)
{
    keyboard.isRgbTxBusy = false;
}

extern "C"
void HID_RxCpltCallback(uint8_t* _data)
{

}