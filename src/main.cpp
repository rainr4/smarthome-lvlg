#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>  // Include for rounding
#include <time.h>  // Include for time formatting
#define LCD_IMPLEMENTATION
#include "lcd_init.h"
#include <lvgl.h>
#include <lvgl_sd_fs.hpp>
#include <esp_i2c.hpp>
#include <gt911.hpp>
#include <ui.h>
#include <base64.h>

#define SD_SCK  12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS   10

#define I2C_SDA 17
#define I2C_SCL 18

#define TOUCH_INT -1
#define TOUCH_RST 38
#define TOUCH_WIDTH 800
#define TOUCH_HEIGHT 480

using namespace arduino;

const char* ssid = "david_iphone";
const char* password = "sausage222";
char lvglssid[100]; 
char lvglpassword[100];

const char* iftttWebhookUrl = "https://maker.ifttt.com/trigger/sensor_alert/with/key/Mkb2mg2bJuCJgkK8lQJBs";

// Define UI objects
lv_obj_t * ui_LRTEMPBAR;
lv_obj_t * ui_LRHUMBAR;
lv_obj_t * ui_LIVINGROOMLABEL;
lv_obj_t * ui_BRTEMPBAR;
lv_obj_t * ui_BRHUMBAR;
lv_obj_t * ui_BEDROOMLABEL;
lv_obj_t * ui_DFLEVELBAR;
lv_obj_t * ui_CONDITIONLABEL;  
lv_obj_t * ui_DATELABEL;      
lv_obj_t * ui_nowifi;      
lv_obj_t *current_screen = nullptr; // Variable to store the current screen

gt911<TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT> touch(Wire);

static uint8_t* lcd_transfer_buffer;
static lv_disp_drv_t disp_drv;

void lvgl_port_tp_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    uint16_t x, y;
    static uint32_t skip_ts = 0;
    if (millis() > skip_ts + 13) {
        skip_ts = millis();
        touch.update();
    }

    if (touch.xy(&x, &y)) {
        printf("touch: (%d, %d)\n", (int)x, (int)y);
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
        return;
    }
    data->state = LV_INDEV_STATE_REL;
}

void lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    lcd_panel_draw_bitmap(area->x1, area->y1, area->x2, area->y2, color_p);
    lv_disp_flush_ready(&disp_drv);
}

static void lvgl_print(const char* str) {
    puts(str);
}

void onConnectButtonClicked(lv_event_t * e) {
    // Retrieve the WiFi credentials from the input fields
    const char* ssid = lv_textarea_get_text(ui_SSID);
    const char* password = lv_textarea_get_text(ui_PASSWORD);

    // Attempt to connect to the WiFi network
    WiFi.begin(ssid, password);

    // Display a message while attempting to connect
    lv_label_set_text(ui_SSIDLABEL, "Connecting...");

    // Wait for connection
    unsigned long startMillis = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMillis < 30000) {
        delay(500);
    }

    // Check if connected successfully
    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text(ui_SSIDLABEL, "Connected!");
        lv_disp_load_scr(ui_home); // Load the home screen
    } else {
        lv_label_set_text(ui_SSIDLABEL, "Failed to connect.");
    }
}

void sendIFTTTNotification() {
  HTTPClient http;
  http.begin(iftttWebhookUrl);
  
  // Send the request
  int httpResponseCode = http.POST("");
  
  if (httpResponseCode > 0) {
    Serial.println("IFTTT request sent successfully");
  } else {
    Serial.println("Error sending IFTTT request");
  }
  
  http.end();
}

// Function to update the dog food level bar
void updateDFLevelBar(float level) {
    if (level < 0) level = 0;
    if (level > 9.5) level = 9.5;
    
    int bar_value = round(9.5 - level);  // Higher level means more empty
    
    if (bar_value < 0) bar_value = 0;
    if (bar_value > 9.5) bar_value = 9.5;
    
    lv_bar_set_value(ui_DFLEVELBAR, bar_value, LV_ANIM_ON);

    // Calculate percentage
    int percentage = round((1 - (level / 9.5)) * 100);

    // Set the text to display the percentage
    char labelText[20];
    snprintf(labelText, sizeof(labelText), "DOG FOOD (%d%%)", percentage);
    lv_label_set_text(ui_DOGFOODLABEL, labelText);

    if (percentage <= 10) { 
        puts("DOG FOOD VERY LOW");
        lv_label_set_text(ui_DOGFOODLABEL, "DOG FOOD VERY LOW");
        sendIFTTTNotification();
    }
}

// Function to format and update date and time
void updateDateTime() {
    time_t now;
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        puts("Failed to obtain time");
        return;
    }

    char dateBuffer[12];  // Increased size to accommodate "MM/DD |"
    snprintf(dateBuffer, sizeof(dateBuffer), "%02d/%02d |", timeinfo.tm_mon + 1, timeinfo.tm_mday);
    lv_label_set_text(ui_DATELABEL, dateBuffer);

    char timeBuffer[11];  // Ensure size is sufficient for "HH:MM AM/PM"
    int hour = timeinfo.tm_hour;
    const char* ampm = (hour >= 12) ? "PM" : "AM";
    if (hour == 0) {
        hour = 12;  // Midnight
    } else if (hour > 12) {
        hour -= 12; // Convert to 12-hour format
    }
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d %s", hour, timeinfo.tm_min, ampm);
    lv_label_set_text(ui_TIMELABEL, timeBuffer);
}


void fetchWeatherData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String weatherApiKey = "a90b3a8da426e61d2577b6d8d969f3a5"; // Replace with your OpenWeatherMap API key
        String city = "76205"; // Replace with the desired city or use ZIP code
        String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + weatherApiKey;

        http.begin(url);
        int httpCode = http.GET();

        if (httpCode == 200) {
            String payload = http.getString();
            Serial.println(payload);

            DynamicJsonDocument doc(1024); // Use DynamicJsonDocument instead of StaticJsonDocument
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                // Extract weather condition from JSON
                String weatherCondition = doc["weather"][0]["description"].as<String>();
                // Update the weather label on the UI
                lv_label_set_text(ui_CONDITIONLABEL, weatherCondition.c_str());
                
                // Extract and format date and time from JSON or use NTP for real-time update
                // This example uses NTP for real-time updates
                updateDateTime(); // Update date and time
            } else {
                puts("JSON deserialization failed");
            }
        } else {
            Serial.printf("HTTP GET failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        puts("WiFi not connected");
    }
}

void fetchJSON() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("https://info.rainr.co/display_recent.php");
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println(payload);
            puts("Fetched JSON");

            DynamicJsonDocument doc(1024); // Use DynamicJsonDocument instead of StaticJsonDocument
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                const char* bedroom_sensor = doc["bedroom"]["sensor"];
                const char* bedroom_location = doc["bedroom"]["location"];
                float bedroom_temperature = doc["bedroom"]["temperature"];
                float bedroom_humidity = doc["bedroom"]["humidity"];

                const char* living_room_sensor = doc["living_room"]["sensor"];
                const char* living_room_location = doc["living_room"]["location"];
                float living_room_temperature = doc["living_room"]["temperature"];
                float living_room_humidity = doc["living_room"]["humidity"];

                const char* dog_food_sensor = doc["dog_food"]["sensor"];
                const char* dog_food_location = doc["dog_food"]["location"];
                float dog_food_level = doc["dog_food"]["level"];

                // Print values to console
                printf("Bedroom:\n Sensor: %s\n Location: %s\n Temperature: %.0f\n Humidity: %.0f\n",
                        bedroom_sensor, bedroom_location, round(bedroom_temperature), round(bedroom_humidity));
                printf("Living Room:\n Sensor: %s\n Location: %s\n Temperature: %.0f\n Humidity: %.0f\n",
                        living_room_sensor, living_room_location, round(living_room_temperature), round(living_room_humidity));
                printf("Dog Food:\n Sensor: %s\n Location: %s\n Level: %.2f\n",
                        dog_food_sensor, dog_food_location, dog_food_level);

                // Update LVGL UI for Living Room
                lv_bar_set_value(ui_LRTEMPBAR, round(living_room_temperature), LV_ANIM_ON);
                lv_bar_set_value(ui_LRHUMBAR, round(living_room_humidity), LV_ANIM_ON);
                char living_room_label_text[50];
                snprintf(living_room_label_text, sizeof(living_room_label_text), "Living Room: %.0f C", round(living_room_temperature));
                lv_label_set_text(ui_LIVINGROOMLABEL, living_room_label_text);

                // Update LVGL UI for Bedroom
                lv_bar_set_value(ui_BRTEMPBAR, round(bedroom_temperature), LV_ANIM_ON);
                lv_bar_set_value(ui_BRHUMBAR, round(bedroom_humidity), LV_ANIM_ON);
                char bedroom_label_text[50];
                snprintf(bedroom_label_text, sizeof(bedroom_label_text), "Bedroom: %.0f C", round(bedroom_temperature));
                lv_label_set_text(ui_BEDROOMLABEL, bedroom_label_text);

                // Update LVGL UI for Dog Food Bar
                updateDFLevelBar(dog_food_level);
                
                // Force LVGL to process the updates
                lv_task_handler();
            } else {
                printf("JSON deserialization failed: %s\n", error.c_str());
            }
        } else {
            printf("HTTP GET failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        puts("WiFi not connected\n");
    }
}

bool setupWiFi() {
    unsigned long startMillis = millis();
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - startMillis > 30000) { // 1 minute timeout
            puts("Failed to connect to WiFi within 1 minute");
            // Load the 'no WiFi' screen for user input
            lv_disp_load_scr(ui_nowifi);
            return false;
        }
    }
    puts("Connected to WiFi");
    return true;
}


void setup() {
    Serial.begin(115200);
    puts("Booted");
    lcd_panel_init();  
    Wire.begin(I2C_SDA, I2C_SCL, 400 * 1000);
    /*SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    SD.begin(SD_CS);*/
    if (!touch.initialize()) {
        puts("Touch init failure");
    }
    touch.rotation(0);
    Serial.printf("Touch size %dx%d\n", (int)touch.width(), (int)touch.height());

    lv_init();
    lv_log_register_print_cb(lvgl_print);

    /* Initialize LVGL buffers */
    static lv_disp_draw_buf_t draw_buf;
    lcd_transfer_buffer = (uint8_t*)ps_malloc(800 * 48 * 2);
    if (lcd_transfer_buffer == nullptr) {
        puts("Failed to allocate buffer memory");
        return;
    }
    lv_disp_draw_buf_init(&draw_buf, lcd_transfer_buffer, nullptr, 800 * 48);

    /* Initialize the display device */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    disp_drv.flush_cb = lvgl_port_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_port_tp_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    // Attach event handler to the Connect button
    lv_obj_add_event_cb(ui_CONNECTBUTTON, onConnectButtonClicked, LV_EVENT_CLICKED, NULL);

    setupWiFi();

    fetchJSON();
    
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // Adjust time zone offset

    fetchWeatherData();

}

void loop(void) {
    lv_task_handler();  // Handle LVGL tasks

    touch.update();
    uint16_t x,y;
    if(touch.xy(&x,&y)) {
        printf("(%d,%d)\n",(int)x,(int)y);
    }

    static unsigned long lastWeatherUpdate = 0;
    static unsigned long lastFetchUpdate = 0;
    static unsigned long lastWiFiCheck = 0;
    unsigned long currentMillis = millis();
    
    static bool wifiWasDisconnected = false; // Track WiFi disconnection status

    // Check WiFi connection every 30 seconds
    if (currentMillis - lastWiFiCheck >= 30000) {
        lastWiFiCheck = currentMillis;

        if (WiFi.status() != WL_CONNECTED) {
            if (!wifiWasDisconnected) {
                // Store the current screen and switch to 'no WiFi' screen
                current_screen = lv_disp_get_scr_act(NULL);
                lv_disp_load_scr(ui_nowifi);
                wifiWasDisconnected = true;
            }
            puts("WiFi disconnected, attempting to reconnect...");
            bool wifiConnected = setupWiFi();  // Attempt to reconnect to WiFi

            if (wifiConnected) {
                // WiFi reconnected
                if (current_screen) {
                    lv_disp_load_scr(current_screen); // Load the previous screen
                } else {
                    lv_disp_load_scr(ui_home); // Load home screen if no previous screen
                }
                wifiWasDisconnected = false;
            }
        } else {
            // WiFi is connected
            if (wifiWasDisconnected) {
                // WiFi was previously disconnected, so update UI
                if (current_screen) {
                    lv_disp_load_scr(current_screen); // Load the previous screen
                } else {
                    lv_disp_load_scr(ui_home); // Load home screen if no previous screen
                }
                wifiWasDisconnected = false;
            }
        }
    }

    // Update weather data every 5 minutes
    if (currentMillis - lastWeatherUpdate >= 300000) {
        if (WiFi.status() == WL_CONNECTED) {
            fetchWeatherData();
            puts("Fetched Weather Data\n");
        } else {
            lv_disp_load_scr(ui_nowifi); // Load 'no WiFi' screen if connection fails
        }
        lastWeatherUpdate = currentMillis;
    }

    // Fetch JSON data every 30 seconds
    if (currentMillis - lastFetchUpdate >= 30000) {
        if (WiFi.status() == WL_CONNECTED) {
            fetchJSON();
            updateDateTime();
        } else {
            lv_disp_load_scr(ui_nowifi); // Load 'no WiFi' screen if connection fails
        }
        lastFetchUpdate = currentMillis;
        puts("Fetched Sensor Data\n");        
    }

    // Allow LVGL to process tasks
    delay(30);  
}
