#include "Wire.h"
#include "SD.h"

// I2C通信関係の定義
#define ADDRESS 0x2A
#define CTRL 0x00
#define tMSB 0x01
#define tLSB 0x02
#define dLSB 0x03

// ピン番号の定義

//子機回収判定のpin
#define INPIN 2
#define LEDPIN 17

// モーター類
#define AIN1 4
#define AIN2 5
#define PWMA 6
#define BIN1 7
#define BIN2 8
#define PWMB 9
#define GearIN1 15
#define GearIN2 16
#define PWMGear 3

// 超音波センサー
#define echoPin 0 // Echo Pin
#define trigPin 1 // Trigger Pin

#define RED 0
#define GREEN 1
#define BLUE 2
#define INFRARED 3

// 配列の大きさをとるマクロ
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))

// 変数・定数の定義
uint16_t offset_val[4] = {0,0,0,0};
uint16_t color_array[4] = {0,0,0,0};
uint16_t color_ave_array[4] = {0,0,0,0};
const uint8_t speed = 50; // モーターのスピード
const uint8_t turn_speed = 50; // 回転時のモータースピード
const uint8_t up_speed = 75; // リフト上昇時のスピード
const uint8_t down_speed = 30; // リフト下降時のスピード

const short int chipSelect = 10;// SDのチップセレクト

//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint = 0;
const uint16_t N = 500;

const short int photoresistor_threshold = 0; // フォトレジスタの閾値
const float color_threshold_ratio = 1.5; // カラーセンサ閾値倍率
const float ir_threshold_ratio =2;

const String front_shape = "\n#"; //"###############################";
const String back_shape = "#";//"###############################\n\n";


// 一回だけ実行
void setup() {
  Serial.begin(9600);
  Wire.begin();  // I2Cマスター接続
  // マニュアル積分時間倍数設定
  GAIN(N);
  pinMode(INPIN,INPUT);
  pinMode(LEDPIN,OUTPUT);
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

  // if (SD.exists("datalog.txt")) { SD.remove("datalog.txt"); }

  // Serial.println(F("ok."));
  SdFile::dateTimeCallback( &dateTime );
  SD_writeln("\n###################");
  SD_writeln("## Program Start ##");
  SD_writeln("###################");
  offset();

}
// カラーセンサーにゲインを書きこむ
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

// SDに必要な奴
void dateTime(uint16_t* date, uint16_t* time)
{
  uint16_t year = 2023;
  uint8_t month = 3, day = 1, hour = 9, minute = 0, second = 0;

  // GPSやRTCから日付と時間を取得
  // FAT_DATEマクロでフィールドを埋めて日付を返す
  *date = FAT_DATE(year, month, day);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  *time = FAT_TIME(hour, minute, second);
}

// SDに書き込む関数１
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

// SDに書き込む関数２
template <class T>
void SD_writeln(T SD_source){
  // ファイルを開く
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // もしファイルが開けたら値を書き込む
  if (dataFile) {
    dataFile.println(SD_source);
    dataFile.close();
    // シリアルポートにも出力
    Serial.println(SD_source);
  }
  // ファイルが開けなかったらエラーを出力
  else {
    Serial.println(F("error opening datalog.txt"));
  }
}

void delay_log(int delay_time){
  delay(delay_time);

  SD_write("delay for ");
  SD_write(delay_time);
  SD_writeln("ms");
}

// カラーセンサーで計測をする。これは次の関数(color)に使うだけ。
void WRITE(uint8_t Tint) {
  uint8_t val;
  // I2C書き込み
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);  // コントロールバイトCALL
  Wire.write(0b10000100 | Tint);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);               // コントロールバイトCALL
  Wire.write(0b00001100 | Tint);  // ADC動作開始,wakeUp
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
  // Serial.print("待機時間:");
  // Serial.println(N * val);
  delay(N * val);  // 積分時間以上の待機時間
}

// カラーセンサーで値をとる。戻り値は赤の値
void update_color_array() {
  uint8_t buff[8];
  WRITE(Tint);
  // I2C読み取り
  Wire.beginTransmission(ADDRESS);
  Wire.write(dLSB);
  Wire.endTransmission(false);
  // データサイズ要求
  Wire.requestFrom(ADDRESS, 8);  // デバイスから8byte要求
  // データ読み取り&データ処理(結合)
  for (int i = 0; i < 8; i++) {
    buff[i] = Wire.read();
  }
  color_array[RED] = buff[0] << 8 | buff[1];   // 上位下位結合16bit(赤)
  color_array[GREEN] = buff[2] << 8 | buff[3];   // 上位下位結合16bit(緑)
  color_array[BLUE] = buff[4] << 8 | buff[5];   // 上位下位結合16bit(青)
  color_array[INFRARED] = buff[6] << 8 | buff[7];  // 上位下位結合16bit(赤外)

  Wire.endTransmission();
}

void take_color_ave(){

  uint16_t last3_color_array[3][4] = {{0}};
  for (int i = 0; i < ARRAY_LENGTH(last3_color_array); ++i){
    update_color_array();
    for(int j = 0; j < ARRAY_LENGTH(last3_color_array[i]); ++j){ last3_color_array[i][j] = color_array[j]; }
  }

  for (int i = 0; i < ARRAY_LENGTH(color_ave_array); ++i){
    color_ave_array[i] = last3_color_array[0][i]/3 + last3_color_array[1][i]/3 + last3_color_array[2][i]/3;
  }

  SD_write("Color intensity: { RED , GREEN , BLUE , INFRARED } = { ");
  for (int i = 0; i < ARRAY_LENGTH(color_ave_array); ++i){
    SD_write(color_ave_array[i]);
    if (i != 3){
      SD_write(" , ");
    } else {
      SD_write(" ");
    }
  }
  SD_write("}\n");

}

//モーター関連の関数

void stop(){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,LOW);

  SD_writeln("Stop moving");
}

void lift_stop(){
  digitalWrite(GearIN1,LOW);
  digitalWrite(GearIN2,LOW);

  SD_writeln("Lift stop");
}

void go_forward(int speed, int delay_time){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,speed);
  analogWrite(PWMB,speed);
  SD_write("\nGo straight. ");
  delay_log(delay_time);
  stop();
}

void go_back(int speed, int delay_time){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,speed);
  analogWrite(PWMB,speed);
  SD_write("\nGo back. ");
  delay_log(delay_time);
  stop();
}

void turn_left(int turn_speed, int delay_time){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed);

  SD_write("\nTurn left. ");
  delay_log(delay_time);
  stop();
}

void turn_right(int turn_speed, int delay_time){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed);

  SD_write("\nTurn right. ");
  delay_log(delay_time);
  stop();
}

void lift_up(int up_speed, int delay_time){
  digitalWrite(GearIN1,HIGH);
  digitalWrite(GearIN2,LOW);
  analogWrite(PWMGear,up_speed);

  SD_write("\nLift up. ");
  delay_log(delay_time);
  lift_stop();
}

void lift_down(int down_speed, int delay_time){
  digitalWrite(GearIN1,LOW);
  digitalWrite(GearIN2,HIGH);
  analogWrite(PWMGear,down_speed);

  SD_write("\nLift down. ");
  delay_log(delay_time);
  lift_stop();
}

//超音波センサーの関数
double measure_distance(){
  double duration;
  double distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds(10); //
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); //センサからの入力
  duration = duration/2; //往復距離を半分にする
  distance = duration*340*100/1000000; // 音速を340m/sに設定

  SD_write("\ndistance:");
  SD_writeln(distance);
  delay(300);

  return distance;
}



void offset(){

  SD_write(front_shape); SD_write(" offset start "); SD_write(back_shape);

  for(int i = 0; i < 10; ++i){
    turn_right(turn_speed, 150);
    take_color_ave();
    for(int j = 0; j < ARRAY_LENGTH(offset_val); ++j){ offset_val[j] += color_ave_array[j]; }
  }
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){ offset_val[i] = offset_val[i] / 10; }

  SD_write("offset_val: { RED , GREEN , BLUE , INFRARED } = { ");
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){
    SD_write(offset_val[i]);
    if (i != 3){
      SD_write(" , ");
    } else {
      SD_write(" ");
    }
  }
  SD_write("}\n");
  SD_write(front_shape); SD_write(" offset finish "); SD_write(back_shape);
}

void collect_unit(int color_select, bool has_child_unit){

  SD_write(front_shape); SD_write(" collect start "); SD_write(back_shape);

  float distance;
  uint16_t consective3_color_array[3][4] = {{0}};
  short int forward_counter = 0;

  //これは距離が20cm以下になるまで繰り返されるwhile
  while(true){
    //これは子機の方向を探すときに繰り返されるwhile
    while(true){
      take_color_ave();
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[0]); ++i){ consective3_color_array[0][i] = consective3_color_array[1][i]; }
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[1]); ++i){ consective3_color_array[1][i] = consective3_color_array[2][i]; }
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[2]); ++i){ consective3_color_array[2][i] = color_ave_array[i]; }
      if(consective3_color_array[1][color_select] > offset_val[color_select] * color_threshold_ratio && consective3_color_array[0][color_select] <= consective3_color_array[1][color_select] && consective3_color_array[2][color_select] <= consective3_color_array[1][color_select] && consective3_color_array[0][color_select] != 0 && consective3_color_array[1][INFRARED] < offset_val[INFRARED] * ir_threshold_ratio){
        // consective3_color_arrayの初期化
        for (int i = 0; i < ARRAY_LENGTH(consective3_color_array); ++i){ for (int j = 0;  j < ARRAY_LENGTH(consective3_color_array[i]); ++j){ consective3_color_array[i][j] = 0; }}
        turn_right(turn_speed, 100);
        break;
      } else {
        turn_left(turn_speed, 50);
      }
    }

    distance = measure_distance();
    if(distance < 20 && distance > 1){
      break;
    } else {
      if (forward_counter < 3){ go_forward(speed, 3000); }
      else { go_forward(speed, 1000); }
      turn_right(turn_speed, 500);
    }
  }


  while(true){
    go_forward(speed, 300);
    distance = measure_distance();
    if(distance < 10){
      break;
    }
  }

  // 子機回収動作
  if (!has_child_unit){
    go_back(speed, 2000);
    lift_down(down_speed, 12500);
  } else {
    // 子機を持っているなら、積み重ね
    go_forward(speed, 500);
    lift_down(down_speed, 8000);
    go_back(speed, 2000);
    lift_down(down_speed, 4500);
  }
  go_forward(speed, 2500);
  lift_up(up_speed, 5000);

  //ここに回収判定。回収できなかったらアームを下げて後退する。
  if(digitalRead(INPIN) == HIGH){
    turn_left(turn_speed, 1800);
    SD_write(front_shape); SD_write(" collect finish "); SD_write(back_shape);
  } else {
    // 回収できない時。改善の余地あり。
    lift_down(down_speed, 1000);
    go_back(speed, 3000);
    collect_unit(color_select);
    }
}


void return_unit(){

  SD_write(front_shape); SD_write(" return start "); SD_write(back_shape);

  uint16_t consective3_color_array[3][4] = {{0}};
  int val;
  while(true){
    //これは拠点の方向を探すときに繰り返されるwhile
    while(true){
      take_color_ave();
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[0]); ++i){ consective3_color_array[0][i] = consective3_color_array[1][i]; }
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[1]); ++i){ consective3_color_array[1][i] = consective3_color_array[2][i]; }
      for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[2]); ++i){ consective3_color_array[2][i] = color_ave_array[i]; }
      if(consective3_color_array[1][INFRARED] > offset_val[INFRARED] * color_threshold_ratio && consective3_color_array[0][INFRARED] <= consective3_color_array[1][INFRARED] && consective3_color_array[2][INFRARED] <= consective3_color_array[1][INFRARED] && consective3_color_array[0][INFRARED] != 0){
        // consective3_color_arrayの初期化
        for (int i = 0; i < ARRAY_LENGTH(consective3_color_array); ++i){ for (int j = 0;  j < ARRAY_LENGTH(consective3_color_array[i]); ++j){ consective3_color_array[i][j] = 0; }}
        turn_right(turn_speed, /* time = */100);
        break;
      } else {
        turn_left(turn_speed, 50);
      }
    }

    // ランイントレース
    val = analogRead(0);
    if(val < photoresistor_threshold){
      go_forward(speed, 500);
      turn_right(turn_speed, 1000);
    } else {
      break;
    }
  }
  lift_down(down_speed, 1000);

  turn_right(turn_speed, 1800);

  SD_write(front_shape); SD_write(" return finish "); SD_write(back_shape);
}

void loop(){
  collect_unit(RED, /* has_child_unit = */ false);
  return_unit();
  collect_unit(BLUE, /* has_child_unit = */ false);
  collect_unit(GREEN, /* has_child_unit = */ true);
  return_unit();
  // 子機を重ねるプログラムは作成中。カラーセンサーの閾値設定は不完全なため修正中。

  while(true);
}
