// cameraコマンド
#include <Arduino.h>
#include <Camera.h>
#include <MP.h>
#include <stdio.h>
#include <string>
#include <stdbool.h>
#include <SDHCI.h>
#include <unistd.h>



SDClass theSD;

// streaming起動・停止を判定するフラグ
bool takePicture_runnning;

// 撮影をする回数
int shots_counter;

// SDカードに保存された画像の数
// int sdcard_image_total;


int8_t   sndid = 100;
int8_t   rcvid; 

int camera_end(){ 
  theCamera.end();
  printf( "camera end\n");
  printf( "\r\n");
  //endStreaming = true;
  return 0;

}


// 画像撮影
int take_picture()
{  
  CamImage TakePictureImg = theCamera.takePicture();
  char filename[16] = {0};
  sprintf(filename, "PICT%03d.JPG", shots_counter);
  printf("Save taken picture as ");
  printf(filename);
  printf("\r\n");

  // Remove the old file with the same file name as new created file, and create new file.
  theSD.remove(filename);
  File myFile = theSD.open(filename, FILE_WRITE);
  myFile.write(TakePictureImg.getImgBuff(), TakePictureImg.getImgSize());
  myFile.close();

  --shots_counter;

  printf("takepicture done\n");

  return 0;
}


// 画像評価の関数　アルゴリズムナンバー1
int evaluator_01() // 引数は判定する画像
{


  return 0;

}




// ストリーミングのコールバック関数
void streamingCB(CamImage img){

  // ストリーミングで取得したデータが正常の場合グレースケールに変換する
  if(!img.isAvailable()){
    printf("return\n");
  }
  printf("streaming\r\n");
  img.convertPixFormat(CAM_IMAGE_PIX_FMT_GRAY);

  uint8_t* imgbuf = (uint8_t*)img.getImgBuff();
  int ret;
  int sub_core_id;
  int8_t sndid = 10;
  uint32_t rcvdata;
  int8_t   rcvid;
  uint8_t pixel_luminance_0or255;

  if(shots_counter > 0){

    // 各サブコアに対して輝度判定を開始するイメージバッファのアドレスを送信する
    for(int subid = 1; subid == 4; subid++){

      ret = MP.Send(sndid, imgbuf[(subid-1)*1536], subid);
      if(ret == sndid){
        printf("Send data to SubCore : %d, subid\n");
      }
    }

    // 各サブコアでカウントした輝度0または255のピクセル数をメインコアで受け取る
    for(int subid = 1; subid == 4; subid++){
      ret = MP.Recv(&rcvid, &rcvdata, subid);
      pixel_luminance_0or255 += rcvdata;
    }

    printf("receve data : %d\n", pixel_luminance_0or255);

    // 輝度0または255のピクセル数が1割以下の場合は画像撮影を実行
    if(pixel_luminance_0or255 < 6144)
    {      
      // sketch_jun6a.inoでフラグ判定するためにtrueにする
      takePicture_runnning = true;
    }



  }

  return 0;
}

// ※フラグ判定してカメラ撮影するコードを書く
void camera_flag_loop()
{
  if(takePicture_runnning)
  {
    takePicture_runnning = false;
    take_picture();

    // 指定した回数の撮影をしたらstreamingを終了する。
    if (shots_counter <= 0)
    {
      // 画像判定で使用したサブコア(1～4)を停止します
      int ret = 0;
      for(int subid = 1; subid <= 4; subid++)
      {
        ret = MP.end(subid);
        if (ret < 0)
        {
          printf("MP.end(%d) error = %d\n", subid, ret);
        }
        printf("stop subcore %d\n", subid);
      }
      
      // streamingを終了します
      theCamera.startStreaming(false, streamingCB); 
 
    }

  }
}



// 画像判定で使用するサブコア(1～4)を起動させてストリーミングを開始する。
int camera_setup(){
  
  // サブコア(1～4)の起動
  int ret = 0;
  for(int subid = 1; subid <= 4; subid++){
    ret = MP.begin(subid);
    if (ret < 0){
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
  MP.RecvTimeout(MP_RECV_POLLING);

  // ストリーミングを開始
  theCamera.begin(1, CAM_VIDEO_FPS_5, 96, 64, CAM_IMAGE_PIX_FMT_YUV422, 7);
  theCamera.setStillPictureImageFormat(1280, 960, CAM_IMAGE_PIX_FMT_JPG, 7); 
  theCamera.startStreaming(true, streamingCB); 
  printf("start streaming\n");
  return 0;
}

int camera_command( int argc, char** argv)
{
  // コマンド引数が2以下の場合は不正コマンドと判断する
  if (argc < 2) {
        printf("not command\n");
        return 1;
    }
  
  // コマンド2が"takepicture"の場合 
  if(strcmp(argv[1], "takepicture") == 0)
  {
    printf("「takepicture」と一致しました\n");
    // 撮影枚数を total_picture_count として定義する
    // 撮影枚数はコマンド引数3で指定する　camera takepicture [撮影する枚数]
    // コマンド実行例：camera takepicture 5←撮影枚数
    shots_counter = atoi(argv[2]);

    // ストリーミングを実行する
    return camera_setup();
  } 
  else if( strcmp(argv[1], "streaming") == 0)
  {
    printf( "streaming\n");

    
  }
  else if( strcmp(argv[1], "set") == 0)
  {
    printf( "set parameter\n");
  }
  else if( strcmp(argv[1], "parameter") == 0)
  {
    printf( "parameter\n");
  }
  else 
  {
    printf("「takepicture」と一致しませんでした。入力された引数: %s\n", argv[1]);
  }

  printf( "\r\n");


// streamingコールバック関数でtake_picture()を実行せずにループ関数の中でbool変数で撮影タイミングを調整する　サブコア5はntshellが使用するためカメラでは使用しない

return 0;
}
