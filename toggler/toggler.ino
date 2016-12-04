int input = 4;
int output = 2;
int currentState = 0;
void setup() {
  pinMode(output, OUTPUT);
  pinMode(input, INPUT);

}

void loop() {
  if(input == HIGH){
    if(currentState == 1){
      currentState = 0;
    }else{
      currentState = 1;
    }
  }
  if(currentState == 1){
    digitalWrite(output, LOW);
  }else if(currentState == 0){
    digitalWrite(output, HIGH);
  }
}
