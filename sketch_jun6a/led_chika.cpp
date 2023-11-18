#include <string.h>
#include <arduino.h>

typedef struct
{
  uint8_t pat;
  int dur;
}seq_t;

const static uint8_t l0 = 0x01;
const static uint8_t l1 = 0x02;
const static uint8_t l2 = 0x04;
const static uint8_t l3 = 0x08;
const static uint8_t off = 0x00;
static const seq_t sequence[] =
{
  { off, 100},
  { l0, 100},
  { l1, 100},
  { l2, 100},
  { l3, 100},
  { l2, 100},
  { l1, 100},
  { l0, 100},
  { off, 100},
  { l3, 100},
  { l2, 100},
  { l1, 100},
  { l0, 100},
  { l1, 100},
  { l2, 100},
  { l3, 100},
  { off, 100},
  { l0, 100},
  { l0|l1, 100},
  { l0|l1|l2, 100},
  { l0|l1|l2|l3, 100},
  {    l1|l2|l3, 100},
  {       l2|l3, 100},
  {          l3, 100},
  {       l2|l3, 100},
  {    l1|l2|l3, 100},
  { l0|l1|l2|l3, 100},
  { l0|l1|l2   , 100},
  { l0|l1      , 100},
  { l0         , 100},
  { l0|l1      , 100},
  { l0|l1|l2   , 100},
  { l0|l1|l2|l3, 100},
  { l0|l1|l2   , 100},
  { l0|l1      , 100},
  { l0         , 100},
};
const size_t n = sizeof(sequence)/sizeof(sequence[0]);

void led_setup()
{
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
}


static const uint8_t ledpat[4] =
{
  LED0, LED1, LED2, LED3,
};

static void led_seq( seq_t seq)
{
  for( int i=0; i<4; i++)
  {
    uint8_t mask = 1<<i;
    seq.pat & mask ? digitalWrite( ledpat[i], HIGH) : digitalWrite( ledpat[i], LOW);
  }
    usleep( seq.dur*1000);
}
static int ii = 0;
void led_loop() {
    led_seq(sequence[ii]);
    if( ++ii==n) ii=0;
    return;
    digitalWrite(LED0, HIGH);
    usleep(100*1000);
    digitalWrite(LED1, HIGH);
    usleep(100*1000);
    digitalWrite(LED2, HIGH);
    usleep(100*1000);
    digitalWrite(LED3, HIGH);
    usleep(1000*1000);

    digitalWrite(LED0, LOW);
    usleep(100*1000);
    digitalWrite(LED1, LOW);
    usleep(100*1000);
    digitalWrite(LED2, LOW);
    usleep(100*1000);
    digitalWrite(LED3, LOW);
    usleep(1000*1000);
}