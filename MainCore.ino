#include <SDHCI.h>
#include <stdio.h>
#include <stdbool.h>
#include <Camera.h>
#include <MP.h>

#define BAUDRATE (115200)
#define TOTAL_PICTURE_COUNT (10)
#define YUV_WIDETH (96)
#define YUV_HEIGHT (64)



SDClass theSD;
int take_picture_count = 0;
int total_picture_count = 10;
bool AbleTakePicture;
bool waiteStreaming;



int8_t   sndid = 100; /*send Address's packet subcore.*/
int8_t   rcvid; /**/
//uint8_t pixel_luminance_0or255;



/* Print error message */
void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

/*camera call back*/
int StreamingCB(CamImage img){
 if(!img.isAvailable() || waiteStreaming){
      printf("return");
      return;
      // pixel_luminance is lower than 10% => take picture.
 }

 printf("streaming\n");

 //if(!AbleTakePicture) return;
 
//if(take_picture_count == total_picture_count) return;

 //printf("start streaming.\n");

img.convertPixFormat(CAM_IMAGE_PIX_FMT_GRAY);

uint8_t* imgbuf = (uint8_t*)img.getImgBuff();

int ret;
int sub_core_id;
int8_t sndid = 10;
uint32_t rcvdata;
int8_t   rcvid;
uint8_t pixel_luminance_0or255;

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
  AbleTakePicture = true;

  return pixel_luminance_0or255;
}else{
  AbleTakePicture = false;
return 0;
}
}


/* take picture */
int Take_picture(){
    if(!AbleTakePicture){
      printf("return");
      return;
      // pixel_luminance is lower than 10% => take picture.
    }
      //change AbleTakePicture=false;
      AbleTakePicture = false;
      printf("Able Take Picture = false\n");

      printf("start take picture : %d\n", take_picture_count);
      CamImage TakePictureImg = theCamera.takePicture();

      if(TakePictureImg.isAvailable()){
        //create file name
        char filename[16] = {0};
        sprintf(filename, "PICT%d.JPG", take_picture_count);

        printf("Save taken picture as ");
        printf(filename);
        //println("");

        //Remove the old file with the same file name as new created file, and create new file.
        theSD.remove(filename);
        File myFile = theSD.open(filename, FILE_WRITE);
        myFile.write(TakePictureImg.getImgBuff(), TakePictureImg.getImgSize());
        myFile.close();

        //take_picture_count + 1. 
        take_picture_count++;
        printf("take_picture_count : %d\n", take_picture_count);

        //wait 1.0 sec
        //sleep(1);

        //change AbleTakePicture=fales;
        printf("AbleTakePicture = true\n");
        AbleTakePicture = true;

        waiteStreaming = false;
        

        
      }
      else
      {
        printf("Filed to take picture");
      }

      
    
    return 0;
  }


void setup()
{
  int ret = 0;
  

  while (!theSD.begin())
  {
    // wait until SD card is mounted.
    printf("Insert SD card.");
  }

  /*Boot SubCore */
  for (int subid = 1; subid <= 5; subid++){
    ret = MP.begin(subid);
    if (ret < 0){
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
  MP.RecvTimeout(MP_RECV_POLLING);
  
  
  /* Camera set up */
  AbleTakePicture = false;
  waiteStreaming = false;
  theCamera.begin(1, CAM_VIDEO_FPS_5, YUV_WIDETH, YUV_HEIGHT, CAM_IMAGE_PIX_FMT_YUV422, 7);//streaming Format set up.dainihikisu 5
  theCamera.setStillPictureImageFormat(1280, 960, CAM_IMAGE_PIX_FMT_JPG, 7); //JPEG Format set up.
  theCamera.startStreaming(true, StreamingCB); 
  printf("Camera set up.\n");

  printf("take_picture\n");

  

  while(take_picture_count >= 10){
    
     waiteStreaming = true;
    printf("waiteStreaming.\n");
    printf("%d\n", StreamingCB);
    Take_picture();
  }
}  



void loop(){
  /*
  if(take_picture_count >= TOTAL_PICTURE_COUNT){
    printf("take_picture_count=10");
    //take_picture_count = 0;
    // STOP Streaming.
    AbleTakePicture = false;
    printf("Take picture done.\n");
    theCamera.startStreaming(false, StreamingCB);
  }else if(AbleTakePicture){
    waiteStreaming = true;
    printf("waiteStreaming.\n");
    printf("%d\n", StreamingCB);
    Take_picture();
    
    
  }else return;*/
  }
  



// MainCore set up.  
/*
void setup() {

  int ret = 0;
  int usbid;

  while (!theSD.begin()){
    // wait until SD card is mounted.
    printf("Insert SD card.");
  }
  
  // Camera set up
  AbleTakePicture=true;
  theCamera.begin(1, 1, YUV_WIDETH, YUV_HEIGHT, CAM_IMAGE_PIX_FMT_YUV422, 7);//streaming Format set up.dainihikisu 5
  theCamera.setStillPictureImageFormat(1280, 960, CAM_IMAGE_PIX_FMT_JPG, 7); //JPEG Format set up.
  theCamera.startStreaming(true, StreamingCB);

  */
  /*Boot SubCore */

  /*
  for (subid = 1; subid <= 5; subid++){
    ret = MP.begin(subid);
    if (ret < 0){
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
  MP.RecvTimeout(MP_RECV_POLLING);
  
  printf(" Camera set up : OK \n");
  
}


void loop(){
  if(take_picture_count == TOTAL_PICTURE_COUNT)
  {
    printf("take_picture_count=10");
    take_picture_count = 0;
    // STOP Streaming.
    theCamera.startStreaming(false, StreamingCB);
  }
  
}
*/



/*
#include <SDHCI.h>
#include <stdio.h>
#include <stdbool.h>
#include <Camera.h>

#define BAUDRATE (115200)
#define TOTAL_PICTURE_COUNT (10)
#define YUV_WIDETH (96)
#define YUV_HEIGHT (64)



SDClass theSD;
int take_picture_count = 0;
bool AbleTakePicture;
*/

/*
 * Print error message
 */

/*
void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

void StreamingCB(CamImage img){
 if(!img.isAvailable()){
  printf("Can not streaming.\n");
  return;
 }
if(take_picture_count == TOTAL_PICTURE_COUNT) return;
img.convertPixFormat(CAM_IMAGE_PIX_FMT_GRAY);

uint32_t* imgbuf = (uint32_t*)img.getImgBuff();
uint8_t grayImg[YUV_WIDETH*YUV_HEIGHT];
uint8_t count_black_pixel;
uint8_t count_white_pixel;
*/

/*
int ret;
int subid;
int8_t msgid;
MyPacket *packet;
*/

/* send imgbuf's address to SubCore. */
/*
for(subid = 1; subid == 5; subid++){
  MP.Send(&msgid, imgbuf[(subid-1)*1228], subid);  
}
*/

/*Recv count pixel */
/*
for(subid = 1; subid == 5; subid++){
  ret = MP.Recv(&msgid, &packet, subid);
    if (ret > 0) {
      printf("%s\n", packet->message);
      /* status -> ready */
      /*
      packet->status = 0;
    } 
}



printf("count Luminance\n");


    printf("Luminance_0 :%d\n", count_black_pixel);
    printf("Luminance_255 :%d\n", count_white_pixel);

    
   

    if(!AbleTakePicture){
      printf("return");
      return;
    }else if(count_black_pixel<=614 || count_white_pixel<=614){
      //change AbleTakePicture=false;
      AbleTakePicture = false;

      printf("start take picture :%d\n", take_picture_count);
      CamImage TakePictureImg = theCamera.takePicture();

      if(TakePictureImg.isAvailable()){
        //create file name
        char filename[16] = {0};
        sprintf(filename, "PICT%d.JPG", take_picture_count);

        Serial.print("Save taken picture as ");
        Serial.print(filename);
        Serial.println("");

        //Remove the old file with the same file name as new created file, and create new file.
        theSD.remove(filename);
        File myFile = theSD.open(filename, FILE_WRITE);
        myFile.write(TakePictureImg.getImgBuff(), TakePictureImg.getImgSize());
        myFile.close();

        //take_picture_count + 1. 
        take_picture_count++;

        //wait 1.0 sec
        sleep(1);

        //change AbleTakePicture=fales;
        AbleTakePicture = true;

        
      }
      else
      {
        Serial.println("Filed to take picture");
      }

      
    }
  }


    
// MainCore set up.  
void setup() {

  int ret = 0;
  int usbid;

  while (!theSD.begin())
  {
    // wait until SD card is mounted.
    printf("Insert SD card.");
  }
  
  // Camera set up
  AbleTakePicture=true;
  theCamera.begin(1, 1, YUV_WIDETH, YUV_HEIGHT, CAM_IMAGE_PIX_FMT_YUV422, 7);//streaming Format set up.dainihikisu 5
  theCamera.setStillPictureImageFormat(1280, 960, CAM_IMAGE_PIX_FMT_JPG, 7); //JPEG Format set up.
  theCamera.startStreaming(true, StreamingCB);
  */
  /*Boot SubCore */
  /*
  for (subid = 1; subid <= 5; subid++){
    ret = MP.begin(subid);
    if (ret < 0){
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
  }
  MP.RecvTimeout(MP_RECV_POLLING);
  
  printf("Camera set up.\n");
  
}

void loop(){
  if(take_picture_count == TOTAL_PICTURE_COUNT)
  {
    printf("take_picture_count=10");
    take_picture_count = 0;
    // STOP Streaming.
    theCamera.startStreaming(false, StreamingCB);
  }
  
}

*/






//
