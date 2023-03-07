#include "Wire.h"
#define ADDRESS 0x2A
#define CTRL 0x00
#define tMSB 0x01
#define tLSB 0x02
#define dLSB 0x03

#define AIN1 12
#define AIN2 13
#define PWMA 11
#define BIN1 5
#define BIN2 4
#define PWMB 6
int speed = 50;

//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint = 0;
const uint16_t N = 1500;
//変数定義
uint8_t i = 0;                         //カウンタ
uint8_t buff[8];                       //8bit×8バッファ
uint16_t datar, datag, datab, datair;  //16bitデータ
void setup() {
  Serial.begin(9600);
  Wire.begin();  //I2Cマスター接続
  //マニュアル積分時間倍数設定
  GAIN(N);

  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);
  pinMode(PWMB,OUTPUT);
}

void GAIN(uint16_t N) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);  //コントロールバイトCALL
  Wire.write(0b11100100);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tMSB);  //積分時間上位
  Wire.write(N >> 8);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tLSB);  //積分時間下位
  Wire.write(N);
  Wire.endTransmission();
}
void WRITE(uint8_t Tint) {
  uint8_t val;
  //I2C書き込み
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);  //コントロールバイトCALL
  Wire.write(0b10000100 | Tint);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);               //コントロールバイトCALL
  Wire.write(0b00001100 | Tint);  //ADC動作開始,wakeUp
  Wire.endTransmission();
  switch (Tint) {
    case 0:
      val = 1;
      break;
    case 1:
      val = 3;
      break;
    case 2:
      val = 45;
      break;
    case 3:
      val = 360;
      break;
  }
  //Serial.print("待機時間:");
  //Serial.println(N * val);
  delay(N * val);  //積分時間以上の待機時間
}
void color() {
  WRITE(Tint);
  //I2C読み取り
  Wire.beginTransmission(ADDRESS);
  Wire.write(dLSB);
  Wire.endTransmission(false);
  //データサイズ要求
  Wire.requestFrom(ADDRESS, 8);  //デバイスから8byte要求
  //データ読み取り&データ処理(結合)
  for (i = 0; i < 7; i++) {
    buff[i] = Wire.read();
  }
  datar = buff[0] << 8 | buff[1];   //上位下位結合16bit(赤)
  datag = buff[2] << 8 | buff[3];   //上位下位結合16bit(赤)
  datab = buff[4] << 8 | buff[5];   //上位下位結合16bit(赤)
  datair = buff[6] << 8 | buff[7];  //上位下位結合16bit(赤)
  //Serial.print("RED:");
  //Serial.println(datar);
  //Serial.print("GREEN:");
  //Serial.println(datag);
  //Serial.print("BLUE:");
  //Serial.println(datab);
  //Serial.print("IR:");
  //Serial.println(datair);
  //Serial.println("");
  Wire.endTransmission();
}
void color_search(){
  unsigned short int lst_red_m[5];
  int i;
  int red_sum=0;
  //同じ方向で3回測定する
  for(i=1;i<=3;i=i+1){
    color();
    red_sum+=datar;
    delay(10);
  }
  int red_mean=0;
  //同じ方向での測定値の平均
  red_mean=red_sum/3;
  //方向ごとの測定値の平均を更新  
  lst_red_m[0]=lst_red_m[1];
  lst_red_m[1]=lst_red_m[2];
  lst_red_m[2]=lst_red_m[3];
  lst_red_m[3]=lst_red_m[4];
  lst_red_m[4]=red_mean;
  Serial.println(red_mean);
}

void goingforward(int speed){
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
    analogWrite(PWMA,speed);
    analogWrite(PWMB,speed);
  }
void goingback(int speed){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,speed);
  analogWrite(PWMB,speed);
}
void turn_left(){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,50);
  analogWrite(PWMB,50);  
}
void turn_right(){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,50);
  analogWrite(PWMB,50);  
} 
void stop(){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,LOW);
}

unsigned short int lst_red_m[5]={0,0,0,0,0};
void loop(){
  //過去5回の平均測定値が上に凸になっている場合に直進する
  if(lst_red_m[0]<lst_red_m[1]<lst_red_m[2]>lst_red_m[3]>lst_red_m[4]){
    Serial.print("RED");
    turn_right();//2回前が極大なので少し戻る
    delay(400);
    stop();
    delay(200);
    goingforward(speed);
    delay(5000);
    stop();
    delay(1000);
  }
  else
  {
  turn_left();
  delay(200);
  stop();
  color_search();
  }
}