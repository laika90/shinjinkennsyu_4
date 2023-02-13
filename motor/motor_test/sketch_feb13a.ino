#define AIN1 13
#define AIN2 12
#define PWMA 11
#define BIN1 10
#define BIN2 9
#define PWMB 8
int speed = 200;
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

  void turnA(){
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
    analogWrite(PWMA,100);
    analogWrite(PWMB,0);  
  }

  void turnB(){
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
    digitalWrite(BIN1,LOW);
    digitalWrite(BIN2,HIGH);
    analogWrite(PWMA,0);
    analogWrite(PWMB,100);  
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
  int i = 0;
  for(i;i<3;i++){
  goingforward(speed);
  delay(5000);
  goingback(speed);
  delay(5000);
  turnA();
  delay(5000);  
  turnB();
  delay(5000);
  }
  

  }