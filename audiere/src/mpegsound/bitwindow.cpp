/* MPEG Sound library

   (C) 1997 by Jung woo-jae */

// Bitwindow.cc
// It's bit reservior for MPEG layer 3

#include "mpegsound.h"

#ifndef WORDS_BIGENDIAN
const int _KEY = 0;
#else
const int _KEY = 3;
#endif

int Mpegbitwindow::getbits(int bits)
{
  if(!bits)
    return 0;

  union
  {
    char store[4];
    int current;
  } u;
  u.current=0;

  int bi=(bitindex&7);
  //u.store[_KEY]=buffer[(bitindex>>3)&(WINDOWSIZE-1)]<<bi;
  u.store[_KEY]=buffer[bitindex>>3]<<bi;
  bi=8-bi;
  bitindex+=bi;

  while(bits)
  {
    if(!bi)
    {
      //u.store[_KEY]=buffer[(bitindex>>3)&(WINDOWSIZE-1)];
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
