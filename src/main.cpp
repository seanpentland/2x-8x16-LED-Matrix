#include <Arduino.h>

#define displayNum1 6
#define displayClk1 7
#define displayNum2 8
#define displayClk2 9

void DisplayStart(int display);
void DisplayEnd(int display);
void DisplaySend(int display, unsigned char send_data);
void DisplaySendCmd(int display, unsigned char send_cmd);
void DisplaySendAddr(int display, unsigned char send_addr);
void DisplayClear(int display);
void DisplayClearBoth();
void DisplaySingle(int x, int y);
void DisplayTest(int x, int y);
void DisplayTable();
void AddToTableSingle(int x, int y);
void RemoveTableSingle(int x, int y);
void ClearTable();

unsigned char row1[] = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 
                        0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF};
unsigned char row2[] = {0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8, 
                        0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0};
unsigned char col2[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char col1[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
unsigned char table[2][16] = {
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};
unsigned int xRev[] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
unsigned int yRev[] = {7,6,5,4,3,2,1,0};

//unsigned char tab[][] = {0x00},{0x00}; //0x80,0x01
//unsigned short gri[]   = {0x8080};
unsigned char dis[] = {displayNum1, displayNum2};
unsigned char disClk[] = {displayClk1, displayClk2};
unsigned char fixedAddr  = 0x44; //fixed address
unsigned char autoAddr   = 0x40; //address auto +1
unsigned char brightness = 0x88; //lowest brightness setting

void setup() {
  Serial.begin(115200);
  pinMode(dis[0], OUTPUT);
  pinMode(dis[1], OUTPUT);
  pinMode(disClk[0], OUTPUT);
  pinMode(disClk[1], OUTPUT);
  digitalWrite(dis[0], LOW);
  digitalWrite(dis[1], LOW);
  digitalWrite(disClk[0], LOW);
  digitalWrite(disClk[1], LOW);
  DisplaySendCmd(0, fixedAddr);
  DisplaySendCmd(1, fixedAddr);
  DisplaySendCmd(0, brightness); //lowest brightness
  DisplaySendCmd(1, brightness);
}

void loop() {

  for(int i=0;i<16;i++){
    for(int j=0;j<16;j++){
      AddToTableSingle(i,j);
      //DisplayTable();
      //delay(3);
    }
  }
  DisplayTable();
  for(int i=0;i<16;i++){
    for(int j=0;j<16;j++){
      RemoveTableSingle(i,j);
      //DisplayTable();
      //delay(3);
    }
  }
  DisplayTable();
  //ClearTable();



} //end main loop


/* --- Remove all items from table --- */
void ClearTable(){
  for(int i=0;i<2;i++){
    for(int j=0;j<16;j++){
      table[i][j] = 0x00;
    }
  }
}

/* --- Remove a single coordinate from table --- */
void RemoveTableSingle(int x, int y){
  if(y>=8){
    table[0][x] &= ~(1<<yRev[y-8]);
  }else if(y<=7){
    table[1][xRev[x]] &= ~(1<<y);
  }
}

/* --- Add a single coordinate to table --- */
void AddToTableSingle(int x, int y){
  if(y>=8){
    table[0][x] |= (1<<yRev[y-8]);
  }else if(y<=7){
    table[1][xRev[x]] |= (1<<y);
  }
}

/* --- Display a single LED based on XY coordinates --- */
void DisplaySingle(int x, int y){
  DisplayClearBoth();
  if(y<=7){
    DisplaySendAddr(1,row2[x]);
    DisplaySend(1,col2[y]);
    DisplayEnd(1);
  }else if(y>=8){
    DisplaySendAddr(0,row1[x]);
    DisplaySend(0,col1[y-8]);
    DisplayEnd(0);
  }
}

/* --- Start Communication with Display --- */
void DisplayStart(int display){
    digitalWrite(disClk[display], LOW);
    digitalWrite(dis[display], HIGH);
    digitalWrite(disClk[display], HIGH);
    digitalWrite(dis[display], LOW);
}

/* --- Send Data to Display --- */
void DisplaySend(int display, unsigned char send_data){
  for (char i = 0; i < 8; i++){
    digitalWrite(disClk[display], LOW);
    if (send_data & 0x01){
      digitalWrite(dis[display], HIGH);
    }else{
      digitalWrite(dis[display], LOW);
    }
    digitalWrite(disClk[display], HIGH);
    send_data = send_data >> 1;
  }
}

/* --- Send Command (Includes Start and Send) --- */
void DisplaySendCmd(int display, unsigned char send_cmd){
  DisplayStart(display);
  DisplaySend(display, send_cmd);
  DisplayEnd(display);
}

/* --- Send Address (Includes Start only) --- */
void DisplaySendAddr(int display, unsigned char send_addr){
  DisplayStart(display);
  DisplaySend(display, send_addr);
}

/* --- End Communnication with display --- */
void DisplayEnd(int display){
  digitalWrite(disClk[display], LOW);
  digitalWrite(dis[display], LOW);
  digitalWrite(disClk[display], HIGH);
  digitalWrite(dis[display], HIGH);
}

/* --- Clear the display --- */
void DisplayClear(int display){
  DisplaySendCmd(display,autoAddr);  
  DisplaySendAddr(display, row1[0]);
  for(int i = 0; i < 16; i++){
    DisplaySend(display, 0x00);
  }
  DisplayEnd(display);
  DisplaySendCmd(display,fixedAddr);
}

/* --- Clear both displays --- */
void DisplayClearBoth(){
  DisplayClear(0);
  DisplayClear(1);
}

/* --- Send contents of table[][] to both displays --- */
void DisplayTable(){
  
  DisplaySendCmd(0,autoAddr);  
  DisplaySendAddr(0, row1[0]);
  for(int i = 0; i < 16; i++){
    DisplaySend(0, table[0][i]);
  }
  DisplayEnd(0);
  DisplaySendCmd(0,autoAddr);
  DisplaySendCmd(1,autoAddr);  
  DisplaySendAddr(1, row1[0]);
  for(int i = 0; i < 16; i++){
    DisplaySend(1, table[1][i]);
  }
  DisplayEnd(1);
  DisplaySendCmd(1,fixedAddr);
}