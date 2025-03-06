/* by honey the codewitch

Portions derived from GT911 by alex-code. Original license follows
MIT License

Copyright (c) 2021 alex-code

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#pragma once
#if __has_include(<Arduino.h>)
#include <Arduino.h>
#include <Wire.h>
namespace arduino {
#else
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <esp_idf_version.h>
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include <driver/i2c_master.h>
#else
#include "driver/i2c.h"
#endif
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
namespace esp_idf {
#endif

template <int16_t PinRst = -1, uint16_t Width = 480, uint16_t Height = 272,uint8_t TouchCount=5, uint8_t Address = 0x5D>
class gt911 {
    static const uint8_t default_config[];
    //
    static constexpr const uint8_t GOODIX_ADDRESS	  		= 0x5D;
    static constexpr const size_t GOODIX_CONFIG_SIZE = 186;
    // Write only registers
    static constexpr const uint16_t GOODIX_REG_COMMAND        = 0x8040;
    static constexpr const uint16_t GOODIX_REG_ESD_CHECK  	  = 0x8041;
    static constexpr const uint16_t GOODIX_REG_PROXIMITY_EN   = 0x8042;

    // Read/write registers
    // The version number of the configuration file
    static constexpr const uint16_t GOODIX_REG_CONFIG_DATA  = 0x8047;
    // X output maximum value (LSB 2 bytes)
    static constexpr const uint16_t GOODIX_REG_MAX_X        = 0x8048;
    // Y output maximum value (LSB 2 bytes)
    static constexpr const uint16_t GOODIX_REG_MAX_Y        = 0x804A;
    // Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
    static constexpr const uint16_t GOODIX_REG_MAX_TOUCH    = 0x804C;

    // Module switch 1
    // 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode */
    static constexpr const uint16_t GOODIX_REG_MOD_SW1      = 0x804D;
    // Module switch 2
    // 7:1 Reserved, 0 Touch key */
    static constexpr const uint16_t GOODIX_REG_MOD_SW2      = 0x804E;

    // Number of debuffs fingers press/release
    static constexpr const uint16_t GOODIX_REG_SHAKE_CNT    = 0x804F;

    // X threshold
    static constexpr const uint16_t GOODIX_REG_X_THRESHOLD  = 0x8057;

    //Configuration update fresh
    static constexpr const uint16_t GOODIX_REG_CONFIG_FRESH = 0x8100;

    // ReadOnly registers (device and coordinates info)
    // Product ID (LSB 4 bytes, GT9110: = 0x06 = 0x00 = 0x00 = 0x09)
    static constexpr const uint16_t GOODIX_REG_ID           = 0x8140;
    // Firmware version (LSB 2 bytes)
    static constexpr const uint16_t GOODIX_REG_FW_VER       = 0x8144;

    // Current output X resolution (LSB 2 bytes)
    static constexpr const uint16_t GOODIX_READ_X_RES       = 0x8146;
    // Current output Y resolution (LSB 2 bytes)
    static constexpr const uint16_t GOODIX_READ_Y_RES       = 0x8148;
    // Module vendor ID
    static constexpr const uint16_t GOODIX_READ_VENDOR_ID   = 0x814A;

    static constexpr const uint16_t GOODIX_READ_COORD_ADDR  = 0x814E;

    static constexpr const uint16_t GOODIX_POINT1_X_ADDR 	= 0x8150;
    static constexpr const uint16_t GOODIX_POINT1_Y_ADDR 	= 0x8152;


    /* Commands for REG_COMMAND */
    //0: read coordinate state
    static constexpr const uint8_t GOODIX_CMD_READ         = 0x00;
    // 1: difference value original value
    static constexpr const uint8_t GOODIX_CMD_DIFFVAL      = 0x01;
    // 2: software reset
    static constexpr const uint8_t GOODIX_CMD_SOFTRESET    = 0x02;
    // 3: Baseline update
    static constexpr const uint8_t GOODIX_CMD_BASEUPDATE   = 0x03;
    // 4: Benchmark calibration
    static constexpr const uint8_t GOODIX_CMD_CALIBRATE    = 0x04;
    // 5: Off screen (send other invalid)
    static constexpr const uint8_t GOODIX_CMD_SCREEN_OFF   = 0x05;

   public:
    using type = gt911;
    static constexpr const uint8_t default_address = GOODIX_ADDRESS;
    static constexpr const uint8_t alt_address = 0x14;
    static constexpr const size_t locations_size = (size_t)TouchCount;
    constexpr static const int16_t pin_rst = PinRst;
    constexpr static const uint8_t address = Address;
    constexpr static const uint16_t fixed_native_width = Width * (Height!=0);
    constexpr static const uint16_t fixed_native_height = Height * (Width!=0);
    struct point {
        uint16_t x;
        uint16_t y;
        point() {}
        point(uint16_t x, uint16_t y) : x(x),y(y) {

        }
        point(const point& rhs) : x(rhs.x), y(rhs.y) {

        }
        bool operator==(const point& rhs) const {
            return x==rhs.x && y==rhs.y ;
        }
        bool operator!=(const point& rhs) const {
            return x!=rhs.x || y!=rhs.y;
        }
        point& operator=(const point& rhs) {
            x=rhs.x;
            y=rhs.y;
            return *this;
        }
    };

   private:

#ifdef ARDUINO
    TwoWire& m_i2c;
#else
    constexpr static const uint8_t ACK_CHECK_EN = 0x1;
    constexpr static const uint8_t ACK_CHECK_DIS = 0x0;
    constexpr static const uint8_t ACK_VAL = 0x0;
    constexpr static const uint8_t NACK_VAL = 0x1;
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    i2c_master_bus_handle_t m_i2c_bus;
    i2c_master_dev_handle_t m_i2c;
#else
    i2c_port_t m_i2c;
#endif
#endif
    bool m_initialized;
    uint16_t X_Resolution;
	uint16_t Y_Resolution;
	uint8_t Number_Of_Touch_Support;
	bool SoftwareNoiseReduction;
    bool ReverseX;
	bool ReverseY;
	bool SwithX2Y;
    size_t m_locations_size;
    point m_locations[locations_size];
    uint8_t GT911_Config[GOODIX_CONFIG_SIZE];
    void i2c_start(uint16_t reg) {
        m_i2c.beginTransmission(address);
        m_i2c.write(reg >> 8);
        m_i2c.write(reg & 0xFF);
    }

    bool write(uint16_t reg, uint8_t data) {
        i2c_start(reg);
        m_i2c.write(data);
        return m_i2c.endTransmission() == 0;
    }

    uint8_t read(uint16_t reg) {
        i2c_start(reg);
        m_i2c.endTransmission();
        m_i2c.requestFrom(address, (uint8_t)1);
        while (m_i2c.available()) {
            return m_i2c.read();
        }
        return 0;
    }

    bool writeBytes(uint16_t reg, uint8_t* data, uint16_t size) {
        i2c_start(reg);
        for (uint16_t i = 0; i < size; i++) {
            m_i2c.write(data[i]);
        }
        return m_i2c.endTransmission() == 0;
    }

    bool readBytes(uint16_t reg, uint8_t* data, uint16_t size) {
        i2c_start(reg);
        m_i2c.endTransmission();

        uint16_t index = 0;
        while (index < size) {
            uint8_t req = _min(size - index, I2C_BUFFER_LENGTH);
            m_i2c.requestFrom(address, req);
            while (m_i2c.available()) {
                data[index++] = m_i2c.read();
            }
            index++;
        }

        return size == index - 1;
    }
    uint32_t product_id() {
        uint32_t result;
        readBytes(GOODIX_REG_ID,(uint8_t*)&result,4);
        return result;
    }
    void reflash() {
        checksum();
        writeBytes(GOODIX_REG_CONFIG_DATA,(uint8_t*)&GT911_Config,GOODIX_CONFIG_SIZE);
    }
    void checksum() {
        uint8_t ccsum = 0;
        for (uint8_t i = 0; i < GOODIX_CONFIG_SIZE; i++) {
            ccsum += GT911_Config[i];
        }

        ccsum = (~ccsum) + 1;
        GT911_Config[184]=ccsum;
    }
    void read_points() {
        uint8_t data[7];
        uint8_t id;
        uint16_t x, y, size;
        m_locations_size = 0;
        uint8_t status = read(GOODIX_READ_COORD_ADDR);
        if((status & 0x80)!=0) {
            m_locations_size = status & 0xF;
            if(m_locations_size!=0) {
                for (uint8_t i=0; i<m_locations_size; i++) {
                    readBytes(GOODIX_POINT1_X_ADDR + i * 8, data,6);
                    read_point(data,&m_locations[i]);
                }    
            }
        }
        
        write(GOODIX_READ_COORD_ADDR, 0);        
    }
    void read_point(const uint8_t* data, point* out_point) const {
        out_point->x = data[0] + (data[1] << 8);
        out_point->y = data[2] + (data[3] << 8);
    }
    void update_config() {
        GT911_Config[1] = X_Resolution & 0x00FF;
        GT911_Config[2] = (X_Resolution >> 8) & 0x00FF;
        //	Set Y resolution
        GT911_Config[3] = Y_Resolution & 0x00FF;
        GT911_Config[4] = (Y_Resolution >> 8) & 0x00FF;
        //  Set touch number
        GT911_Config[5] = Number_Of_Touch_Support;
        //  set reverse Y
        GT911_Config[6] = 0;
        GT911_Config[6] |= ReverseY << 7;
        //  set reverse X
        GT911_Config[6] |= ReverseX << 6;
        //  set switch X2Y
        GT911_Config[6] |= SwithX2Y << 3;
        //  set Sito
        GT911_Config[6] |= SoftwareNoiseReduction << 2;
    }
    gt911(const gt911& rhs) = delete;
    gt911& operator=(const gt911& rhs) = delete;
    void do_move(gt911& rhs) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        m_i2c_bus = rhs.m_i2c_bus;
#endif        
        m_i2c = rhs.m_i2c;
        X_Resolution = rhs.X_Resolution;
        Y_Resolution = rhs.Y_Resolution;
        Number_Of_Touch_Support = rhs.Number_Of_Touch_Support;
        SwithX2Y=rhs.SwithX2Y;
        ReverseX=rhs.ReverseX;
        ReverseY=rhs.ReverseY;
        SoftwareNoiseReduction = rhs.SoftwareNoiseReduction;
        memcpy(m_locations, rhs.m_locations, m_locations_size * sizeof(point));
        memcpy(GT911_Config,rhs.GT911_Config,sizeof(GT911_Config));
    }
    
   public:
    gt911(
#ifdef ARDUINO
        TwoWire& i2c = Wire
#else
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        i2c_master_bus_handle_t i2c
#else
        i2c_port_t i2c = I2C_NUM_0
#endif
#endif
    ) : 
#if defined(ARDUINO) || ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        m_i2c(i2c), 
#else
        m_i2c_bus(i2c),
        m_i2c(nullptr),
#endif
    m_initialized(false),
     X_Resolution(fixed_native_width),Y_Resolution(fixed_native_height), Number_Of_Touch_Support(locations_size),SoftwareNoiseReduction(true),ReverseX(false),ReverseY(false),SwithX2Y(false), m_locations_size(0), GT911_Config() // 0x80FC - 0x8100) 
    {
    }
    gt911(gt911&& rhs) {
        do_move(rhs);
    }
    gt911& operator=(gt911&& rhs) {
        do_move(rhs);
        return *this;
    }
    bool initialized() const { return m_initialized; }
    bool initialize() {
        if (!m_initialized) {
            m_locations_size = 0;
            m_i2c.begin();
            memcpy(GT911_Config,default_config,sizeof(default_config));
            update_config();
            reset();
            uint32_t pid = product_id();
            if(pid==0) {
                return false;
            }
            
            reflash();
            write(GOODIX_REG_COMMAND,0);
            m_initialized = true;
        }
        return m_initialized;
    }
    uint16_t native_width() const {
        if(!initialized()) {
            return 0;
        }
        return X_Resolution;
    }
    uint16_t native_height() const {
        if(!initialized()) {
            return 0;
        }
        return Y_Resolution;
    }
    uint16_t width() const {
        return (SwithX2Y)?native_height():native_width();
    }
    uint16_t height() const {
        return (SwithX2Y)?native_width():native_height();
    }
    int8_t rotation() const { 
        if(SwithX2Y) {
            if(ReverseY && !ReverseX) {
                return 1;
            } else if(ReverseX && !ReverseY) {
                return 3;
            }
        } else {
            if(ReverseX && ReverseY) {
                return 2;
            } else if(!ReverseX && !ReverseY) {
                return 0;
            }
        }
        return -1;
    }
    void rotation(uint8_t value) {
        switch(value) {
            case 1:
                SwithX2Y=true;
                ReverseY=true;
                ReverseX=false;
            break;
            case 2:
                SwithX2Y=false;
                ReverseY=true;
                ReverseX=true;
            break;
            case 3:
                SwithX2Y=true;
                ReverseX=true;
                ReverseY=false;
            break;
            default: // 0
                SwithX2Y=false;
                ReverseX=false;
                ReverseY=false;
            break;
        }
        update_config();
        reflash();
    }
    
    bool xy(uint16_t* out_x,uint16_t* out_y) const {
        return xy(0,out_x,out_y);
    }
    bool xy2(uint16_t* out_x,uint16_t* out_y) const {
        return xy(1,out_x,out_y);
    }
    bool xy(int index,uint16_t* out_x,uint16_t* out_y) const {
        if(m_locations_size>index) {
            if(out_x) {
                *out_x = m_locations[index].x;
            }
            if(out_y) {
                *out_y = m_locations[index].y;
            }
            return true;
        }
        return false;
    }
    
    void reset() {
        if (pin_rst > -1) {
            pinMode(pin_rst, OUTPUT);
            delay(1);
            digitalWrite(pin_rst, LOW);
        
            delay(12);
        
            pinMode(pin_rst, INPUT);
    
            delay(6);
            delay(51);
        }
    }
    void update() {
        if (!initialized()) {
            return;
        }
        read_points();
    }
};
template <int16_t PinRst , uint16_t Width, uint16_t Height ,uint8_t TouchCount, uint8_t Address>
const uint8_t gt911<PinRst,Width,Height,TouchCount,Address>::default_config[] =
{
		0x81, 0x00, 0x04, 0x58, 0x02, 0x0A, 0x0C, 0x20, 0x01, 0x08, 0x28, 0x05, 0x50, // 0x8047 - 0x8053
		0x3C, 0x0F, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8054 - 0x8060
		0x00, 0x89, 0x2A, 0x0B, 0x2D, 0x2B, 0x0F, 0x0A, 0x00, 0x00, 0x01, 0xA9, 0x03, // 0x8061 - 0x806D
		0x2D, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, // 0x806E - 0x807A
		0x59, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, 0x93, 0x24, 0x00, 0x7D, 0x2C, // 0x807B - 0x8087
		0x00, 0x6B, 0x36, 0x00, 0x5D, 0x42, 0x00, 0x53, 0x50, 0x00, 0x53, 0x00, 0x00, // 0x8088	- 0x8094
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8095 - 0x80A1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A2 - 0x80AD
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, // 0x80AE - 0x80BA
		0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // 0x80BB - 0x80C7
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80C8 - 0x80D4
		0x02, 0x04, 0x06, 0x08, 0x0A, 0x0F, 0x10, 0x12, 0x16, 0x18, 0x1C, 0x1D, 0x1E, // 0x80D5 - 0x80E1
		0x1F, 0x20, 0x21, 0x22, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, // 0x80E2 - 0x80EE
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80EF - 0x80FB
		0x00, 0x00, 0xD6, 0x01 };
}  // namespace