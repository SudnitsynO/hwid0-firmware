#include "graphic.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_i2c.h"
#include <stdint.h>
#include <tuple>

#define SET_DISPLAY_ON 0xAF
#define SET_INVERSE 0xA7
#define SET_ENTRIE_DISPLAY_OFF 0xA5

template <class RESET_PIN> class SSD1306 : public Graphic<bool>
{
  public:
    void init(I2C_HandleTypeDef &i2c_bus)
    {
        _bus = &i2c_bus;
        HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PinState::GPIO_PIN_RESET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PinState::GPIO_PIN_SET);
        HAL_Delay(200);
        send_cmd(0xAE); // Выключить дисплей

        // send_cmd(0xD5,0x80);

        send_cmd(0xA8, 0x3F); // Установить multiplex ratio
        // 1/64 duty (значение по умолчанию), 0x1F - 128x32, 0x3F - 128x64

        send_cmd(0xD3, 0x00); // Смещение дисплея (offset)
        // Нет смещения

        send_cmd(0x40); // Начала строки начала разверки 0x40 с начала RAM

        send_cmd(0x20, 0x00); // Режим автоматической адресации
        // 0-по горизонтали с переходом на новую страницу (строку)
        // 1 - по вертикали с переходом на новую строку
        // 2 - только по выбранной странице без перехода

        send_cmd(0xA1); // Режим разверки по странице (по X)
                        // A1 - нормальный режим (слева/направо) A0 - обратный (справа/налево)

        send_cmd(0xC8);       // Режим сканирования озу дисплея
                              // для изменения системы координат
                              // С0 - снизу/верх (начало нижний левый угол)
                              // С8 - сверху/вниз (начало верний левый угол)
        send_cmd(0xDA, 0x12); // Аппаратная конфигурация COM
                              // 0x02 - 128x32, 0x12 - 128x64

        send_cmd(0x81, 0xCF); // Установка яркости дисплея
        // 0x8F..0xCF

        // send_cmd(0xD9, 0xF1); // Настройка фаз DC/DC преоразователя
        // 0x22 - VCC подается извне / 0xF1 для внутренего

        // send_cmd(0xDB, 0x40); // Установка уровня VcomH
        // Влияет на яркость дисплея 0x00..0x70

        send_cmd(0xA4); // Режим нормальный

        send_cmd(0xA6); // 0xA6 - нет инверсии, 0xA7 - инверсия дисплея

        send_cmd(0xD5, 0x80); // Настройка частоты обновления дисплея
                              ///+----- делитель 0-F/ 0 - деление на 1
                              //+------ частота генератора. по умочанию 0x80
        send_cmd(0x8D, 0x14); // Управление внутреним преобразователем
        // 0x10 - отключить (VCC подается извне) 0x14 - запустить внутрений DC/DC

        send_cmd(0xAF); // Дисплей включен
    }
    void test()
    {
        /*
       for(int i = 0; i < 50; i++)
       {
           SetPixel(i, i, true);
       }
       */
       //Line(0, 0, 127, 63, true);
       Circle(64+8, 55, 7, true);
       Circle(64-8, 55, 7, true);
       Line(64-8, 49, 64-8, 20, true);
       Line(64+8, 49, 64+8, 20, true);
       Arc(64, 20, 8, 0, 180, true);
       Line(64, 12, 64, 20, true);
       Line(64-8, 22, 64+8, 22, true);
        UpdateDisplay();
        /*
                uint8_t packet[1025];
                for (int i = 0; i < 1025; i++)
                    packet[i] = 0xFF;
                packet[0] = 0x40;
                HAL_I2C_Master_Transmit(_bus, 0x3C << 1, packet, 1025, 1000);
                */
        // HAL_Delay(3000);
    }
    void send_cmd(uint8_t cmd)
    {
        uint8_t packet[2];
        packet[0] = 0x00;
        packet[1] = cmd;
        HAL_I2C_Master_Transmit(_bus, 0x3C << 1, packet, 2, 1000);
    }
    void send_cmd(uint8_t cmd, uint8_t extra_byte1)
    {
        uint8_t packet[2];
        packet[0] = 0x00;
        packet[1] = cmd;
        packet[2] = extra_byte1;
        HAL_I2C_Master_Transmit(_bus, 0x3C << 1, packet, 3, 1000);
    }
    void send_cmd(uint8_t cmd, uint8_t extra_byte1, uint8_t extra_byte2)
    {
        uint8_t packet[2];
        packet[0] = 0x00;
        packet[1] = cmd;
        packet[2] = extra_byte1;
        packet[1] = extra_byte2;
        HAL_I2C_Master_Transmit(_bus, 0x3C << 1, packet, 2, 1000);
    }
    void virtual UpdateDisplay()
    {
        VRAM[0] = 0x40;
        HAL_I2C_Master_Transmit(_bus, 0x3C << 1, VRAM, 1025, 1000);
    }
    virtual void SetPixel(int x, int y, bool Color)
    {
        if(x >= Width()) return;
        if(x < 0) return;
        if(y >= Height()) return;
        if(y < 0) return;

        const int line = y / 8;
        const int column = x;
        const int row = y % 8;
        const auto addr = VRAM + 1 + line * 128 + column;
        *addr = Color ? *addr | (1 << row) : *addr & ~(1 << row);
    }
    virtual bool GetPixel(int x, int y)
    {
        return false;
    }
    void virtual DrawText(int x, int y, bool Color, char *text)
    {
    }
    virtual void DrawText(const char *text, bool Color)
    {
    }
    virtual void Fill(int x1, int y1, int x2, int y2, bool Color)
    {
    }
    virtual void Clear(bool color = 0)
    {
    }
    virtual void Invalidate(int x1, int y1, int x2, int y2)
    {
    }
    virtual void SetTextPos(int x, int y)
    {
    }
    virtual void DrawText(const char *text, uint16_t Color, Font<uint16_t> *font)
    {
    }
    virtual void DrawText(const char *text, uint16_t Color)
    {
    }
    virtual int Width()
    {
        return 128;
    }

    virtual int Height()
    {
        return 64;
    }

  private:
    I2C_HandleTypeDef *_bus;
    uint8_t VRAM[1025];
};