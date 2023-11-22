#include "ntshell_stub.hpp"
#include "led_chika.hpp"
#include "camera_command.hpp"

void setup()
{
  ntshell_setup();  
  led_setup();
}

void loop()
{
  led_loop();

  // カメラ動作をフラグで制御します
  // 撮影実行前にtakePicture_runnning = falseに変更する
  // 撮影が完了したらint shots_counterの値を -1 する
  camera_flag_loop();
}
