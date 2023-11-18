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

int take_picture_count = 0;
int total_picture_count;


int8_t   sndid = 100;
int8_t   rcvid; 

int camera_end(){ 
  theCamera.end();
  printf( "camera end\n");
  printf( "\r\n");
  //endStreaming = true;
  return 0;

}

int take_picture(){
  // total_picture_countはポインタ参照する
  /*if(!AbleTakePicture){
      printf("return");
      return;
    }
  AbleTakePicture = false;
  */
  CamImage TakePictureImg = theCamera.takePicture();
  if(TakePictureImg.isAvailable()){
    char filename[16] = {0};
    sprintf(filename, "PICT%03d.JPG", take_picture_count);
    printf("Save taken picture as ");
    printf(filename);
    printf("\r\n");

    // Remove the old file with the same file name as new created file, and create new file.
    theSD.remove(filename);
    File myFile = theSD.open(filename, FILE_WRITE);
    myFile.write(TakePictureImg.getImgBuff(), TakePictureImg.getImgSize());
    myFile.close();

    //change AbleTakePicture=fales;
    printf("AbleTakePicture = true\r\n");
    //AbleTakePicture = true;
  }//else{
    //printf("Feild to take picture");
  //}

  return 0;
}

void streamingCB(CamImage img){
  if(!img.isAvailable()){
    printf("return");
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

  if(total_picture_count > take_picture_count){
    take_picture_count++;
    printf("take_picture_count : %d\n", take_picture_count);

    /* send imgbuf address to subcore */
    for(int subid = 1; subid == 5; subid++){

      ret = MP.Send(sndid, imgbuf[(subid-1)*1228], subid);
      if(ret == sndid){
        printf("Send data to SubCore : %d, subid\n");
      }
    }

    /* Recve count pixel(Luminance 0 or 255 pixel)*/
    for(int subid = 1; subid == 5; subid++){
      ret = MP.Recv(&rcvid, &rcvdata, subid);
      pixel_luminance_0or255 += rcvdata;
    }

    printf("receve data : %d\n", pixel_luminance_0or255);
    if(pixel_luminance_0or255 < 6144){
      //AbleTakePicture = true;
      //return pixel_luminance_0or255;
      take_picture();
    }else{
      //AbleTakePicture = false;
    }

  }else {
    int ret = 0;
    for(int subid = 1; subid <= 5; subid++){
      ret = MP.end(subid);
      if (ret < 0){
        printf("MP.end(%d) error = %d\n", subid, ret);
      }
      printf("stop subcore %d\n", subid);
    }
    take_picture_count = 0;
    camera_end();
    //theCamera.startStreaming(false, streamingCB); 
    //theCamera.end();
    //printf( "\r\n");
    

   
  }

  return 0;
}



int camera_setup(){
  
  // サブコア(1～5)の起動
  int ret = 0;
  for(int subid = 1; subid <= 5; subid++){
    ret = MP.begin(subid);
    if (ret < 0){
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
  MP.RecvTimeout(MP_RECV_POLLING);

  /* Camera set up */
  theCamera.begin(1, CAM_VIDEO_FPS_5, 96, 64, CAM_IMAGE_PIX_FMT_YUV422, 7);
  theCamera.setStillPictureImageFormat(1280, 960, CAM_IMAGE_PIX_FMT_JPG, 7); 
  theCamera.startStreaming(true, streamingCB); 
  printf("Camera set up.\n");
  return 0;
}



//int auto_take_picture(int* total_picture_count){
int auto_take_picture(){

  //　撮影カウントの初期化
  take_picture_count = 0;

  camera_setup();
  
  return 0;
}

int camera_command( int argc, char** argv)
{
  if (argc < 2) {
        printf("not command\n");
        return 1;
    }
  
  if(strcmp(argv[1], "takepicture") == 0) {
    int picture_count = 0;

    printf("「takepicture」と一致しました\n");
    //picture_count = atoi(argv[2]);
    total_picture_count = atoi(argv[2]);
    printf("Take %d picture\r\n", picture_count);
    //auto_take_picture(&picture_count);
    auto_take_picture();
    } else {
        printf("「takepicture」と一致しませんでした。入力された引数: %s\n", argv[1]);
    }

  printf( "\r\n");

return 0;
}