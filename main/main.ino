#include "Wire.h"
#include "SD.h"

//I2C通信関係の定義
#define ADDRESS 0x2A
#define CTRL 0x00
#define tMSB 0x01
#define tLSB 0x02
#define dLSB 0x03

//ピン番号の定義

//モーター類
#define AIN1 4
#define AIN2 5
#define PWMA 6
#define BIN1 7
#define BIN2 8
#define PWMB 9
#define GearIN1 15
#define GearIN2 16
#define PWMGear 3

//超音波センサー
#define echoPin 0 // Echo Pin
#define trigPin 1 // Trigger Pin

//変数・定数の定義
int start_val;
int speed = 50; //モーターのスピード
uint8_t buff[8];                     //8bit×8バッファ
uint16_t datar, datag, datab, datair;
int lst_red_m[]={0,0,0};
double Duration = 0; //受信した間隔
double Distance = 0; //距離
const int chipSelect = 10;//SDのチップセレクト

//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint = 0;
const uint16_t N = 500;

//一回だけ実行
void setup() {
  Serial.begin(9600);
  Wire.begin();  //I2Cマスター接続
  //マニュアル積分時間倍数設定
  GAIN(N);
  start_val = color_red_mean();

  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);
  pinMode(PWMB,OUTPUT);
  pinMode(GearIN1,OUTPUT);
  pinMode(GearIN2,OUTPUT);
  pinMode(PWMGear,OUTPUT);

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);

  // SSピン（Unoは10番、Megaは53番）は使わない場合でも出力にする必要があります。
  // そうしないと、SPIがスレーブモードに移行し、SDライブラリが動作しなくなります。
  pinMode(SS, OUTPUT);
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // 失敗、何もしない
    while(1);
  }
  //Serial.println(F("ok."));
  SdFile::dateTimeCallback( &dateTime );

  SD_Stringln("\nProgram Start");
  SD_String("Initial Value of Red (Calibration): ");
  SD_intln(start_val);
}
//カラーセンサーにゲインを書きこむ
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


//SDに必要な奴
void dateTime(uint16_t* date, uint16_t* time)
{
  uint16_t year = 2013;
  uint8_t month = 2, day = 3, hour = 9, minute = 0, second = 0;

  // GPSやRTCから日付と時間を取得
  // FAT_DATEマクロでフィールドを埋めて日付を返す
  *date = FAT_DATE(year, month, day);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  *time = FAT_TIME(hour, minute, second);
}

void SD_intln(int SD_num){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.println(SD_num);
    dataFile.close();
    // シリアルポートにも出力
    Serial.println(SD_num);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  }
}

void SD_Stringln(String SD_Str){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.println(SD_Str);
    dataFile.close();
    // シリアルポートにも出力
    Serial.println(SD_Str);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  } 
}

void SD_Doubleln(double SD_Double){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.println(SD_Double);
    dataFile.close();
    // シリアルポートにも出力
    Serial.println(SD_Double);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  } 
}

void SD_int(int SD_num){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.print(SD_num);
    dataFile.close();
    // シリアルポートにも出力
    Serial.print(SD_num);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  }
}

void SD_String(String SD_Str){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.print(SD_Str);
    dataFile.close();
    // シリアルポートにも出力
    Serial.print(SD_Str);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  } 
}

template <class T>
void SD_write(T SD_source){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.print(SD_source);
    dataFile.close();
    // シリアルポートにも出力
    Serial.print(SD_source);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  } 
}

//カラーセンサーで計測をする。これは次の関数(color)に使うだけ。
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

//カラーセンサーで値をとる。戻り値は赤の値
int color_red() {
  WRITE(Tint);
  //I2C読み取り
  Wire.beginTransmission(ADDRESS);
  Wire.write(dLSB);
  Wire.endTransmission(false);
  //データサイズ要求
  Wire.requestFrom(ADDRESS, 8);  //デバイスから8byte要求
  //データ読み取り&データ処理(結合)
  for (int i = 0; i < 7; i++) {
    buff[i] = Wire.read();
  }
  datar = buff[0] << 8 | buff[1];   //上位下位結合16bit(赤)
  //datag = buff[2] << 8 | buff[3];   //上位下位結合16bit(緑)
  //datab = buff[4] << 8 | buff[5];   //上位下位結合16bit(青)
  //datair = buff[6] << 8 | buff[7];  //上位下位結合16bit(赤外)
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
  return datar;
}

int color_red_mean(){
  int lst_color_red[] = {0,0,0}; 
  lst_color_red[0] = color_red();
  lst_color_red[1] = color_red();
  lst_color_red[2] = color_red();
  int color_red_val = (lst_color_red[0])/3 + lst_color_red[1]/3 + lst_color_red[2]/3;

  //Serial.print(lst_color_red[0]);
  //Serial.print(", ");
  //Serial.print(lst_color_red[1]);
  //Serial.print(", ");
  //Serial.println(lst_color_red[2]);
  //Serial.print("Average: ");
  //Serial.println(color_red_val);
  return color_red_val;
  
}

//モーター関連の関数
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

void lift_up(int up_speed){
  digitalWrite(GearIN1,HIGH);
  digitalWrite(GearIN2,LOW);
  analogWrite(PWMGear,up_speed);
}

void lift_down(int down_speed){
  digitalWrite(GearIN1,HIGH);
  digitalWrite(GearIN2,LOW);
  analogWrite(PWMGear,down_speed);
}

void lift_stop(){
  digitalWrite(GearIN1,LOW);
  digitalWrite(GearIN2,LOW);
}

//超音波センサーの関数
void distance(){
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds( 10 ); //
  digitalWrite(trigPin, LOW);
  Duration = pulseIn(echoPin, HIGH); //センサからの入力
  if (Duration > 0) {
    Duration = Duration/2; //往復距離を半分にする
    Distance = Duration*340*100/1000000; // 音速を340m/sに設定
    SD_String("Distance:");
    SD_Doubleln(Distance);
    //SD_Stringln(" cm");
  }
}

void loop(){
  while(1){
    while(1){
      int redred = color_red_mean();
      SD_String("Red: ");
      SD_intln(redred);
      lst_red_m[0] = lst_red_m[1];
      lst_red_m[1] = lst_red_m[2];
      lst_red_m[2] = redred;
      if(lst_red_m[1] > start_val * 1.3 && lst_red_m[0] < lst_red_m[1] && lst_red_m[2] < lst_red_m[1]){
        //turn_right();
        //delay(200);
        //stop();
        break;
      }
      // turn_left();
      // delay(200);
      // stop();
      SD_Stringln("Turning");

    }

    SD_Stringln("Red detected");
    distance();
    if(Distance < 20 && Distance > 1){
      break;
    } else{
      // goingforward(speed);
      // delay(500);
      // stop();
      // turn_right();
      // delay(1000);
      // stop();
    }

  }
  SD_Stringln("Object Collection Phase");
  while(1){
    // goingforward(speed);
    // delay(300);
    // stop();
    distance();
    if(Distance < 10.0){
      // goingforward(speed);
      // delay(2000);
      // stop();
      break;
    }
  }

  SD_Stringln("Lift Start");
  // lift_up(50);
  // delay(3000);
  // lift_stop();

  // turn_right();
  // delay(3000);
  // stop();

  SD_Stringln("Collected");
  while(1){};
  
}