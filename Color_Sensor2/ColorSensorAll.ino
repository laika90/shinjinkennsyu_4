#include "Wire.h"
#define ADDRESS 0x2A
#define CTRL 0x00
#define tMSB 0x01
#define tLSB 0x02
#define dLSB 0x03
//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint=0;
const uint16_t N=1500;
//変数定義
uint8_t   i=0;//カウンタ
uint8_t   buff[8];//8bit×8バッファ
uint16_t  datar, datag, datab, datair;//16bitデータ
void setup() {
  Serial.begin(9600);
  Wire.begin();//I2Cマスター接続
  //マニュアル積分時間倍数設定
  GAIN(N);
}
void GAIN(uint16_t N){
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);//コントロールバイトCALL
  Wire.write(0b11100100);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tMSB);//積分時間上位
  Wire.write(N>>8);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tLSB);//積分時間下位
  Wire.write(N);
  Wire.endTransmission();
}
void WRITE(uint8_t Tint){
  uint8_t val;
  //I2C書き込み
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);//コントロールバイトCALL
  Wire.write(0b10000100|Tint);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);//コントロールバイトCALL
  Wire.write(0b00001100|Tint);//ADC動作開始,wakeUp
  Wire.endTransmission();
  switch(Tint){
    case 0:
      val=1;
      break;
    case 1:
      val=3;
      break;
    case 2:
      val=45;
      break;
    case 3:
      val=360;
      break;
  }
  Serial.print("待機時間:");Serial.println(N*val);
  delay(N*val);//積分時間以上の待機時間
}
void loop() {
  WRITE(Tint);
  //I2C読み取り
  Wire.beginTransmission(ADDRESS);
  Wire.write(dLSB);
  Wire.endTransmission(false);
  //データサイズ要求
  Wire.requestFrom(ADDRESS,8);//デバイスから8byte要求
  //データ読み取り&データ処理(結合)
  for(i=0;i<7;i++){
    buff[i] = Wire.read();
  }
  datar =   buff[0]<<8 | buff[1];//上位下位結合16bit(赤)
  datag =   buff[2]<<8 | buff[3];//上位下位結合16bit(赤)
  datab =   buff[4]<<8 | buff[5];//上位下位結合16bit(赤)
  datair =  buff[6]<<8 | buff[7];//上位下位結合16bit(赤)
  Serial.print("RED:");Serial.println(datar);
  Serial.print("GREEN:");Serial.println(datag);
  Serial.print("BLUE:");Serial.println(datab);
  Serial.print("IR:");Serial.println(datair);
  Serial.println("");
  Wire.endTransmission(); 
  }