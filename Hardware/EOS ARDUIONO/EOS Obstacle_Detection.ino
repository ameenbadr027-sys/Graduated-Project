const int trigPin = 2; 
const int echoPin = 3; 
const int buzzerPin = 4;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600); // التواصل مع الـ ESP32-S3
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  if (distance > 0 && distance < 30) {
    digitalWrite(buzzerPin, HIGH);
    Serial.println("SNAP"); // الأمر اللي هيفهمه الـ ESP
    delay(1000); // تأخير عشان ما يصورش 100 صورة في ثانية
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  delay(100);
}
