#include <pthread.h>
#include <ntlibc.h>
#include <ntopt.h>
#include <ntshell.h>
#include <ntstdio.h>
#include <arduino.h>

// コマンド実行関数のヘッダ
#include "echo_command.hpp"
// コマンドを追加するときは、ヘッダを増やしていく
// #include "hoge_command.hpp"
#include "camera_command.hpp"

typedef struct
{
  const char* name;
  int (*cmd)(int,char**);
} command_t;

// コマンドテーブル
static const command_t command[] =
{
  // echoコマンド
  {"echo",echo_command},
  // コマンドを追加するときは、要素を増やしていく
  // {"hoge",hoge_command},
  {"camera",camera_command},

};
// コマンドの数
const static int command_num = sizeof(command)/sizeof(command[0]);

static int read(char* buf, int cnt, void* extobj)
{
  for( int i=0; i<cnt; i++)
  {
    Serial.readBytes( &buf[i], 1);
  }
  return cnt;
}
static int write(const char* buf, int cnt, void* extobj)
{
  for( int i=0; i<cnt; i++)
  {
    Serial.write( buf[i]);
  }
  return cnt;
}
static int command_func( int argc, char **argv, void* extobj)
{
  if( argc < 1) return -1;
  for( int i=0; i<command_num; i++)
  {
    if( strcmp( command[i].name, argv[0])==0)
    {
      return command[i].cmd(argc,argv);
    }
  }
  return -1;
}

static int cb(const char* text, void* extobj)
{
  ntopt_parse( text, command_func, extobj);
  return 0;
}

static ntshell_t ntshell;

// pthread実行関数
static void shell( void *arg)
{
  ntshell_set_prompt( &ntshell, "RSP02>");
  ntshell_execute( &ntshell);
}

// ntshell初期化関数
// mainから呼ばれると、ntshellが動くように良きに計らう
void ntshell_setup()
{
  Serial.begin(115200);
  Serial.setTimeout( -1);
  ntshell_init(
    &ntshell,
    read,
    write,
    cb,
    nullptr
  );

  pthread_t shell_th;
  pthread_create( &shell_th, NULL, shell, (void*)NULL);
}
