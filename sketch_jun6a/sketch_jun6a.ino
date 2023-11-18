#include "ntshell_stub.hpp"
#include "led_chika.hpp"

void setup()
{
  ntshell_setup();  
  led_setup();
}

void loop()
{
  led_loop();
}
