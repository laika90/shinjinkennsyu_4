#define AIN1 12
#define AIN2 13
#define PWMA 11
#define BIN1 5
#define BIN2 4
#define PWMB 6
int speed = 50;
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
void setup() {
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  pinMode(PWMA,OUTPUT);
  pinMode(BIN1,OUTPUT);
  pinMode(BIN2,OUTPUT);
  pinMode(PWMB,OUTPUT);   
}

void loop() {
  goingforward(speed);
  delay(5000);
  goingback(speed);
  delay(5000);
  turn_right();
  delay(5000);  
  turn_left();
  delay(5000);
  }