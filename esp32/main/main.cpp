#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"

#include "lcd_com.h"
#include "lcd_lib.h"
#include "fontx.h"

#if CONFIG_INTERFACE_I2S
#define INTERFACE INTERFACE_I2S
#elif CONFIG_INTERFACE_GPIO
#define INTERFACE INTERFACE_GPIO
#elif CONFIG_INTERFACE_REG
#define INTERFACE INTERFACE_REG
#endif

#include "ili9341.h"
#define DRIVER "ILI9341"
#define INIT_FUNCTION(a, b, c, d, e) ili9341_lcdInit(a, b, c, d, e)

static const char *TAG = "MAIN";

extern "C"
void app_main(void)
{
	ESP_LOGI(TAG, "Initializing SPIFFS");
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 10,
		.format_if_mount_failed =true
	};

	// Use settings defined above toinitialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total,&used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
	} else {
		ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
	}

	//SPIFFS_Directory("/spiffs/");

	FontxFile fx16G[2];
	FontxFile fx24G[2];
	FontxFile fx32G[2];
	InitFontx(fx16G,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
	InitFontx(fx24G,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
	InitFontx(fx32G,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic

	TFT_t dev;
	lcd_interface_cfg(&dev, INTERFACE);

	INIT_FUNCTION(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

    // Multi Font Test
    uint16_t color;
    uint8_t ascii[40];
    uint16_t margin = 10;
    color = WHITE;
    lcdSetFontDirection(&dev, 0);
    int xoff = 0, yoff = 0;
    while (1)
    {
        uint16_t xpos = 0;
        uint16_t ypos = 15;
        int xd = 0;
        int yd = 1;
        if(CONFIG_WIDTH < CONFIG_HEIGHT) {
            lcdSetFontDirection(&dev, 1);
            xpos = (CONFIG_WIDTH-1)-16;
            ypos = 0;
            xd = 1;
            yd = 0;
        }
        printf("Offset %d\n", yoff);
        if (yoff == 0)
            lcdFillScreen(&dev, BLACK);
        strcpy((char *)ascii, "16Dot Gothic Font");
        lcdDrawString(&dev, fx16G, xpos + xoff, ypos + yoff, ascii, color);

        xpos = xpos - (24 * xd) - (margin * xd);
        ypos = ypos + (16 * yd) + (margin * yd);
        strcpy((char *)ascii, "24Dot Gothic Font");
        lcdDrawString(&dev, fx24G, xpos + xoff, ypos + yoff, ascii, color);

        xpos = xpos - (32 * xd) - (margin * xd);
        ypos = ypos + (24 * yd) + (margin * yd);
        if (CONFIG_WIDTH >= 240) {
            strcpy((char *)ascii, "32Dot Gothic Font");
            lcdDrawString(&dev, fx32G, xpos + xoff, ypos + yoff, ascii, color);
            xpos = xpos - (32 * xd) - (margin * xd);;
            ypos = ypos + (32 * yd) + (margin * yd);
        }
        --xoff;
        yoff += 3;
        if (yoff > 20)
            yoff = 0;
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
