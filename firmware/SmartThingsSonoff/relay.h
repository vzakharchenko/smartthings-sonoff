#ifndef relay_h
#define relay_h

class Relay
{
  private:

    uint8_t pin;

  public:

    Relay(uint8_t pin = 0) {
      this->pin = pin;
      pinMode ( pin, OUTPUT );
    }

    void on() {
      digitalWrite(pin, HIGH);
    }

    void off() {
      digitalWrite(pin, LOW);
    }

    bool isOn() {
      return digitalRead(pin);
    }


    void loop() {
    }

};
#endif /* relay_h */
