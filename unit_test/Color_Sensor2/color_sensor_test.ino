#include <Wire.h>

//アドレス指定
#define S11059_ADDR 0x2A
#define CONTROL_MSB 0x00
#define CONTROL_1_LSB 0x89
#define CONTROL_2_LSB 0x09
#define SENSOR_REGISTER 0x03

int loop_count = 1;

void setup()
{
    Serial.begin(9600);//シリアル通信を9600bpsで初期化
    Wire.begin();//I2Cを初期化

    Wire.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
    Wire.write(CONTROL_MSB);//コントロールバイトを指定
    Wire.write(CONTROL_1_LSB);//ADCリセット、スリープ解除
    Wire.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

    Wire.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
    Wire.write(CONTROL_MSB);//コントロールバイトを指定
    Wire.write(CONTROL_2_LSB);//ADCリセット解除、バスリリース
    Wire.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了
}

void loop() {
    int counter = 1;
    int blueave = 0;
    int bluetotal = 0;
    int redave = 0;
    int redtotal = 0;
    int greenave = 0;
    int greentotal = 0;
    int IRave = 0;
    int IRtotal = 0;

    Serial.print("#################### loop ");
    Serial.print(loop_count);
    Serial.print(" ####################\n");

    while (counter < 400){

        //変数宣言
        int high_byte, low_byte, red, green, blue, IR;
        int t = millis();//aruduinoの起動開始からの経過時間を表す関数
        delay(50);//50msec待機(0.05秒待機)

        Wire.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
        Wire.write(SENSOR_REGISTER);//出力データバイトを指定
        Wire.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

        Wire.requestFrom(S11059_ADDR, 8);//I2Cデバイス「S11059_ADDR」に8Byteのデータ要求

        if(Wire.available()){
            high_byte = Wire.read();//high_byteに「赤(上位バイト)」のデータ読み込み
            low_byte = Wire.read();//high_byteに「赤(下位バイト)」のデータ読み込み
            red = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、redに代入

            high_byte = Wire.read();//high_byteに「緑(上位バイト)」のデータ読み込み
            low_byte = Wire.read();//high_byteに「緑(下位バイト)」のデータ読み込み
            green = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、greenに代入

            high_byte = Wire.read();//high_byteに「青(上位バイト)」のデータ読み込み
            low_byte = Wire.read();//high_byteに「青(下位バイト)」のデータ読み込み
            blue = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、blueに代入

            high_byte = Wire.read();//high_byteに「赤外(上位バイト)」のデータ読み込み
            low_byte = Wire.read();//high_byteに「赤外(下位バイト)」のデータ読み込み
            IR = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、IRに代入
            bluetotal = bluetotal + blue;
            blueave = bluetotal / counter;
            redtotal = redtotal + red;
            redave = redtotal / counter;
            greentotal = greentotal + green;
            greenave = greentotal / counter;
            IRtotal = IRtotal + IR;
            IRave = IRtotal / counter;//平均値を計算
            }

        Wire.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

        Serial.print("blue:");
        Serial.print(blue);//「blue」をシリアルモニタに送信
        Serial.print(", red:");//文字列「,」をシリアルモニタに送信
        Serial.print(red);//「red」をシリアルモニタに送信
        Serial.print(", green:");//文字列「,」をシリアルモニタに送信
        Serial.print(green);//「green」をシリアルモニタに送信
        Serial.print(", IR:");//文字列「,」をシリアルモニタに送信
        Serial.print(IR);//「IR」をシリアルモニタに送信  
        Serial.print(", t:");//文字列「,」をシリアルモニタに送信
        Serial.print(t);
        Serial.print(", counter:");
        Serial.print(counter);
        Serial.println("");//改行
        Serial.print("blueave:");
        Serial.print(blueave);
        Serial.print(", redave: ");  
        Serial.print(redave);
        Serial.print(", greenave:");
        Serial.print(greenave);
        Serial.print(", IRave:");
        Serial.print(IRave);
        Serial.println("");//改行

        ++counter;
    }

    ++loop_count;
}