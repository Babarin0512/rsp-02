#if (SUBCORE != 3)
#error "Core selection is wrong!!"
#endif
#include <MP.h>

#define MY_MSGID    10

volatile int status; /* 0:ready, 1:busy */ 
int count_pixel = 0;

void setup()
{
  int ret = 0;

  ret = MP.begin();
   if (ret < 0) {
   // errorLoop(2);
  }
}

void loop()
{
  int      ret;
  int8_t   msgid;
  int count_pixel = 0;
  uint32_t* rcvimgbuf;

  if(status != 0) return;

  /* Recve YUV422's file address */
  ret = MP.Recv(&msgid, rcvimgbuf);
    if (ret > 0) {
      printf("recv_done");
    }
    else {
      printf("Nothing date in packet.\n");
      return 0; 
    }

  if (status == 0) {

    /* status -> busy */
    status = 1;

    
    // SubCore ga 1 to kateisuru.(romaji)
    int EndPixel = 1227 * 1; //keisansuru pixel no kosuu(romaji)

    /* Count 0 or 255 pixel */
    for (int n = rcvimgbuf; n < rcvimgbuf[0] + EndPixel; n++){
      if (rcvimgbuf[n] > 255){
      count_pixel++;
    }
    else if(rcvimgbuf[n] < 0){
      count_pixel++;
    }
    }
    

    /* Send to MainCore */
    ret = MP.Send(msgid, count_pixel);
    if (ret < 0) {
      printf("MP.Send error = %d\n", ret);
    }

    /* status -> ready */
    status = 0;

    /* Reset count_pixel */
    count_pixel = 0;
  }

}
