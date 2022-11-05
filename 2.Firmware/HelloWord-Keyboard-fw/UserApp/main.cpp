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
                for (uint8_t i = 0; i < HWKeyboard::LED_NUMBER; i++)
                    keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{0, 0, 0});
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
                    keyboard.SetRgbBufferByID(i, HWKeyboard::Color_t{keyboard.color[0], keyboard.color[1], keyboard.color[2]});
                break;
            default:
                break;
        }





        // Send RGB buffers to LEDs
        keyboard.SyncLights();
    }
}

HWKeyboard::KeyCode_t lastKeyCode = HWKeyboard::RESERVED;
uint8_t lastTouchId = -1;
/* Event Callbacks -----------------------------------------------------------*/
extern "C" void OnTimerCallback() // 1000Hz callback
{
    keyboard.ScanKeyStates();  // Around 40us use 4MHz SPI clk
    keyboard.ApplyDebounceFilter(100);
    //keyboard.Remap(keyboard.FnPressed() ? 2 : 1);  // When Fn pressed use layer-2
    keyboard.Remap(1);
    if (keyboard.KeyPressed(HWKeyboard::FN))
    {
        keyboard.Release(HWKeyboard::FN);
        //避免重复操作
        if(!keyboard.KeyPressed(lastKeyCode))
        {
            lastKeyCode = HWKeyboard::RESERVED;
        }
        else
        {
            keyboard.Release(lastKeyCode);
        }

        //切换灯光模式
        if(lastKeyCode == HWKeyboard::RESERVED && keyboard.KeyPressed(HWKeyboard::BACKSPACE)){
            lastKeyCode = HWKeyboard::BACKSPACE;
            if(keyboard.mode > 1){
                keyboard.mode = 0;
            }
            else{
                keyboard.mode++;
            }
            keyboard.Release(HWKeyboard::BACKSPACE);
            lastKeyCode = HWKeyboard::BACKSPACE;
        }

        //mode-2 修改颜色
        int rgb = keyboard.color[0] + keyboard.color[1] + keyboard.color[2];
        if(keyboard.mode == 2 && keyboard.KeyPressed(HWKeyboard::UP_ARROW)){
            static bool fadeDir = true;

            fadeDir ? rgb++ : rgb--;
            if (rgb >= 750) fadeDir = false;
            else if (rgb < 1) fadeDir = true;

            int tmp = rgb;
            memset(keyboard.color, 0, 3);
            for (int i = 0; i <= rgb / 250; i++)
            {
                keyboard.color[i] = tmp / 250 >= 1 ? 250 : tmp % 250;
                tmp -= keyboard.color[i];
            }
            keyboard.Release(HWKeyboard::UP_ARROW);
        }
    }

    if(keyboard.GetTouchBarState() > 0)
    {
//        uint8_t touchId = (uint8_t) pow((double) keyboard.GetTouchBarState(), (double) 1.0 / 2);
//        if(lastTouchId >= 0)
//        {
//            touchId > lastTouchId ? keyboard.KeyPressed(HWKeyboard::RIGHT_ARROW) : keyboard.KeyPressed(HWKeyboard::LEFT_ARROW);
//
//        }
//        lastTouchId = touchId;
//        //keyboard.SetRgbBufferByID(touchId, HWKeyboard::Color_t{250, 0, 0});
//        if(keyboard.GetTouchBarState((uint8_t) 4) > 0)
//            keyboard.SetRgbBufferByID(touchId, HWKeyboard::Color_t{250, 0, 0});

    }
    /*if (keyboard.KeyPressed(HWKeyboard::LEFT_CTRL) &&
        keyboard.KeyPressed(HWKeyboard::A))
    {
        // do something...

        // or trigger some keys...
        keyboard.Press(HWKeyboard::DELETE);
    }*/

    /*----  ----*/

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