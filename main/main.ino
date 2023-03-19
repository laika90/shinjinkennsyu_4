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
#define PHOTO_PIN 0

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
uint16_t consective3_color_array[3][4] = {{0}};
short int find_counter = 0;
const uint8_t speed = 50; // モーターのスピード
const uint8_t turn_speed = 50; // 回転時のモータースピード
const uint8_t up_speed = 75; // リフト上昇時のスピード
const uint8_t down_speed = 30; // リフト下降時のスピード

const short int chipSelect = 10;// SDのチップセレクト

//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint = 0;
const uint16_t N = 3120;

const short int photoresistor_threshold = 300; // フォトレジスタの閾値
const float color_threshold_ratio = 3; // カラーセンサ閾値倍率
const float ir_threshold_ratio = 1; // 赤外のみ弱いので

const String front_sharp = "\n#";
const String back_sharp = "#\n";

const short int right_correction = 8;

// 一回だけ実行
void setup() {
  Serial.begin(9600);
  Wire.begin();  // I2Cマスター接続
  // マニュアル積分時間倍数設定
  GAIN(N);
  pinMode(INPIN,INPUT);
  pinMode(LEDPIN,OUTPUT);
  pinMode(PHOTO_PIN, INPUT);
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
  lift_up(50, 7000);

}
// カラーセンサーにゲインを書きこむ
void GAIN(uint16_t N) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);  //コントロールバイトCALL
  Wire.write(0b11101100);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tMSB);  //積分時間上位
  Wire.write(0b00001100);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(tLSB);  //積分時間下位
  Wire.write(0b00110000);
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
  Wire.write(0b10001100 | Tint);
  Wire.endTransmission(false);
  Wire.beginTransmission(ADDRESS);
  Wire.write(CTRL);               // コントロールバイトCALL
  Wire.write(0b00001100 | Tint);  // ADC動作開始,wakeUp
  Wire.endTransmission();
  delay(2200);  // 積分用待機時間。546ms * 4色。一回の処理にかかる時間。
}

// カラーセンサーで値をとる。
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

  // 出力のフォーマット
  SD_write("Color intensity: { RED , GREEN , BLUE , INFRARED } = { ");
  for (int i = 0; i < ARRAY_LENGTH(color_array); ++i){
    SD_write(color_array[i]);
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
  analogWrite(PWMB,speed+right_correction);
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
  analogWrite(PWMB,speed+right_correction+4);
  SD_write("\nGo back. ");
  delay_log(delay_time);
  stop();
}

void turn_right(int turn_speed, int delay_time){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed+right_correction);

  SD_write("\nTurn right. ");
  delay_log(delay_time);
  stop();
}

void turn_left(int turn_speed, int delay_time){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed+right_correction);

  SD_write("\nTurn left. ");
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

  SD_write(front_sharp); SD_write(" offset start "); SD_write(back_sharp);

  // 三か所で色をとる。合計約7秒。
  update_color_array();
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){ offset_val[i] += color_array[i]; }
  turn_left(turn_speed, 150);
  update_color_array();
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){ offset_val[i] += color_array[i]; }
  turn_right(turn_speed, 300);
  update_color_array();
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){ offset_val[i] += color_array[i]; }
  turn_left(turn_speed, 150);

  // 全体を1/3
  for (int i = 0; i < ARRAY_LENGTH(offset_val); ++i){ offset_val[i] = offset_val[i]/3; }

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
  SD_write(front_sharp); SD_write(" offset finish "); SD_write(back_sharp);
}

void update_consective3_color_array(){
  for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[0]); ++i){ consective3_color_array[0][i] = consective3_color_array[1][i]; }
  for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[1]); ++i){ consective3_color_array[1][i] = consective3_color_array[2][i]; }
  for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[2]); ++i){ consective3_color_array[2][i] = color_array[i]; }
}

void reset_consective3_color_array(){
  for (int i = 0; i < ARRAY_LENGTH(consective3_color_array); ++i){ for (int j = 0;  j < ARRAY_LENGTH(consective3_color_array[i]); ++j){ consective3_color_array[i][j] = 0; }}
}

bool is_local_maximum(int color_select){
  if (color_select == INFRARED){
    return consective3_color_array[1][color_select] >= offset_val[color_select] + ir_threshold_ratio && consective3_color_array[0][color_select] <= consective3_color_array[1][color_select] && consective3_color_array[2][color_select] < consective3_color_array[1][color_select];
  } else {
    return consective3_color_array[1][color_select] >= offset_val[color_select] + color_threshold_ratio && consective3_color_array[0][color_select] <= consective3_color_array[1][color_select] && consective3_color_array[2][color_select] < consective3_color_array[1][color_select];
  }
}

bool is_null(int color_select){
  return consective3_color_array[0][color_select] == 0;
}

bool is_base(){
  return consective3_color_array[1][INFRARED] > offset_val[INFRARED] * ir_threshold_ratio;
}

bool is_maximum(int color_select){
  int max_index = RED;
  for (int i = 0; i < ARRAY_LENGTH(consective3_color_array[1]); ++i){
    if (consective3_color_array[1][i] > consective3_color_array[1][max_index]){
      max_index = i;
    }
  }
  return max_index == color_select;
}

bool is_infrared(){
  return consective3_color_array[1][RED] > offset_val[RED] &&  consective3_color_array[1][BLUE] > offset_val[BLUE] && consective3_color_array[1][GREEN] > offset_val[GREEN];
}

bool is_green(){
  return consective3_color_array[1][GREEN] > 2 * consective3_color_array[1][INFRARED];
}

bool is_stuck(int & stuck_counter, const short int & find_counter){
  return stuck_counter > 15 && find_counter > 1;
}

void escape_stuck(int & stuck_counter, int color_select){
  SD_writeln("$ stuck $");

  // 一回点灯
  digitalWrite(LEDPIN, HIGH);
  delay(500);
  digitalWrite(LEDPIN, LOW);
  if (color_select == INFRARED){ digitalWrite(LEDPIN, HIGH); }

  go_back(speed, 1000);
  turn_right(turn_speed, 300);
  stuck_counter = 0;
  reset_consective3_color_array();
}

bool can_see_object(int & stuck_counter, const short int & find_counter){
  return stuck_counter <= 20 || find_counter != 0;
}

bool can_see_ir(const int & stuck_counter){
  return stuck_counter <= 20;
}

bool can_see_color(const int & stuck_counter){
  return stuck_counter <= 50;
}

void approach_higher_values(int & stuck_counter, int & color_max_index, int & color_max){

  turn_right(turn_speed, 60*color_max_index);
  go_forward(speed, 2000);
  stuck_counter = 0;
  color_max_index = 0;
  color_max = 0;
  turn_right(turn_speed, 400);
}

// void go_straight_unconditionally(int & stuck_counter, int color_select){
//   SD_writeln("can't find the child unit or base");
//
//   // 2回点灯
//   digitalWrite(LEDPIN, HIGH);
//   delay(500);
//   digitalWrite(LEDPIN, LOW);
//   delay(500);
//   digitalWrite(LEDPIN, HIGH);
//   delay(500);
//   digitalWrite(LEDPIN, LOW);
//   if (color_select == INFRARED){ digitalWrite(LEDPIN, HIGH); }
//
//   turn_right(turn_speed, 50 * stuck_counter);
//   go_forward(speed, 2000);
//   turn_right(turn_speed, 300);
//   stuck_counter = 0;
// }

void search_for(int color_select){
  int stuck_counter = 0;
  int color_max = 0;
  int color_max_index = 0;
  while(true){
    update_color_array();
    update_consective3_color_array();
    bool find_object;
    if (color_select == INFRARED){ find_object = is_local_maximum(color_select) && is_infrared()/* && is_maximum(color_select)&& !is_null(color_select)*/; }
    else if (color_select == GREEN){ find_object = is_local_maximum(color_select) && !is_null(color_select) && is_maximum(color_select) && is_green()/*&& !is_base()*/; }
    else { find_object = is_local_maximum(color_select) && !is_null(color_select) && is_maximum(color_select)/*&& !is_base()*/; }

    if(find_object){
      reset_consective3_color_array();
      turn_right(turn_speed, 65);
      find_counter += 1;
      break;
    } else {
      turn_left(turn_speed, 50);
      ++stuck_counter;
      ++color_max_index;

      // IR値の更新
      if (consective3_color_array[2][color_select] >= color_max){
        color_max = consective3_color_array[2][color_select];
        color_max_index = 0;
      }

      // スタック判定
      if (is_stuck(stuck_counter, find_counter)){ escape_stuck(stuck_counter, color_select); }
      // 適当な方向へ直進
      // else if (!can_see_object(stuck_counter, find_counter)){ go_straight_unconditionally(stuck_counter, color_select); }
      // 赤外線のみ実装
      if (!can_see_color(stuck_counter)){ approach_higher_values(stuck_counter, color_max_index, color_max); }
    }
  }
}

void roughly_approach(int color_select){
  float distance;
  short int forward_counter = 0;
  while(true){
    search_for(color_select);
    distance = measure_distance();
    if(distance < 20 && distance > 1){
      break;
    } else {
      if (forward_counter < 3){
        go_forward(speed, 3000);
        ++forward_counter;
      }
      else { go_forward(speed, 2000); }

      distance = measure_distance();
      if(distance < 20 && distance > 1){ break; }

      turn_right(turn_speed, 400);
    }
  }
}

void precisely_approach(int color_select){
  float distance;
  short int zero_counter = 0;
  while(true){
    go_forward(speed, 300);
    distance = measure_distance();
    if (distance == 0){
      ++zero_counter;
      if (zero_counter > 3){ break; }
      else {
        delay_log(300);
        continue;
      }
    zero_counter = 0;
    }
    else if(distance < 10){
      break;
    }
  }
}

void lift_unit(bool has_child_unit){
  if (!has_child_unit){
    go_back(speed, 2000);
    lift_down(down_speed, 12500);
  } else {
    // 子機を持っているなら、積み重ね
    go_forward(speed, 1000);
    lift_down(down_speed, 7000);
    go_back(speed, 2000);
    lift_down(down_speed, 4500);
  }
  go_forward(speed-10, 4000);
  lift_up(up_speed, 5000);
}

bool hold_unit(){
  int start_time = millis();
  int end_time = millis();
  while(end_time - start_time < 10000){
    end_time = millis();
    if (digitalRead(INPIN) == HIGH){
      SD_writeln("hold unit is true");
      return true;
    }
  }
  SD_writeln("hold unit is false. restart collecting unit.");
  return false;
}

void collect_unit(int color_select, bool has_child_unit){

  SD_write(front_sharp); SD_write(" collect start "); SD_write(color_select); SD_write(" "); (back_sharp);

  roughly_approach(color_select);
  precisely_approach(color_select);
  lift_unit(has_child_unit);

  //ここに回収判定。回収できなかったらアームを下げて後退する。
  if(hold_unit()){
    if (color_select == RED){ turn_left(turn_speed, 1200); }
    else { turn_left(turn_speed, 1800); }
    SD_write(front_sharp); SD_write(" collect finish "); SD_write(back_sharp);
  } else {
    // 回収できない時。改善の余地あり。
    go_back(speed, 3000);
    lift_down(down_speed, 12500);
    go_forward(speed, 3000);
    go_back(speed, 3000);
    lift_up(50, 7000);
    turn_right(turn_speed, 300);
    collect_unit(color_select, has_child_unit);
    }
  find_counter = 0;
}

void approach_base(){
  digitalWrite(LEDPIN, HIGH);
  bool is_countinue = true;
  int photo_val;
  int start_time;
  int end_time;
  while(is_countinue){
    search_for(INFRARED);
    start_time = millis();
    end_time = millis();
    while(end_time - start_time < 5000){
      go_forward(speed+10, 50);
      photo_val = analogRead(PHOTO_PIN);
      SD_write("photo_val: ");
      SD_writeln(photo_val);
      if (photo_val < photoresistor_threshold){
        is_countinue = false;
        break;
      }
      end_time = millis();
    }
    if (is_countinue){ turn_right(turn_speed, 400); }
  }
  digitalWrite(LEDPIN, LOW);
  find_counter = 0;
}

void return_unit(int color_select){

  SD_write(front_sharp); SD_write(" return start "); SD_write(back_sharp);

  approach_base();
  //子機降ろす
  while(true){
    lift_down(down_speed, 12500);
    if(color_select == RED) { go_forward(speed, 1000); }
    go_back(speed, 2500);
    lift_up(50, 7000);
    if (!hold_unit()){ break; }
    else { go_forward(speed, 2500); }
  }
  turn_left(turn_speed, 1000);

  SD_write(front_sharp); SD_write(" return finish "); SD_write(back_sharp);
}

void loop(){
  collect_unit(RED, /* has_child_unit = */ false);
  return_unit(RED);
  collect_unit(GREEN, /* has_child_unit = */ false);
  collect_unit(BLUE, /* has_child_unit = */ true);
  return_unit(BLUE);
  // 子機を重ねるプログラムは作成中。カラーセンサーの閾値設定は不完全なため修正中。

  while(true);
}
