#ifndef _HEADSET_H_
#define _HEADSET_H_

/* Headset button */
struct headset_button {
    int adc_min;
    int adc_max;
    int code;
    const char* select;
};

struct headset_driver_data {
    byte detect_pin;
    byte button_pin;
    byte micbias_pin;
    int adc_threshold_headset_mic_detect;
    int hs_type;
    int btn_code;
    const char* btn_text;
    headset_button buttons[3];

    int (*headset_micbias)(int);
    void (*headset_detect_irq)(uint8_t, void (*userFunc)(void), int);
    void (*headset_button_irq)(uint8_t, void (*userFunc)(void), int); 
};

/* Headset type */
typedef enum headset_type {
    HEADSET_UNKNOWN,
    HEADSET_4POLE,
    HEADSET_3POLE,
    HEADSET_TYPE_ERROR = -1
} headset_type;

/* Button type */
typedef enum button_type {
    BUTTON_ERROR = -1,
    BUTTON_NONE = 0,
    BUTTON_SENDEND = 1,
    BUTTON_UP,
    BUTTON_DOWN,
} button_type;

#endif /* _HEADSET_H_ */
