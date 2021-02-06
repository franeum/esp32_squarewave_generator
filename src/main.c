#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
//#include "esp_log.h"
#include "esp_partition.h"
#include <driver/dac.h>
#include "driver/periph_ctrl.h"
//#include "driver/timer.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>


#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
//#include "esp_sleep.h"
//#include "sdkconfig.h"


uint8_t squarewave[] = {
    //128, 217, 255, 217, 128, 39, 1, 39  
    191, 63  
};

uint8_t scale[] = { 48, 50, 52, 53, 55, 57, 59 };


// inc = 1:     frq = (1000000 / 100 / 2) * 1 = 5000hz
// inc = 0.5:   frq = (1000000 / 100 / 2) * 0.5 = 2500hz
// inc? 2500hz / (1000000 / 100 / 2) = 0.5 => 2500 / 5000;
// 440hz = 440 / 5000 => inc = 0.088
float idx = 0.f;
static float increment = 0.088;

static float hz2inc(float hz);
static float mtof(int hz);
static void periodic_timer_callback(void* arg);
static void application_task(void* args);

//static const char* TAG = "example";


// 8.17579891564 * exp(.0577622650 * f);

static float hz2inc(float hz) {
    return hz / 10000.0;
}

static float mtof(int midi) {
    return 8.17579891564 * exp(.0577622650 * (double)(midi));
}

static void application_task(void* args)
{
    srand(time(NULL));
    /*
    while(1) {
        //ESP_LOGI(TAG, "application_task: running application task");
        for (uint8_t i=0; i<29; i++) {
            uint8_t base = scale[i % 7];
            uint8_t offset = 12 * (i / 7);
            base += offset;
            increment = hz2inc(mtof(base));
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    */
    while(1) {
        uint8_t i = (uint8_t)(rand() % 29);
        uint8_t base = scale[i % 7];
        uint8_t offset = 12 * (i / 7);
        base += offset;
        increment = hz2inc(mtof(base));
        vTaskDelay(40 / portTICK_PERIOD_MS);    
    }
}


void app_main(void)
{
    /* Create two timers:
     * 1. a periodic timer which will run every 0.5s, and print a message
     */

    dac_output_enable(DAC_CHANNEL_1);

    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "periodic"
    };

    esp_timer_handle_t periodic_timer;
    //ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    esp_timer_create(&periodic_timer_args, &periodic_timer);
    /* The timer has been created but is not running yet */

    

    /* Start the timers */
    //ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10));


    esp_timer_start_periodic(periodic_timer, 50); // max frequency: 1000000 / 20 / 8 = 6250
    xTaskCreate(application_task, "application_task", 2048, NULL, uxTaskPriorityGet(NULL), NULL);
}


static void periodic_timer_callback(void* arg)
{
    dac_output_voltage(DAC_CHANNEL_1, squarewave[(uint8_t)idx]);
    idx += increment;
    if (idx >= 2.0) idx = 0.0;
}