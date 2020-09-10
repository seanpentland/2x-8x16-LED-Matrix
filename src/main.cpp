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

unsigned char row[2][16] = {
  {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF},
  {0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8,0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0}};
unsigned char col[2][8] = {
  {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80},
  {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}};
unsigned char table[2][16] = {
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};
unsigned int xRev[] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
unsigned int yRev[] = {7,6,5,4,3,2,1,0};

unsigned char dis[] = {displayNum1, displayNum2};
unsigned char disClk[] = {displayClk1, displayClk2};
unsigned char fixedAddr  = 0x44; //fixed address
unsigned char autoAddr   = 0x40; //address auto +1
unsigned char brightness = 0x88; //lowest brightness setting

void setup() {
  Serial.begin(115200);
  for(int i; i<2;i++){
    pinMode(dis[i], OUTPUT);
    pinMode(disClk[i], OUTPUT);
    digitalWrite(dis[i], LOW);
    digitalWrite(disClk[i], LOW);
    DisplaySendCmd(i, fixedAddr);
    DisplaySendCmd(i, brightness); //lowest brightness
  }
  
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
    DisplaySendAddr(1,row[1][x]);
    DisplaySend(1,col[1][y]);
    DisplayEnd(1);
  }else if(y>=8){
    DisplaySendAddr(0,row[0][x]);
    DisplaySend(0,col[0][y-8]);
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
  DisplaySendAddr(display, row[0][0]);
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
  for(int r;r<2;r++){
    DisplaySendCmd(r,autoAddr);  
    DisplaySendAddr(r, row[0][0]);
    for(int i = 0; i < 16; i++){
      DisplaySend(r, table[r][i]);
    }
    DisplayEnd(r);
    DisplaySendCmd(r,fixedAddr);
  }
}