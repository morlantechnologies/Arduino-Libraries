//

const int pinTemp = A0;      // pin of temperature sensor
int pinFan = 6;


float temperature,temp_c;
int B=3975;                  // B value of the thermistor
float resistance;

void setup()
{
    Serial.begin(9600);
    pinMode(pinFan,OUTPUT);
    
}

void loop()
{
    int val = analogRead(pinTemp);                               // get analog value
    resistance=(1023-val)*10000/val;                      // get resistance
    temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;     // calc temperature
    temp_c = (5.0/9)*(temperature-32.0);
    Serial.println(temp_c);
 digitalWrite(pinFan,HIGH);
    
    
    delay(1000);          // delay 1s

    if (temp_c > 25)
    {
      digitalWrite(pinFan,HIGH);
    }
    else
    {
      digitalWrite(pinFan,LOW);
    }
    
}
