// KSND Study project for Electric circuit analysis 2
// Programed by ykp74 2016.10.18
/*===========================================================================
                     INCLUDE FILES FOR MODULE
===========================================================================*/
#include <Servo.h>
#include <DS1302.h>
#include <TM1637.h>
#include <LiquidCrystal_I2C.h>
#ifndef FEATURE_DUE
#include "TimerOne.h"
#else
#include "DueTimer.h"
#endif

#include "headset.h"  /* Headset header */

/*===========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

===========================================================================*/
#define SERVO_OUT_PIN   4
Servo servo;

#define RTC_CE_PIN      10
#define RTC_IO_PIN      9
#define RTC_SCLK_PIN    8

DS1302 rtc(RTC_CE_PIN, RTC_IO_PIN, RTC_SCLK_PIN);

// 4 segments with TM1637 diaplay
#define TM1637_CLK      22
#define TM1637_DIO      23

TM1637 tm1637( TM1637_CLK, TM1637_DIO);
int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};

// set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C  lcd(0x3F, 16, 2);
char lcd_buffer[17][2] = {0,};

const char* ht_type_text[3] = {
    "(No)","(4P)","(3P)"
};

const char* btn_type_text[4] = {
    "BUTTON_NONE",
    "BUTTON_SENDEND",
    "BUTTON_DOWN",
    "BUTTON_UP"
};

#define DISP_TIME_UPDATE            0
#define DISP_HEADSET_TYPE_UPDATE    1
#define DISP_HEADSET_BUTTON_UPDATE  2
static int cnt = 0;

static headset_driver_data ht;
static int adc_value = 0;

static int state = 0;
static int pre_state = 0; 

/*===========================================================================
FUNCTION init_device

DESCRIPTION
  Init the device
DEPENDENCIES
  None
RETURN VALUE
  None
SIDE EFFECTS
  None
===========================================================================*/
void init_ksnd_project_device(void)
{
    //LCD Display
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("==KSND PROJECT==");
    lcd.setCursor(0, 1);
    lcd.print("System SW2 ykp74");
    delay(5000);
    lcd.clear();

    /* Servo Motor init */
    servo.attach(SERVO_OUT_PIN);

    //4pin segment display 
    //BRIGHT_TYPICAL = 2
    //BRIGHT_DARKEST = 0
    tm1637.set(BRIGHT_TYPICAL);
    tm1637.init();

    pinMode( 11, OUTPUT); //red

    Serial.println("init_device()!!");  //Error to upload to the MEGA with 
    //This is the bug on Mega Bootloader   Do not use '!!!' 
}

void rtc_get_current_time_update(void)
{
    static boolean clock_point = true;
    clock_point = !clock_point;
    
    Time t= rtc.getTime();

    TimeDisp[0] = t.hour / 10;
    TimeDisp[1] = t.hour % 10;

    TimeDisp[2] = t.min / 10;
    TimeDisp[3] = t.min % 10;

    tm1637.point(clock_point);
    tm1637.display(TimeDisp);

    if( clock_point == false ){
        return;
    }

    memset( lcd_buffer,0,sizeof(char)*17*2);

    lcd.setCursor(0, 0);

    sprintf( &lcd_buffer[0][0],"%4d/%2d/%2d  %4s", t.year,t.mon,t.date, ht_type_text[ht.hs_type]);
    lcd.print(&lcd_buffer[0][0]);

    sprintf( &lcd_buffer[0][1],"%2d:%2d:%2d", t.hour,t.min,t.sec);
    lcd.setCursor(0, 1);
    lcd.print(&lcd_buffer[0][1]);
}

void rtc_set_init_time()
{
    rtc.writeProtect(false);
    rtc.halt(false);
    Time t(2016,10,4,19,36,30,3);
    rtc.getTime(t);
}

void display_update_handler(int events)
{
    static unsigned long previous_time = 0;
    static boolean output = HIGH;
    unsigned long current_time = millis();

    if( pre_state != events ){
        lcd.clear();
        cnt = 0;
    }

    switch(events){
        case DISP_HEADSET_TYPE_UPDATE:
            if( current_time - previous_time >= 500 ){
                previous_time = current_time;

                if( ht.hs_type == HEADSET_4POLE ){
                    lcd.setCursor(0, 0);  lcd.print("HSET(4P) Plug in");
                } else if( ht.hs_type == HEADSET_3POLE ){
                    lcd.setCursor(0, 0);  lcd.print("HSET(3P) Plug in");
                } else {
                    lcd.setCursor(0, 0);  lcd.print("HSET Pluged Out!");
                }
                lcd.setCursor(0, 1);
                sprintf( &lcd_buffer[0][1],"adc_value = %d",adc_value);
                lcd.print(&lcd_buffer[0][1]);
                cnt++;
            }
            break;

        case DISP_HEADSET_BUTTON_UPDATE:
            if( current_time - previous_time >= 500 ){
                previous_time = current_time;

                if( ht.btn_code == BUTTON_SENDEND ){
                    lcd.setCursor(0, 0);  lcd.print("SEND_END Pressed");
                } else if( ht.btn_code == BUTTON_UP ){
                    lcd.setCursor(0, 0);  lcd.print("VOL_UP Pressed!!");
                } else if( ht.btn_code == BUTTON_DOWN ){
                    lcd.setCursor(0, 0);  lcd.print("VOL_DOWN Pressed");
                } else {
                    lcd.setCursor(0, 0);  lcd.print("Button Released!");
                }
                lcd.setCursor(0, 1);
                memset( lcd_buffer,0,sizeof(char)*16*2);
                sprintf( &lcd_buffer[0][1],"adc_value = %d ",adc_value);
                lcd.print(&lcd_buffer[0][1]);
                cnt++;
            }
            break;

        case DISP_TIME_UPDATE:
        default:
            if( current_time - previous_time >= 500 ){
                output = !output;
                digitalWrite( 11, output);  //GPIO  Output
                previous_time = current_time;
                rtc_get_current_time_update();
            }
            break;

    }
    pre_state = state; //Avoid build error

    /* Override time for display the information */
    if( cnt >= 5){
        state = DISP_TIME_UPDATE;
        cnt = 0;
    }
}

void display_update( int events, int time ){
    state = events;
    cnt = time;
}

/*===========================================================================
                     Headset Driver code
===========================================================================*/
#ifndef FEATURE_DUE 
#define work_queue(fn,t)  Timer1.initialize(t*1000); Timer1.attachInterrupt(fn)
#define work_clear()  Timer1.stop()
#else
#define work_queue(fn,t) Timer4.attachInterrupt(fn).start(t*1000)
#define work_clear() Timer4.stop()
#endif

static int gpio_detect_value_last = 0;
static int gpio_button_value_last = 0;


int headset_mic_bias(int enable ){
    if( enable == 1){
        digitalWrite(ht.micbias_pin, HIGH);
        return 1;
    } else {
        digitalWrite(ht.micbias_pin, LOW);
        return 0;
    }
}

void headset_detect_irq_enble(int enable){
    if( enable == 1 ){
        attachInterrupt(digitalPinToInterrupt(ht.detect_pin), headset_detect_irq_handler,CHANGE);
        work_clear();
    } else {
        detachInterrupt(digitalPinToInterrupt(ht.detect_pin));
    }
}

void headset_button_irq_enable(int enable){
    if( enable == 1 ){
        attachInterrupt(digitalPinToInterrupt(ht.button_pin), headset_button_irq_handler,CHANGE);
        work_clear();
    } else { 
        detachInterrupt(digitalPinToInterrupt(ht.button_pin));
    }
}

void headset_button_work_func(void)
{
    int i = 0;
    int gpio_button_value_current = 0;

     headset_button_irq_enable(1);

     gpio_button_value_current = digitalRead(ht.button_pin);
     if( gpio_button_value_current == gpio_button_value_last ){
        digitalWrite(12,!gpio_button_value_current);

        /* Get the adc value */
        adc_value = analogRead(A0); //read adc
        Serial.print("headset_button_work_func : adc_value = ");
        Serial.println(adc_value);

        if( gpio_button_value_current == 0 ){
            /* Pressed the headset Button */
            for(i=0;i < 3;i++){
                if( adc_value >= ht.buttons[i].adc_min 
                   && adc_value < ht.buttons[i].adc_max ){
                    ht.btn_text=  ht.buttons[i].select;
                    ht.btn_code = ht.buttons[i].code;
                }
            }
            Serial.print("##### Press Button = ");
            Serial.println( ht.btn_text);
            display_update( DISP_HEADSET_BUTTON_UPDATE, 0);
        } else {
            /* Released the headset Button */
            if( ht.btn_code == BUTTON_NONE ){
                Serial.println("##### Fake Release Btn #####");
                return;
            }
            Serial.print("##### Release Button = ");
            Serial.println( ht.btn_text);
            ht.btn_code = BUTTON_NONE; //Reset the key Code
            display_update( DISP_HEADSET_BUTTON_UPDATE, 0);
        }
     }
}

void headset_button_irq_handler(void)
{
    headset_button_irq_enable(0);

    gpio_button_value_last = digitalRead(ht.button_pin);

    //Serial.println("headset_button_irq_handler : ");
    work_queue( headset_button_work_func, 10 );
}

headset_type headset_type_detect( headset_driver_data* head_info )
{
    /* enable mic bias to headset */
    head_info->headset_micbias(1);
    /* Get the adc value */
    adc_value = analogRead(A0); //read adc
    Serial.print("headset_type_detect : adc_value = ");
    Serial.println(adc_value);

    if( adc_value >= head_info->adc_threshold_headset_mic_detect ){
        return HEADSET_4POLE;
    } else if( adc_value > 0 && adc_value < 100 ){
        return HEADSET_3POLE;
    } else {
        return HEADSET_UNKNOWN;
    }
}

void headset_detect_work_func(void)
{
    headset_type type = HEADSET_TYPE_ERROR;
        
    headset_detect_irq_enble(1);

    gpio_detect_value_last = digitalRead(ht.detect_pin);
    
    if( gpio_detect_value_last == LOW ){
        /* Connect Headset Device */
        type = headset_type_detect(&ht);

        switch(type){
            case HEADSET_4POLE:
                ht.hs_type = HEADSET_4POLE;
                display_update( DISP_HEADSET_TYPE_UPDATE, 0);

                Serial.println("##### headset(4P) plug in ######");
                delayMicroseconds(10000);
                headset_button_irq_enable(1);
                break;

            case HEADSET_3POLE:
                ht.hs_type = HEADSET_3POLE;
                display_update( DISP_HEADSET_TYPE_UPDATE, 0);

                Serial.println("##### headset(3P) plug in ######");
                /* disable mic bias to headset */
                ht.headset_micbias(0);
                break;

            default:
                break;
        }
    } else {
        /* disable mic bias to headset */
        ht.headset_micbias(0);

        if( ht.hs_type == HEADSET_UNKNOWN || ht.hs_type == HEADSET_TYPE_ERROR){
            return;
        }

        /* Disconnect Headset Device */
        ht.hs_type = HEADSET_UNKNOWN;
        display_update( DISP_HEADSET_TYPE_UPDATE, 0);

        Serial.println("#########  headset plug out !! #########");
    }
}

void headset_detect_irq_handler(void){
    headset_detect_irq_enble(0);
    headset_button_irq_enable(0);

    Serial.println("headset_detect_irq_handler : ");
    work_queue( headset_detect_work_func, 100 );
}

void headset_set_data(headset_driver_data* head_info)
{
    head_info->buttons[0].adc_max = 700;
    head_info->buttons[0].adc_min = 100;
    head_info->buttons[0].code = BUTTON_SENDEND;
    head_info->buttons[0].select = btn_type_text[BUTTON_SENDEND];
        
    head_info->buttons[1].adc_max = 900;
    head_info->buttons[1].adc_min = 701;
    head_info->buttons[1].code = BUTTON_DOWN;
    head_info->buttons[1].select = btn_type_text[BUTTON_DOWN];

    head_info->buttons[2].adc_max = 1024;
    head_info->buttons[2].adc_min = 901;
    head_info->buttons[2].code = BUTTON_UP;
    head_info->buttons[2].select = btn_type_text[BUTTON_UP];
   
    head_info->adc_threshold_headset_mic_detect = 1000;
}

void headset_init(headset_driver_data* head_info)
{
    head_info->detect_pin = 2;  //int 0
    head_info->button_pin = 3;  //int 1    //5 interrupt pin for MEGA
    head_info->micbias_pin = 7;
    head_info->headset_micbias = headset_mic_bias;
    head_info->hs_type = HEADSET_UNKNOWN;
    head_info->btn_code = BUTTON_NONE;

    pinMode(head_info->micbias_pin, OUTPUT);
    pinMode(12, OUTPUT); //green

    /* Set headset data */
    headset_set_data(head_info);

    attachInterrupt(digitalPinToInterrupt(head_info->button_pin),
                                            headset_button_irq_handler,CHANGE);
    attachInterrupt(digitalPinToInterrupt(head_info->detect_pin),
                                            headset_detect_irq_handler,CHANGE);

    /* Check the headset status in Boot */
    headset_detect_work_func();
}

/*===========================================================================
                     ADUINO MAIN ROUTINE
===========================================================================*/
void setup() 
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial3.begin(9600);

    init_ksnd_project_device();
    //rtc_set_init_time();
    //tone(11, 1000, 100);

    //interrupt test routine
    headset_init( &ht );
    
    Serial.println("setup()!!");
}

void loop() 
{
    // put your main code here, to run repeatedly:
    char input = 0;
    int input_0, input_3;

    display_update_handler( state );

    input_0 = Serial.available();
    input_3 = Serial3.available();

    if(input_0 || input_3 ){
        if( input_0 ){
            input = Serial.read();
        } else if( input_3 ){
            input = Serial3.read();
        } else {
            input = 0;
        }

        switch(input){
            case'0': servo.write(0);
                break;
            case'1': servo.write(90);
                break;
            case'2': servo.write(180);
                break;
            case'3': servo.write(255);
                break;
            case'T':
                break;
        }
        Serial.println("Servo Motor spin!! ");
    }
}
