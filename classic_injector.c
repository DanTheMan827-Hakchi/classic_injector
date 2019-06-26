#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <SDL2/SDL.h>
#include <signal.h>
#include <pthread.h>

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

enum{
  btLoad=20,
  btSave=21,
  btQuit=22,
};

void pressjb(int b)
{
   SDL_Event event;
   event.type = SDL_JOYBUTTONDOWN;
   event.jbutton.button = b;
   SDL_PushEvent(&event);
}

void releasejb(int b)
{
   SDL_Event event;
   event.type = SDL_JOYBUTTONUP;
   event.jbutton.button = b;
   SDL_PushEvent(&event);
}

void pushjb(int b)
{
   pressjb(b);
   usleep(300000);
   releasejb(b);
}

void*_threadPressLoad(void*b)
{
   int i;
   for(i=0;i<3;++i)
      usleep(500000);
   pressjb(btLoad);
   for(i=0;i<3;++i)
      usleep(500000);
   releasejb(btLoad);
   return 0;
}

void signalHandler(int sig, siginfo_t *info, void *ucontext);
typedef void (*__sigaction_t)(int sig, siginfo_t *info, void *ucontext);

static struct
{
    int signal;
    __sigaction_t handler;
    struct sigaction oldhandler;
}handlers[]={
    {SIGTERM,signalHandler,{{0},{{0}},0,0}},
    {SIGHUP,signalHandler,{{0},{{0}},0,0}},
    {SIGINT,signalHandler,{{0},{{0}},0,0}},
    {SIGUSR1,signalHandler,{{0},{{0}},0,0}},
    {SIGUSR2,signalHandler,{{0},{{0}},0,0}},
    {SIGPIPE,(__sigaction_t)SIG_IGN,{{0},{{0}},0,0}}
};

struct sigaction*oldhandler(int sig)
{
   for(size_t i=0;i<ARRAY_SIZE(handlers);++i)
   {
      if (handlers[i].signal == sig)
         return &handlers[i].oldhandler;
   }
   return 0;
}

typedef struct{
   int sig;
   siginfo_t *info;
   void *ucontext;
} sigdata_t;

void*_threadSignalHandler(void*_sig)
{
   uintptr_t sig=(uintptr_t)_sig;
   FILE *fp;
   int i;
   switch(sig)
   {
   case SIGTERM:
   case SIGINT:
   case SIGHUP:
      pressjb(btSave);
      usleep(500000);
      usleep(500000);
      printf("quit()\n");
      releasejb(btSave);
      pushjb(btQuit);
      for(i=0;i<10;++i)
         usleep(500000);
      exit(1);
      break;
   case SIGUSR1:
      pushjb(19);
      break;
   case SIGUSR2:
      if ((fp = fopen("control", "r")) != NULL)
      {
         char str[128];
         unlink("control");
         while ((!feof(fp)) && (fgets(str, sizeof(str), fp)!=0))
         {
            if (memcmp(str,"push",4)==0)
               pushjb(strtol(str+4,0,10));
            if (memcmp(str,"press",5)==0)
               pressjb(strtol(str+5,0,10));
            if (memcmp(str,"release",7)==0)
               releasejb(strtol(str+7,0,10));
            if (memcmp(str,"msleep",6)==0)
               usleep(strtol(str+6,0,10)*1000);
         }
         fclose(fp);
      }
      break;
   default:
      break;
   }
   return 0;
}

void signalHandler(int sig, siginfo_t *info, void *ucontext)
{
   pthread_t tid;
   pthread_create(&tid, 0, _threadSignalHandler, (void*)sig);
   pthread_detach(tid);
}

void __init(void) __attribute__((constructor));
void __init(void)
{
   printf("Classic Injector Loaded\n\n");
   printf("DanTheMan827 was here\n");
   printf("madmonkey was here\n\n");

   pthread_t tid;
   pthread_create(&tid, 0, _threadPressLoad, (void*)tid);
   pthread_detach(tid);

   struct sigaction handler;
   memset(&handler,0,sizeof(handler));
   sigemptyset(&handler.sa_mask);
   for(size_t i=0;i<ARRAY_SIZE(handlers);++i)
   {
      sigaddset(&handler.sa_mask,handlers[i].signal);
   }
   handler.sa_flags=SA_RESTART|SA_SIGINFO;
   for(size_t i=0;i<ARRAY_SIZE(handlers);++i)
   {
      handler.sa_sigaction=handlers[i].handler;
      sigaction(handlers[i].signal,&handler,&handlers[i].oldhandler);
   }
}
