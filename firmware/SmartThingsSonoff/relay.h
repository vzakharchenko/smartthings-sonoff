#ifndef relay_h
#define relay_h

class Relay
{
  private:

    uint8_t pin;

    static const uint8_t RELAY_STATE_OFF = 0;
    static const uint8_t RELAY_STATE_ON = 1;
    uint8_t state = RELAY_STATE_OFF;

  public:

    Relay(uint8_t pin = 0) {
      this->pin = pin;
      pinMode ( pin, OUTPUT );
    }

    void on() {
      digitalWrite(pin, HIGH);
      state = RELAY_STATE_ON;
    }

    void off() {
      digitalWrite(pin, LOW);
      state = RELAY_STATE_OFF;
    }

    bool isOn() {
      return state != RELAY_STATE_OFF;
    }


    void loop() {
    }

};
#endif /* relay_h */
