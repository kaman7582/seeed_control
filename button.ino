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
#define MAX_Y 320
#define MAX_NAME   8
#define AMP_MAX    9
#define FEQ_MAX    5
#define X_START    30
#define Y_START    70
#define MAX_BTN_NUM  2
#define  MAX_ENTRY 2
#define DEFAULT_FONT_SIZE  4
#define DEFAULT_AMP_IDX    4
#define DEFAULT_FREQ_IDX   3
#define TIME_OUT_MS        1500
enum key_state
{
    key_init = 1,
    key_pressed,
    key_release,
    both_pressed,
    both_release,
};

enum time_out_state
{
    not_timeout = 0,
    in_timeout ,
    out_timeout,
};

enum btn_key_define
{
    btn_up,
    btn_down,
    btn_max
};

typedef struct btn_key_ctrl
{
    char key_state;
    char pin_num;
    char key_volt;
    void (*btn_action)(void);
    void (*two_btn_action)(void);
}btn_key_ctrl;


typedef struct item_info
{
    char item_name[MAX_NAME];
}item_info;


typedef struct entry_pos_s
{
    int start_x;
    int start_y;
    int option_x;
}entry_pos_t;

typedef struct menu_colums
{
    item_info *item_list;
    char m_name[MAX_NAME];
    char items_cnt;
    char font_size;
    char item_idx;
    entry_pos_t entry_pos;
}menu_entry_t;

typedef struct timeout_ctrl_s
{
    int key_time;
    int key_triggle;
    int timeout_time;
}timeout_ctrl_t;

void move_up(void);
void move_down(void);
void two_click_action(void); 
item_info    amp_items[AMP_MAX]={{"5pC   "},{"10pC  "},{"20pC  "},{"50pC  "},{"100pC "},{"200pC "},{"500pC "},{"1000pC"},{"2000pC"}};
item_info    feq_items[FEQ_MAX] = {{"50HZ  "},{"100HZ "},{"200HZ "},{"500HZ "},{"1000HZ"}};
char blank_entry[MAX_NAME]={"      "};
btn_key_ctrl btn_ctrl[MAX_BTN_NUM]={{key_init,WIO_KEY_A,1,move_up},{key_init,WIO_KEY_B,1,move_down}};
int    cur_entry = 0;
char  btn_io_volt[MAX_BTN_NUM]={1,1};//default high voltatge, pressed -->0
timeout_ctrl_t tm_ctrl={0,0,TIME_OUT_MS};
menu_entry_t m_entries[MAX_ENTRY]={
  {amp_items,"AMP",AMP_MAX,DEFAULT_FONT_SIZE,DEFAULT_AMP_IDX,{X_START,Y_START,150}},
  {feq_items,"FEQ",FEQ_MAX,DEFAULT_FONT_SIZE,DEFAULT_FREQ_IDX,{X_START,Y_START+70,150}}
};

#define SET_BTN_VOLT(btn,state)     btn_ctrl[btn].key_volt = state
#define BOTH_KEY_PRESSED()          (btn_ctrl[0].key_volt == 0 && btn_ctrl[1].key_volt == 0)
#define BOTH_KEY_RELEASE()          (btn_ctrl[0].key_volt == 1 && btn_ctrl[1].key_volt == 1)

#define GET_BTN_VOLT(btn)           btn_ctrl[btn].key_volt
#define GET_BTN_STATE(btn)          btn_ctrl[btn].key_state
#define SET_BTN_STATE(btn,state)    btn_ctrl[btn].key_state = state
#define BTN_KEY_ACTION(btn)         btn_ctrl[btn].btn_action()

//blinking three seconds
void timeout_start()
{
    tm_ctrl.key_time = millis();
    tm_ctrl.key_triggle = 1;
}

void timeout_stop()
{
    tm_ctrl.key_time = 0;
    tm_ctrl.key_triggle = 0;
}

int timeout_check()
{
    int cur_tm = millis();
    if(tm_ctrl.key_triggle == 0)
        return 0;
    if((cur_tm - tm_ctrl.key_time)<tm_ctrl.timeout_time)
        return 1;
    else
    {
        timeout_stop();
        return 2;
    }
    return 0;
}
/*
int blinking_check()
{
    if(tm_ctrl.key_time == 0)
    {
        tm_ctrl.key_time = millis();
        return 0;
    }
    else
    {
        int cur_tm = millis();
        if((cur_tm - tm_ctrl.key_time)<tm_ctrl.timeout_time)
            return 1;
    }
    return 0;
}*/

/*
#define BTN_PRESSED(btn) btn_pressed |= (1<<btn)
#define BTN_RELEASE(btn) btn_pressed &= ~(1<<btn)
#define ALL_KEY_PRESSED() (btn_pressed==(1<<btn_max -1))?1:0
#define ALL_KEY_RELEASED() btn_pressed = 0

void init_user_gui()
{
    int x_gap = MAX_X/2;
    int y_gap = MAX_Y/MAX_ENTRY;
    int x_blank = x_gap/3;
    int y_middle = (y_gap)/3;
    for(int i = 0 ; i < MAX_ENTRY ; i++)
    {
        m_entries[i].entry_pos.start_x =  x_blank;
        m_entries[i].entry_pos.option_x = x_gap;
        m_entries[i].entry_pos.start_y =  i*y_gap + y_middle;
    }
}

*/

void ui_draw_string(char * in_str,int x, int y,int font_sz)
{
  #ifdef SEEED_LCD
    //tft.setFreeFont(FF1);
    tft.setTextSize(font_sz);
    tft.drawString(in_str,x, y);
    //tft.setCursor(x, y); 
    //tft.print(in_str);
  #endif
}

void display_user_gui()
{
    menu_entry_t * p_entry;
    //init_user_gui();
    for(int i = 0 ; i < MAX_ENTRY ; i++)
    {
        p_entry = &m_entries[i];
        ui_draw_string(p_entry->m_name,p_entry->entry_pos.start_x,p_entry->entry_pos.start_y,p_entry->font_size);
        ui_draw_string(p_entry->item_list[p_entry->item_idx].item_name,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
    }
}

void display_current_option()
{
    menu_entry_t * p_entry = &m_entries[cur_entry];
    ui_draw_string(p_entry->item_list[p_entry->item_idx].item_name,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
}

void menu_update(int press_key)
{
    menu_entry_t * p_entry = &m_entries[cur_entry];
    int cur_item = p_entry->item_idx;
    int max_item = p_entry->items_cnt-1;
    switch(press_key)
    {
        case btn_up:
            cur_item -= 1;
            if(cur_item < 0)
            {
                cur_item = max_item;
            }
            p_entry->item_idx = cur_item;

            break;
        case btn_down:
            cur_item += 1;
            if(cur_item > max_item)
            {
                cur_item = 0;
            }
            p_entry->item_idx = cur_item;

            break;
    }
    //display_current_option();
    //display_user_gui();
    timeout_start();
}
int previous_time = 0;
int is_info_now = 0;
void user_menu_display()
{
    menu_entry_t * p_entry;
    //init_user_gui();
    for(int i = 0 ; i < MAX_ENTRY ; i++)
    {
        p_entry = &m_entries[i];
        if(i != cur_entry)
        {
            //tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            ui_draw_string(p_entry->m_name,p_entry->entry_pos.start_x,p_entry->entry_pos.start_y,p_entry->font_size);
            ui_draw_string(p_entry->item_list[p_entry->item_idx].item_name,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
        }
        else
        {
            int t_check = timeout_check();
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            ui_draw_string(p_entry->m_name,p_entry->entry_pos.start_x,p_entry->entry_pos.start_y,p_entry->font_size);
            if( t_check == in_timeout)
            {
                if(previous_time == 0)
                    previous_time = millis();
                int now_tm = millis();
                if((now_tm - previous_time)>200)
                {
                    previous_time = 0;
                    is_info_now = (is_info_now)?0:1;
                }
                if(is_info_now)
                {
                    ui_draw_string(p_entry->item_list[p_entry->item_idx].item_name,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
                }
                else
                {
                    ui_draw_string(blank_entry,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
                }

            }
            else 
            {
                ui_draw_string(p_entry->item_list[p_entry->item_idx].item_name,p_entry->entry_pos.option_x,p_entry->entry_pos.start_y,p_entry->font_size);
                if(t_check  == out_timeout)
                {
                    Serial.println("send remote cmd");
                }
            }
        }
    }
}

void entry_update()
{
    cur_entry = (cur_entry +1)%MAX_ENTRY;
    Serial.println(cur_entry);
}

#if 0
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
#endif



#if 0
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
#endif

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
    menu_update(btn_up);
    #endif
}

void move_down(void)
{
    #ifdef SEEED_LCD
    Serial.println("B Key pressed");
    menu_update(btn_down);
    #endif
}

void both_btn_action(void)
{
    #ifdef SEEED_LCD
    Serial.println("both_btn_action");
    entry_update();
    #endif
}

/*

    int re_read_key = -1;
    int pressed_key = -1;
    int st_time = 0,end_time = 0;
    int tmp_io;
    for(int i = 0; i < MAX_BTN_NUM ;i++)
    {
        int volt = digitalRead(btn_ctrl[i].pin_num);
        if(volt == LOW)
        {
            SET_BTN_VOLT(i,volt);
            delay(20);
            re_read_key = (i == btn_up)?btn_down:btn_up;
            pressed_key = i;
            break;
        }
        if(volt == HIGH)
        {
            if(GET_BTN_STATE(i) == key_pressed)
            {
                SET_BTN_STATE(i,key_release);
                delay(500);
                Serial.println("key presssed");
                SET_BTN_STATE(i,key_init);
                return;
            }
        }
    }

    if(re_read_key != -1 && GET_BTN_STATE(pressed_key) == key_init)
    {
        st_time = millis();
        end_time = st_time;
        while((end_time - st_time)<500)
        {
            tmp_io = digitalRead(btn_ctrl[re_read_key].pin_num);
            end_time = millis();
            if(tmp_io == LOW)
            {
                Serial.println("two Key pressed");
                return;
            }
        }
    }

    if(pressed_key != -1)
    {
        if(GET_BTN_STATE(pressed_key) == key_init)
        {
            SET_BTN_STATE(pressed_key,key_pressed);
        }
    }


    if(BOTH_KEY_RELEASE())
    {
        if(GET_BTN_STATE(btn_up) == both_release||GET_BTN_STATE(btn_down) == both_release)
        {
            Serial.println("both release");
            for(int i = 0; i < MAX_BTN_NUM ;i++)
            {
                SET_BTN_STATE(i,key_init);
                return;
            }
        }
    }

*/

void button_detect()
{
  //A PRESSED B PRESSED == LOW

   for(int i = 0; i < MAX_BTN_NUM ;i++)
    {
        int volt = digitalRead(btn_ctrl[i].pin_num);
        SET_BTN_VOLT(i,volt);
    }

    if(BOTH_KEY_PRESSED())
    {
        Serial.println("two Key pressed");
        //two key have to move down
        /*
        if(GET_BTN_STATE(btn_up)!= both_pressed&& GET_BTN_STATE(btn_down)!= both_pressed)
        {
            for(int i = 0; i < MAX_BTN_NUM ;i++)
            {
                SET_BTN_STATE(i,both_pressed);
            }
        }*/
        if(GET_BTN_STATE(btn_up)!= both_pressed&& GET_BTN_STATE(btn_down)!= both_pressed)
        {
            for(int i = 0; i < MAX_BTN_NUM ;i++)
            {
                SET_BTN_STATE(i,both_pressed);
            }
        }
        else
        {
            for(int i = 0; i < MAX_BTN_NUM ;i++)
            {
                if(GET_BTN_STATE(i) == both_release)
                    SET_BTN_STATE(i,both_pressed);
            }
        }
    }
    if(BOTH_KEY_RELEASE())
    {
        if(GET_BTN_STATE(btn_up)== both_release || GET_BTN_STATE(btn_down)== both_release)
        {
            Serial.println("two Key released");
            for(int i = 0; i < MAX_BTN_NUM ;i++)
            {
                SET_BTN_STATE(i,key_init);
            }
            return;
        }

    }
    for(int i = 0; i < MAX_BTN_NUM ;i++)
    {
        if(GET_BTN_VOLT(i) == LOW)
        {
            if(GET_BTN_STATE(i) == key_init)
                {
                    SET_BTN_STATE(i,key_pressed);
                    delay(20);
                    break;
                }
        }
        if(GET_BTN_VOLT(i) == HIGH)
        {
            if(GET_BTN_STATE(i) == key_pressed)
            {
                SET_BTN_STATE(i,key_release);
                //TODO THING
                BTN_KEY_ACTION(i);
                SET_BTN_STATE(i,key_init);
                delay(20);
                break;
            }
            if(GET_BTN_STATE(i) == both_pressed)
            {
                both_btn_action();
                SET_BTN_STATE(i,both_release);
                delay(20);
                break;
            }

        }
    }
    #if 0
for(int i = 0; i < MAX_BTN_NUM ;i++)
   {
      int io_state = digitalRead(btn_ctrl[i].pin_num);

      switch(io_state)
      {
          case LOW:
            {
                if(btn_ctrl[i].key_state == key_release)
                    {
                        int all_key;
                        btn_ctrl[i].key_state = key_pressed;
                        BTN_PRESSED(i);
                        all_key = ALL_KEY_PRESSED();
                        if(all_key)
                        {
                            //TODO , both key pressed.move entry
                            //KEY state change
                            btn_ctrl[i].two_btn_action();
                            ALL_KEY_RELEASED();
                        }
                    }
            }
            break;
           case HIGH:
            {
                if(btn_ctrl[i].key_state == key_pressed)
                {
                    btn_ctrl[i].key_state = key_release;
                    btn_ctrl[i].btn_action();
                    BTN_RELEASE(i);
                    delay(100);
                }
            }
      }
   }
   
   for(int i = 0; i < MAX_BTN_NUM ;i++)
    {
        int volt = digitalRead(btn_ctrl[i].pin_num);
        if(volt == LOW)
            {
                SET_BTN_VOLT(i,volt);
                break;
            }
    }
    if(BOTH_KEY_PRESSED())
    {
        //Serial.println("two Key pressed");
        //two key have to move down

        return;
    }
    for(int i = 0; i < MAX_BTN_NUM ;i++)
    {
        if(GET_BTN_VOLT(i) == LOW)
        {
            if(GET_BTN_STATE(i) == key_init)
                SET_BTN_STATE(i,key_pressed);
        }
        if(GET_BTN_VOLT(i) == HIGH)
        {
            if(GET_BTN_STATE(i) == key_pressed)
            {
                SET_BTN_STATE(i,key_release);
                //TODO THING
                BTN_KEY_ACTION(i);
                delay(500);
                SET_BTN_STATE(i,key_init);
            }
        }

    }
    int up_io = digitalRead(btn_ctrl[btn_up].pin_num);
    int down_io = digitalRead(btn_ctrl[btn_down].pin_num);
    int st_time = 0,end_time;
    if(up_io == LOW || down_io == LOW)
    {
        st_time = millis();
        end_time = st_time;
        while((end_time - st_time)<500)
        {
            
            down_io = digitalRead(btn_ctrl[btn_down].pin_num);
            end_time = millis();
            if(down_io == LOW)
            {
                Serial.println("two Key pressed");
                return;
            }
        }
        
    }
#endif

}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(115200);
    tft.begin();
    tft.setRotation(3);
    button_init();
    display_user_gui();
    //init_user_menu();
    //display_user_menu(&u_menu);
    
}

// the loop function runs over and over again forever
void loop() {
    button_detect();
    user_menu_display();
}
