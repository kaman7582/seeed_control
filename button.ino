/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

//void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);
//digitalPinToInterrupt(pin)

#include <Seeed_Arduino_FreeRTOS.h>
#include"TFT_eSPI.h"
#include "my_font.h" 

#define SEEED_LCD

TFT_eSPI tft = TFT_eSPI();

#define MAX_X 240
#define MAX_y 320
#define MAX_NAME   8
#define AMP_MAX    9
#define FEQ_MAX    5
#define X_ITEM_MAX 4
#define Y_ITEM_MAX 10
#define Y_START    10
#define MAX_BTN_NUM  2

enum key_state
{
    key_pressed = 1,
    key_release
};

enum btn_key_define
{
    btn_up,
    btn_down,
    btn_max
};

typedef struct cur_pos
{
    int col_pos;
    int row_pos;
}cur_pos;

typedef struct btn_key_ctrl
{
    int key_state;
    int pin_num;
    void (*btn_action)(void);
    cur_pos c_pos;
}btn_key_ctrl;


typedef struct item_info
{
    char item_name[MAX_NAME];
}item_info;

typedef struct menu_colums
{
    item_info *item_list;
    int items_cnt;
    char m_name[MAX_NAME];
}menu_colums;

void move_up(void);
void move_down(void);

item_info amp_items[AMP_MAX]={{"5pC"},{"10pC"},{"20pC"},{"50pC"},{"100pC"},{"200pC"},{"500pC"},{"1000pC"},{"2000pC"}};
item_info feq_items[FEQ_MAX] = {{"50HZ"},{"100HZ"},{"200HZ"},{"500HZ"},{"1000HZ"}};
btn_key_ctrl btn_ctrl[MAX_BTN_NUM]={{key_release,WIO_KEY_A,move_up,{0,0}},{key_release,WIO_KEY_B,move_down,{0,0}}};

menu_colums  m_colums[]={
  {amp_items,AMP_MAX,"AMP"},
  {feq_items,FEQ_MAX,"FEQ"}
};

typedef struct pos_list
{
  int xray_list[X_ITEM_MAX];
  int yray_list[Y_ITEM_MAX];
  int x_members;
  int y_members;
}pos_list;

typedef struct user_menu
{
    menu_colums *p_colms;
    int col_cnt;
    pos_list items_pos;
}user_menu;

user_menu u_menu;

int init_user_menu(void)
{
    int col_cnt = sizeof(m_colums)/sizeof(menu_colums);
    int i = 0,x_gap = 0,y_gap = 0;
    int x_offset= 0;
    if(col_cnt == 0)
      return -1;
    //we must find out the right x pos for all the columns
    u_menu.col_cnt = col_cnt;
    u_menu.items_pos.x_members = col_cnt;
    u_menu.items_pos.y_members = Y_ITEM_MAX;
    u_menu.p_colms = m_colums;
    x_gap = (int)((MAX_X)/col_cnt);
    x_offset = (int)(x_gap/3);
    for(; i < col_cnt; i++ )
    {
        u_menu.items_pos.xray_list[i] = i*x_gap+x_offset;
    }
    y_gap = (int)(MAX_y/Y_ITEM_MAX);
    //we use fix y items, which is 10 rows
    for(i = 0; i < Y_ITEM_MAX ; i++)
    {
         u_menu.items_pos.yray_list[i] = Y_START+(i+1)*y_gap;
    }
    return 0;
}

typedef void (*draw_string)(char * in_str,int x, int y);

void ui_draw_string(char * in_str,int x, int y)
{
  #ifdef SEEED_LCD
    //tft.setFreeFont(FF1);
    tft.setTextSize(2);
    tft.drawString(in_str,x, y);
    //tft.setCursor(x, y); 
    //tft.print(in_str);
  #endif
}


void display_user_menu(user_menu *usr_m)
{
    int x_cnt = usr_m->col_cnt;
    int i =0,j;
    int x,y;
    for(;i < x_cnt ; i++)
    {
       x= usr_m->items_pos.xray_list[i];
       ui_draw_string(usr_m->p_colms[i].m_name,x, Y_START);
       for(j =0 ; j < usr_m->p_colms[i].items_cnt ; j++)
       {
           int tmpy = u_menu.items_pos.yray_list[j];
           ui_draw_string(usr_m->p_colms[i].item_list[j].item_name,x, tmpy);
       }
    }
}

//button pressed and release

void button_init()
{
    //pinMode(WIO_KEY_A, INPUT_PULLUP);
    //pinMode(WIO_KEY_B, INPUT_PULLUP);
    //pinMode(WIO_KEY_C, INPUT_PULLUP);
    int i = 0;
    for(; i < MAX_BTN_NUM ;i++)
    {
        pinMode(btn_ctrl[i].pin_num, INPUT_PULLUP);
    }
}

void move_up(void)
{
    #ifdef SEEED_LCD
    Serial.println("A Key pressed");
    #endif
}

void move_down(void)
{
    #ifdef SEEED_LCD
    Serial.println("B Key pressed");
    #endif
}

void button_detect()
{
  //for(; i < MAX_BTN_NUM ;i++)
  //A PRESSED B PRESSED == LOW
  #if 0
  int up_btn = digitalRead(WIO_KEY_A);
  int down_btn = digitalRead(WIO_KEY_B);
  if(up_btn == LOW)
  {
      if(btn_ctrl[btn_up].key_state == key_release)
          btn_ctrl[btn_up].key_state = key_pressed;
      /*add some delay*/
  }
  else if (up_btn == HIGH)
  {
    /* code */
      if(btn_ctrl[btn_up].key_state == key_pressed)
      {
          btn_ctrl[btn_up].key_state = key_release;
          move_up();
      }

  }
  #endif
for(int i = 0; i < MAX_BTN_NUM ;i++)
   {
      int io_state = digitalRead(btn_ctrl[i].pin_num);

      switch(io_state)
      {
          case LOW:
            {
                if(btn_ctrl[i].key_state == key_release)
                    btn_ctrl[i].key_state = key_pressed;
            }
            break;
           case HIGH:
            {
                if(btn_ctrl[i].key_state == key_pressed)
                {
                    btn_ctrl[i].key_state = key_release;
                    btn_ctrl[i].btn_action();
                    delay(500);
                }
            }
      }
   }
}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(115200);
    tft.begin();
    
    int xpos =  0;
    int ypos = 40;

    button_init();
    init_user_menu();
    display_user_menu(&u_menu);
    
}

// the loop function runs over and over again forever
void loop() {
    button_detect();
}