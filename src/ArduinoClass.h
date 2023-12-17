#ifndef STARGATE_ARDUINOCLASS_H
#define STARGATE_ARDUINOCLASS_H


class ArduinoClass
{
public:
    ArduinoClass() = default;
    virtual ~ArduinoClass() = default;
    virtual void setup() = 0;
    virtual void loop() = 0;
};

#endif //STARGATE_ARDUINOCLASS_H
