#include "display.h"

TFT_eSPI           tft;
static TFT_eSprite wifi_status_display(&tft);
static TFT_eSprite heartbeat_display(&tft);
static TFT_eSprite battery_status_display(&tft);

display_fonts_s Display_Fonts;

display_params_s   Display_Params;
Display_Rotation_e display_rotation = Display_Rotation_Portrait;

void wio_display_init(Display_Rotation_e rotation_i)
{
    tft.begin();
    tft.init();
    tft.setRotation(rotation_i);
    Display_Params.weight_height = 70;
    Display_Params.timer_height = 70;
    Display_Params.heartbeat_height = 30;
    Display_Params.wifi_status_height = 30;
    Display_Params.battery_status_height = 30;
    switch (rotation_i)
    {
    case Display_Rotation_Landscape:
        Display_Params.width = 320;
        Display_Params.height = 240;
        Display_Params.title_height = 50;
        Display_Params.weight_start_y = Display_Params.title_height;
        Display_Params.weight_x_pos = Display_Params.width - 40;
        Display_Params.timer_start_y = Display_Params.weight_start_y + Display_Params.weight_height;
        break;
    case Display_Rotation_Portrait:
        Display_Params.width = 240;
        Display_Params.height = 320;
        Display_Params.title_height = 60;
        Display_Params.weight_start_y = Display_Params.title_height + 20;
        Display_Params.weight_x_pos = Display_Params.width - 10;
        Display_Params.timer_start_y = Display_Params.weight_start_y + Display_Params.weight_height;
        break;
    }
    Display_Params.battery_status_start_y = Display_Params.height - Display_Params.battery_status_height - 1;
    Display_Params.wifi_status_start_y = Display_Params.battery_status_start_y - Display_Params.wifi_status_height;
    Display_Params.heartbeat_start_y = Display_Params.wifi_status_start_y - Display_Params.heartbeat_height;
    Display_Fonts.title_font = &FreeSansBold18pt7b;
    Display_Fonts.weigth_font = &FreeSansBold18pt7b;
    Display_Fonts.timer_font = &FreeSansBold18pt7b;
    Display_Fonts.status_font = &FreeSans9pt7b;
    tft.fillScreen(TFT_BLACK);
    heartbeat_display.createSprite(Display_Params.width, Display_Params.heartbeat_height);
    wifi_status_display.createSprite(Display_Params.width, Display_Params.wifi_status_height);
    battery_status_display.createSprite(Display_Params.width, Display_Params.battery_status_height);
    Serial.println("Display init completed.");
}

void wio_set_background(void)
{
    tft.fillScreen(TFT_WHITE);
    tft.fillRect(0, 0, Display_Params.width, Display_Params.title_height - 1, TFT_DARKGREEN);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(Display_Fonts.title_font);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM); // Middle-center
    tft.drawString("MQTT Client", (Display_Params.width / 2) - 1, (Display_Params.title_height / 2) - 2);
    Serial.println("Display set background completed.");
}

void wio_wifi_status_update(const char *pStatusMessage)
{
    char display_msg[26];
    snprintf(display_msg, 25, "Wifi: %s", pStatusMessage);
    wifi_status_display.fillSprite(TFT_LIGHTGREY);
    wifi_status_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    wifi_status_display.setFreeFont(Display_Fonts.status_font);
    wifi_status_display.setTextColor(TFT_BLACK);
    wifi_status_display.setTextDatum(MC_DATUM); // Middle-left
    wifi_status_display.drawString(display_msg, (Display_Params.width / 2) - 1, (Display_Params.wifi_status_height / 2) - 1);
    wifi_status_display.pushSprite(0, Display_Params.wifi_status_start_y);
}

void wio_heartbeat_update(const char *pStatusMessage)
{
    char display_msg[26];
    snprintf(display_msg, 25, "%s UTC", pStatusMessage);
    heartbeat_display.fillSprite(TFT_LIGHTGREY);
    heartbeat_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    heartbeat_display.setFreeFont(Display_Fonts.status_font);
    heartbeat_display.setTextColor(TFT_BLACK);
    heartbeat_display.setTextDatum(MC_DATUM); // Middle-left
    heartbeat_display.drawString(display_msg, (Display_Params.width / 2) - 1, (Display_Params.heartbeat_height / 2) - 1);
    heartbeat_display.pushSprite(0, Display_Params.heartbeat_start_y);
}


void wio_battery_status_update(void)
{
    char    wio_battery_status_msg[15];
    int32_t battery_state;
    battery_state = get_battery_state();
    if (battery_state >= 0)
    {
        snprintf(wio_battery_status_msg, 14, "Batt: %3d %%", (int)battery_state);
    }
    else
    {
        snprintf(wio_battery_status_msg, 14, "Batt: N/A");
    }
    battery_status_display.fillSprite(TFT_LIGHTGREY);
    battery_status_display.drawFastHLine(0, 0, Display_Params.width, TFT_BLUE);
    battery_status_display.setFreeFont(Display_Fonts.status_font);
    battery_status_display.setTextColor(TFT_BLACK);
    battery_status_display.setTextDatum(MC_DATUM); // Middle-center
    battery_status_display.drawString(wio_battery_status_msg, (Display_Params.width / 2) - 1,
                                      (Display_Params.battery_status_height / 2) - 1);
    battery_status_display.pushSprite(0, Display_Params.battery_status_start_y);
    Serial.println(wio_battery_status_msg);
}
