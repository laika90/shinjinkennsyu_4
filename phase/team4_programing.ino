#include "Wire.h"
#include "SD.h"

//I2C通信関係の定義
#define ADDRESS 0x2A
#define CTRL 0x00
#define tMSB 0x01
#define tLSB 0x02
#define dLSB 0x03

//ピン番号の定義

//子機回収判定のpin
#define INPIN 2
#define LEDPIN 17

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
int turn_speed = 50; //回転時のモータースピード
int up_speed = 50; // リフト上昇時のスピード
int down_speed = 40; //リフト下降時のスピード

const int chipSelect = 10;//SDのチップセレクト

//"color_mean"の引数に使って
const short int select_red = 1;
const short int select_green = 2;
const short int select_blue = 3;
const short int select_infrared = 4;
//積分時間 00:87.5us, 01:1.4ms, 10:22.4ms, 11:179.2ms
const uint8_t Tint = 0;
const uint16_t N = 500;

const int photoresistor = 0; //フォトレジスタの閾値

//一回だけ実行
void setup() {
  Serial.begin(9600);
  Wire.begin();  //I2Cマスター接続
  //マニュアル積分時間倍数設定
  GAIN(N);
  start_val = color_mean(select_red);

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
  //Serial.println(F("ok."));
  SdFile::dateTimeCallback( &dateTime );

  SD_writeln("\n#################");
  SD_writeln("##Program Start##");
  SD_writeln("#################");
  SD_write("Initial Value of Red (Calibration): ");
  SD_writeln(start_val);
  SD_write("\n");
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
  uint16_t year = 2023;
  uint8_t month = 3, day = 1, hour = 9, minute = 0, second = 0;

  // GPSやRTCから日付と時間を取得
  // FAT_DATEマクロでフィールドを埋めて日付を返す
  *date = FAT_DATE(year, month, day);

  // FAT_TIMEマクロでフィールドを埋めて時間を返す
  *time = FAT_TIME(hour, minute, second);
}

//SDに書き込む関数１
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

//SDに書き込む関数２
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
int measuring_color(int color_select) {
  uint16_t datar, datag, datab, datair;
  uint8_t buff[8];
  uint16_t color_value;
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
  datag = buff[2] << 8 | buff[3];   //上位下位結合16bit(緑)
  datab = buff[4] << 8 | buff[5];   //上位下位結合16bit(青)
  datair = buff[6] << 8 | buff[7];  //上位下位結合16bit(赤外)

  Wire.endTransmission();
  
  switch(color_select) {
    case 1:
      color_value = datar;
      break;
    case 2:
      color_value = datag;
      break;
    case 3:
      color_value = datab;
      break;
    case 4:
      color_value = datair;
      break;

  }
  
  return color_value;
}

int color_mean(int color_select){
  int lst_color[] = {0,0,0}; 
  String color_log;

  lst_color[0] = measuring_color(color_select);
  lst_color[1] = measuring_color(color_select);
  lst_color[2] = measuring_color(color_select);
  int color_value = (lst_color[0])/3 + lst_color[1]/3 + lst_color[2]/3;
  
  switch(color_select) {
    case 1:
      color_log = "red";
      break;
    case 2:
      color_log = "green";
      break;
    case 3:
      color_log = "blue";
      break;
    case 4:
      color_log = "infrared";
      break;

  }

  SD_write("\nMeasuring ");
  SD_writeln(color_log);
  SD_write("Color intensity: ");
  SD_writeln(color_value);
  return color_value;
  
}

//モーター関連の関数
void goingforward(int speed){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,speed);
  analogWrite(PWMB,speed);

  SD_writeln("\nGoing straight");
}
void goingback(int speed){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,speed);
  analogWrite(PWMB,speed);

  SD_writeln("\nGoing back");
}
void turn_left(int turn_speed){
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH);
  digitalWrite(BIN2,LOW);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed);  

  SD_writeln("\nTurning_left");
}
void turn_right(int turn_speed){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,HIGH);
  analogWrite(PWMA,turn_speed);
  analogWrite(PWMB,turn_speed);  

  SD_writeln("\nTurning right");
} 
void stop(){
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW);
  digitalWrite(BIN2,LOW);

  SD_writeln("Stop moving");
}

void lift_up(int up_speed){
  digitalWrite(GearIN1,HIGH);
  digitalWrite(GearIN2,LOW);
  analogWrite(PWMGear,up_speed);

  SD_writeln("\nLift up");
}

void lift_down(int down_speed){
  digitalWrite(GearIN1,HIGH);
  digitalWrite(GearIN2,LOW);
  analogWrite(PWMGear,down_speed);

  SD_writeln("\nLift down");
}

void lift_stop(){
  digitalWrite(GearIN1,LOW);
  digitalWrite(GearIN2,LOW);

  SD_writeln("Lift stop");
}

//超音波センサーの関数
double measuring_distance(){
  double duration;
  double distance;

  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds( 10 ); //
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH); //センサからの入力
  duration = duration/2; //往復距離を半分にする
  distance = duration*340*100/1000000; // 音速を340m/sに設定

  SD_write("\ndistance:");
  SD_writeln(distance);
  delay(300);

  return distance;
  }

void delay_log(int delay_time){
  delay(delay_time);

  SD_write("delay for ");
  SD_write(delay_time);
  SD_writeln("ms");
}
  




void collecting_LED(int color_select){
  double Distance;
  uint16_t array_color[] = {0,0,0};

  //これは距離が20cm以下になるまで繰り返されるwhile
  while(1){
    //これは子機の方向を探すときに繰り返されるwhile
    while(1){
      int color_ave = color_mean(color_select);
      array_color[0] = array_color[1];
      array_color[1] = array_color[2];
      array_color[2] = color_ave;
      if(array_color[1] > start_val * 1.3 && array_color[0] < array_color[1] && array_color[2] < array_color[1] && array_color[0] != 0){
        turn_right(turn_speed);
        delay_log(100);
        stop();
        break;
      }else{
      turn_left(turn_speed);
      delay_log(100);
      stop();
      }
    }
 
    Distance = measuring_distance();
    if(Distance < 20 && Distance > 1){
      break;
    }else{
      goingforward(speed);
      delay_log(1500);
      stop();
      turn_right(turn_speed);
      delay_log(1000);
      stop();
    }
  }


  while(1){
    goingforward(speed);
    delay_log(300);
    stop();
    Distance = measuring_distance();
    if(Distance < 10){
      break;
    }
  }

  goingforward(speed);
  delay_log(1200);
  stop();

  lift_up(up_speed);
  delay_log(1000);
  lift_stop();

  //ここに回収判定。回収できなかったらアームを下げて後退する。
  if(digitalRead(INPIN) == HIGH){
    turn_left(turn_speed);
    delay_log(4000);
    stop();
  } else{
      lift_down(down_speed);
      delay_log(1000);
      lift_stop();

      goingback(speed);
      delay_log(3000);
      stop();
      collecting_LED(color_select);
    }
}


void returning_object(){
  uint16_t array_color[] = {0,0,0};
  SD_writeln(array_color[0]);
  SD_writeln(array_color[1]);
  SD_writeln(array_color[2]);
  while(1){
    while(1){
      int color_ave = color_mean(select_infrared);
      array_color[0] = array_color[1];
      array_color[1] = array_color[2];
      array_color[2] = color_ave;
      if(array_color[1] > start_val * 1.3 && array_color[0] < array_color[1] && array_color[2] < array_color[1] && array_color[0] != 0){
        turn_right(turn_speed);
        delay_log(200);
        stop();
        break;
      }
      turn_left(turn_speed);
      delay_log(200);
      stop();
    }
 
    int val = analogRead(0);
    if(val < photoresistor){
      goingforward(speed);
      delay_log(500);
      stop();

      turn_right(turn_speed);
      delay_log(1000);
      stop();
    } else{
      break;
    }
  }
  lift_down(down_speed);
  delay_log(1000);
  lift_stop();

  turn_right(turn_speed);
  delay_log(4000);
  stop(); 

}  

void loop(){
  collecting_LED(select_red);
  returning_object();
  collecting_LED(select_blue);
  returning_object();
  collecting_LED(select_green);
  returning_object();
  //子機を重ねるプログラムは作成中。カラーセンサーの閾値設定は不完全なため修正中。

  while(1);
}