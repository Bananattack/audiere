/* MPEG Sound library

   (C) 1997 by Jung woo-jae */

// Mpegtoraw.cc
// Server which get mpeg format and put raw format.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PTHREADEDMPEG
#include <pthread.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "mpegsound.h"
#include "mpegsound_locals.h"

#define MY_PI 3.14159265358979323846

Mpegtoraw::Mpegtoraw(Soundinputstream *loader,Soundplayer *player)
{
  __errorcode=SOUND_ERROR_OK;
  frameoffsets=NULL;

  forcetomonoflag=false;
  downfrequency=0;

  this->loader=loader;
  this->player=player;
}

Mpegtoraw::~Mpegtoraw()
{
  if(frameoffsets)delete [] frameoffsets;
}

#ifndef WORDS_BIGENDIAN
#define _KEY 0
#else
#define _KEY 3
#endif

int Mpegtoraw::getbits(int bits)
{
  union
  {
    char store[4];
    int current;
  }u;
  int bi;

  if(!bits)return 0;

  u.current=0;
  bi=(bitindex&7);
  u.store[_KEY]=buffer[bitindex>>3]<<bi;
  bi=8-bi;
  bitindex+=bi;

  while(bits)
  {
    if(!bi)
    {
      u.store[_KEY]=buffer[bitindex>>3];
      bitindex+=8;
      bi=8;
    }

    if(bits>=bi)
    {
      u.current<<=bi;
      bits-=bi;
      bi=0;
    }
    else
    {
      u.current<<=bits;
      bi-=bits;
      bits=0;
    }
  }
  bitindex-=bi;

  return (u.current>>8);
}

void Mpegtoraw::setforcetomono(bool flag)
{
  forcetomonoflag=flag;
}

void Mpegtoraw::setdownfrequency(int value)
{
  downfrequency=0;
  if(value)downfrequency=1;
}

bool Mpegtoraw::getforcetomono(void)
{
  return forcetomonoflag;
}

int Mpegtoraw::getdownfrequency(void)
{
  return downfrequency;
}

int  Mpegtoraw::getpcmperframe(void)
{
  int s;

  s=32;
  if(layer==3)
  {
    s*=18;
    if(version==0)s*=2;
  }
  else
  {
    s*=SCALEBLOCK;
    if(layer==2)s*=3;
  }

  return s;
}

inline void Mpegtoraw::flushrawdata(void)
#ifdef PTHREADEDMPEG
{
  if(threadflags.thread)
  {
    if(((threadqueue.tail+1)%threadqueue.framenumber)==threadqueue.head)
    {
      while(((threadqueue.tail+1)%threadqueue.framenumber)==threadqueue.head)
	usleep(200);
    }
    memcpy(threadqueue.buffer+(threadqueue.tail*RAWDATASIZE),rawdata,
	   RAWDATASIZE*sizeof(short int));
    threadqueue.sizes[threadqueue.tail]=(rawdataoffset<<1);
    
    if(threadqueue.tail>=threadqueue.frametail)
      threadqueue.tail=0;
    else threadqueue.tail++;
  }
  else
  {
    player->putblock((char *)rawdata,rawdataoffset<<1);
    currentframe++;
  }
  rawdataoffset=0;
}
#else
{
  player->putblock((char *)rawdata,rawdataoffset<<1);
  currentframe++;
  rawdataoffset=0;
};
#endif

inline void stripfilename(char *dtr,char *str,int max)
{
  char *ss;
  int p=0,s=0;

  for(;str[p];p++)
    if(str[p]=='/')
    {
      p++;
      s=p;
    }

  ss=str+s;
  for(p=0;p<max && ss[p];p++)dtr[p]=ss[p];
  dtr[p]=0;
}

// Convert mpeg to raw
// Mpeg headder class
void Mpegtoraw::initialize(char *)
{
  static bool initialized=false;

  register int i;
  register REAL *s1,*s2;
  REAL *s3,*s4;

  scalefactor=SCALE;
  calcbufferoffset=15;
  currentcalcbuffer=0;

  s1=calcbufferL[0];s2=calcbufferR[0];
  s3=calcbufferL[1];s4=calcbufferR[1];
  for(i=CALCBUFFERSIZE-1;i>=0;i--)
    calcbufferL[0][i]=calcbufferL[1][i]=
    calcbufferR[0][i]=calcbufferR[1][i]=0.0;

  if(!initialized)
  {
    for(i=0;i<16;i++)hcos_64[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/64.0));
    for(i=0;i< 8;i++)hcos_32[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/32.0));
    for(i=0;i< 4;i++)hcos_16[i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/16.0));
    for(i=0;i< 2;i++)hcos_8 [i]=1.0/(2.0*cos(MY_PI*double(i*2+1)/ 8.0));
    hcos_4=1.0/(2.0*cos(MY_PI*1.0/4.0));
    initialized=true;
  }

  layer3initialize();

  currentframe=decodeframe=0;
  if(loadheader())
  {
    totalframe=(loader->getsize()+framesize-1)/framesize;
    loader->setposition(0);
  }
  else totalframe=0;


  if(frameoffsets)delete [] frameoffsets;

  if(totalframe>0)
  {
    frameoffsets=new int[totalframe];
    for(i=totalframe-1;i>=0;i--)
      frameoffsets[i]=0;
  }
  else frameoffsets=NULL;

#ifdef PTHREADEDMPEG
  threadflags.thread=false;
  threadqueue.buffer=NULL;
  threadqueue.sizes=NULL;
#endif
};

void Mpegtoraw::setframe(int framenumber)
{
  int pos=0;

  if(frameoffsets==NULL)return;
  if(framenumber==0)pos=frameoffsets[0];
  else
  {
    if(framenumber>=totalframe)framenumber=totalframe-1;
    pos=frameoffsets[framenumber];
    if(pos==0)
    {
      int i;

      for(i=framenumber-1;i>0;i--)
	if(frameoffsets[i]!=0)break;

      loader->setposition(frameoffsets[i]);

      while(i<framenumber)
      {
	loadheader();
	i++;
	frameoffsets[i]=loader->getposition();
      }
      pos=frameoffsets[framenumber];
    }
  }

  clearbuffer();
  loader->setposition(pos);
  decodeframe=currentframe=framenumber;
}

void Mpegtoraw::clearbuffer(void)
{
#ifdef PTHREADEDMPEG
  if(threadflags.thread)
  {
    threadflags.criticalflag=false;
    threadflags.criticallock=true;
    while(!threadflags.criticalflag)usleep(1);
    threadqueue.head=threadqueue.tail=0;
    threadflags.criticallock=false;
  }
#endif
  player->abort();
  player->resetsoundtype();
}

bool Mpegtoraw::loadheader(void)
{
  register int c;
  bool flag;

  sync();

// Synchronize
  flag=false;
  do
  {

    if((c=loader->getbytedirect())<0)break;

    if(c==0xff) {
      while(!flag)
      {
	if((c=loader->getbytedirect())<0)
	{
	  flag=true;
	  break;
	}
	if((c&0xf0)==0xf0)
	{
	  flag=true;
	  break;
	}
	else if(c!=0xff)break;
      }
    }
    else if (c=='I') { // possible ID3v2 tag
        char buf[10];
        int c2,c3;
        int length;

        if((c2=loader->getbytedirect())<0) break;
        if (c2=='D') {
            if((c3=loader->getbytedirect())<0) break;
            if(c3=='3') {
                  // OK, found ID3v2 tag.
                if (!loader->_readbuffer(buf,7)) break;
                
                  // Compute number of bytes to skip
                length=((buf[3]&0x7f)<<21) + ((buf[4]&0x7f)<<14) +
                    ((buf[5]&0x7f)<<7) + (buf[6]&0x7f);
                
       //printf("Found ID3v2 tag; skipping %d bytes\n",length);
                while (length-->0) {
                    if(loader->getbytedirect()<0) {
                        break;
                    }
                }
            }
        }
    }
    //else {
    //    printf("garbage character: %02x\n",c);
    //}
  }while(!flag);

  if(c<0)return seterrorcode(SOUND_ERROR_FINISH);



// Analyzing
  c&=0xf;
  protection=c&1;
  layer=4-((c>>1)&3);
  version=(_mpegversion)((c>>3)^1);

  c=((loader->getbytedirect()))>>1;
  padding=(c&1);             c>>=1;
  frequency=(_frequency)(c&2); c>>=2;
  bitrateindex=(int)c;
  if(bitrateindex==15)return seterrorcode(SOUND_ERROR_BAD);

  c=((unsigned int)(loader->getbytedirect()))>>4;
  extendedmode=c&3;
  mode=(_mode)(c>>2);


// Making information
  inputstereo= (mode==single)?0:1;
  if(forcetomonoflag)outputstereo=0; else outputstereo=inputstereo;

  /*  if(layer==2)
    if((bitrateindex>=1 && bitrateindex<=3) || (bitrateindex==5)) {
      if(inputstereo)return seterrorcode(SOUND_ERROR_BAD); }
    else if(bitrateindex==11 && mode==single)
    return seterrorcode(SOUND_ERROR_BAD); */

  channelbitrate=bitrateindex;
  if(inputstereo)
    if(channelbitrate==4)channelbitrate=1;
    else channelbitrate-=4;

  if(channelbitrate==1 || channelbitrate==2)tableindex=0; else tableindex=1;

  if(layer==1)subbandnumber=MAXSUBBAND;
  else
  {
    if(!tableindex)
      if(frequency==frequency32000)subbandnumber=12; else subbandnumber=8;
    else if(frequency==frequency48000||
	    (channelbitrate>=3 && channelbitrate<=5))
      subbandnumber=27;
    else subbandnumber=30;
  }

  if(mode==single)stereobound=0;
  else if(mode==joint)stereobound=(extendedmode+1)<<2;
  else stereobound=subbandnumber;

  if(stereobound>subbandnumber)stereobound=subbandnumber;

  // framesize & slots
  if(layer==1)
  {
    framesize=(12000*bitrate[version][0][bitrateindex])/
              frequencies[version][frequency];
    if(frequency==frequency44100 && padding)framesize++;
    framesize<<=2;
  }
  else
  {
    framesize=(144000*bitrate[version][layer-1][bitrateindex])/
      (frequencies[version][frequency]<<version);
    if(padding)framesize++;
    if(layer==3)
    {
      if(version)
	layer3slots=framesize-((mode==single)?9:17)
	                     -(protection?0:2)
	                     -4;
      else
	layer3slots=framesize-((mode==single)?17:32)
	                     -(protection?0:2)
	                     -4;
    }
  }

  if(!fillbuffer(framesize-4))seterrorcode(SOUND_ERROR_FILEREADFAIL);

  if(!protection)
  {
    getbyte();                      // CRC, Not check!!
    getbyte();
  }


  if(loader->eof())return seterrorcode(SOUND_ERROR_FINISH);

  return true;
}

/***************************/
/* Playing in multi-thread */
/***************************/
#ifdef PTHREADEDMPEG
/* Player routine */
void Mpegtoraw::threadedplayer(void)
{
  while(!threadflags.quit)
  {
    while(threadflags.pause || threadflags.criticallock)
    {
      threadflags.criticalflag=true;
      usleep(200);
    }

    if(threadqueue.head!=threadqueue.tail)
    {
      player->putblock(threadqueue.buffer+threadqueue.head*RAWDATASIZE,
      		       threadqueue.sizes[threadqueue.head]);
      currentframe++;
      if(threadqueue.head==threadqueue.frametail)
	threadqueue.head=0;
      else threadqueue.head++;
    }
    else
    {
      if(threadflags.done)break;  // Terminate when done
      usleep(200);
    }
  }
  threadflags.thread=false;
}

static void *threadlinker(void *arg)
{
  ((Mpegtoraw *)arg)->threadedplayer();

  return NULL;
}

bool Mpegtoraw::makethreadedplayer(int framenumbers)
{
  threadqueue.buffer=
    (short int *)malloc(sizeof(short int)*RAWDATASIZE*framenumbers);
  if(threadqueue.buffer==NULL)
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
  threadqueue.sizes=(int *)malloc(sizeof(int)*framenumbers);
  if(threadqueue.sizes==NULL)
    seterrorcode(SOUND_ERROR_MEMORYNOTENOUGH);
  threadqueue.framenumber=framenumbers;
  threadqueue.frametail=framenumbers-1;
  threadqueue.head=threadqueue.tail=0;


  threadflags.quit=threadflags.done=
  threadflags.pause=threadflags.criticallock=false;

  threadflags.thread=true;
  if(pthread_create(&thread,0,threadlinker,this))
    seterrorcode(SOUND_ERROR_THREADFAIL);

  return true;
}

void Mpegtoraw::freethreadedplayer(void)
{
  threadflags.criticallock=
  threadflags.pause=false;
  threadflags.done=true;               // Terminate thread player
  while(threadflags.thread)usleep(10); // Wait for done...
  if(threadqueue.buffer)free(threadqueue.buffer);
  if(threadqueue.sizes)free(threadqueue.sizes);
}




void Mpegtoraw::stopthreadedplayer(void)
{
  threadflags.quit=true;
};

void Mpegtoraw::pausethreadedplayer(void)
{
  threadflags.pause=true;
};

void Mpegtoraw::unpausethreadedplayer(void)
{
  threadflags.pause=false;
};


bool Mpegtoraw::existthread(void)
{
  return threadflags.thread;
}

int  Mpegtoraw::getframesaved(void)
{
  if(threadqueue.framenumber)
    return
      ((threadqueue.tail+threadqueue.framenumber-threadqueue.head)
       %threadqueue.framenumber);
  return 0;
}

#endif


#include <iostream.h>
// Convert mpeg to raw
bool Mpegtoraw::run(int frames)
{
  clearrawdata();
  if(frames<0)lastfrequency=0;

  for(;frames;frames--)
  {
    if(totalframe>0)
    {
      if(decodeframe<totalframe)
	frameoffsets[decodeframe]=loader->getposition();
    }

    if(loader->eof())
    {
      seterrorcode(SOUND_ERROR_FINISH);
      break;
    }
    if(loadheader()==false)break;

    if(frequency!=lastfrequency)
    {
      if(lastfrequency>0)seterrorcode(SOUND_ERROR_BAD);
      lastfrequency=frequency;
    }
    if(frames<0)
    {
      frames=-frames;
      player->setsoundtype(outputstereo,16,
			   frequencies[version][frequency]>>downfrequency);
    }

    decodeframe++;

    if     (layer==3)extractlayer3();
    else if(layer==2)extractlayer2();
    else if(layer==1)extractlayer1();

    flushrawdata();
    if(player->geterrorcode())seterrorcode(geterrorcode());
  }

  return (geterrorcode()==SOUND_ERROR_OK);
}
