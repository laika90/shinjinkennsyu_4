#include <Stepper.h>  // ライブラリのインクルード

#define PIN1 2  // 青
#define PIN2 3  // ピンク
#define PIN3 4  // 黄
#define PIN4 5  // オレンジ

// ステッピングモーター(出力軸)が1回転するのに必要なステップ数
#define STEP 2048
 
Stepper stepper1(STEP, PIN1, PIN3, PIN2, PIN4);  // オブジェクトを生成
 
void setup() {

  stepper1.setSpeed( 15 );  // 1分間当たりの回転数を設定(rpm)

  pinMode(PIN1, OUTPUT);    // デジタルピンを出力に設定
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
  pinMode(PIN4, OUTPUT);
  
}


void loop() {

  stepper1.step( 2048 );    // 360°回転させる(2048ステップ)

  digitalWrite(PIN1, LOW);  // 出力を停止(モーターへの電流を止め発熱を防ぐ)
  digitalWrite(PIN2, LOW); 
  digitalWrite(PIN3, LOW); 
  digitalWrite(PIN4, LOW);     
  delay(1000);
 

  stepper1.step( -1024 );   // -180°回転させる(-1024ステップ)

  digitalWrite(PIN1, LOW);  // 出力を停止(モーターへの電流を止め発熱を防ぐ)
  digitalWrite(PIN2, LOW); 
  digitalWrite(PIN3, LOW); 
  digitalWrite(PIN4, LOW);     
  delay(1000);  
}