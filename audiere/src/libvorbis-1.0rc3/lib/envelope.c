/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: PCM data envelope analysis and manipulation
 last mod: $Id$

 Preecho calculation.

 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "codec_internal.h"

#include "os.h"
#include "scales.h"
#include "envelope.h"
#include "misc.h"
#include "iir.c" /* Yes, ugly, but needed for inlining */

/* Digital filter designed by mkfilter/mkshape/gencode A.J. Fisher */

static int   cheb_highpass_stages=6;
static float cheb_highpass_B[]={1.f,-6.f,15.f,-20.f,15.f,-6.f,1.f};

static int   cheb_bandpass_stages=6;
static float cheb_bandpass_B[]={-1.f,0.f,3.f,0.f,-3.f,0.f,1.f};


/* 10kHz Chebyshev highpass */
static float cheb_highpass10k_gain= 54.34519586f;
static float cheb_highpass10k_A[]={
  -0.2064797169f,
  -0.5609713214f,
  -1.1352465327f,
  -1.4495555418f,
  -1.7938140760f,
  -0.9473564683f};

/* 6kHz-10kHz Chebyshev bandpass */
static float cheb_bandpass6k_gain=113.4643935f;
static float cheb_bandpass6k_A[]={
  -0.5712621337f,
  1.5626130710f,
  -3.3348854983f,
  4.0471340821f,
  -4.0051680331f,
  2.2786325610f};

/* 3kHz-6kHz Chebyshev bandpass */
static float cheb_bandpass3k_gain= 248.8359377f;
static float cheb_bandpass3k_A[]={
  -0.6564230022f,
  3.3747911257f,
  -8.0098635981f,
  11.0040876874f,
  -9.2250963484f,
  4.4760355389f};

/* 1.5kHz-3kHz Chebyshev bandpass */
static float cheb_bandpass1k_gain= 1798.537183f;
static float cheb_bandpass1k_A[]={
  -0.8097527363f,
  4.7725742682f,
  -11.9800219408f,
  16.3770336223f,
  -12.8553129536f,
  5.4948074309f};

void _ve_envelope_init(envelope_lookup *e,vorbis_info *vi){
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  int ch=vi->channels;
  int i;
  e->winlength=ci->blocksizes[0]/2; /* not random */
  e->minenergy=fromdB(gi->preecho_minenergy);
  e->iir=_ogg_calloc(ch*4,sizeof(*e->iir));
  e->filtered=_ogg_calloc(ch*4,sizeof(*e->filtered));
  e->ch=ch;
  e->storage=128;
  for(i=0;i<ch*4;i+=4){

    IIR_init(e->iir+i,cheb_highpass_stages,cheb_highpass10k_gain,
	     cheb_highpass10k_A,cheb_highpass_B);
    IIR_init(e->iir+i+1,cheb_bandpass_stages,cheb_bandpass6k_gain,
	     cheb_bandpass6k_A,cheb_bandpass_B);
    IIR_init(e->iir+i+2,cheb_bandpass_stages,cheb_bandpass3k_gain,
	     cheb_bandpass3k_A,cheb_bandpass_B);
    IIR_init(e->iir+i+3,cheb_bandpass_stages,cheb_bandpass1k_gain,
	     cheb_bandpass1k_A,cheb_bandpass_B);

    e->filtered[i]=_ogg_calloc(e->storage,sizeof(*e->filtered[i]));
    e->filtered[i+1]=_ogg_calloc(e->storage,sizeof(*e->filtered[i+1]));
    e->filtered[i+2]=_ogg_calloc(e->storage,sizeof(*e->filtered[i+2]));
    e->filtered[i+3]=_ogg_calloc(e->storage,sizeof(*e->filtered[i+3]));
  }

}

void _ve_envelope_clear(envelope_lookup *e){
  int i;
  for(i=0;i<e->ch*4;i++){
    IIR_clear((e->iir+i));
    _ogg_free(e->filtered[i]);
  }
  _ogg_free(e->filtered);
  _ogg_free(e->iir);
  memset(e,0,sizeof(*e));
}

/* straight threshhold based until we find something that works better
   and isn't patented */
static float _ve_deltai(envelope_lookup *ve,float *pre,float *post){
  long n=ve->winlength;

  long i;

  /* we want to have a 'minimum bar' for energy, else we're just
     basing blocks on quantization noise that outweighs the signal
     itself (for low power signals) */

  float minV=ve->minenergy;
  float A=minV*minV*n;
  float B=A;

  for(i=0;i<n;i++){
    A+=pre[i]*pre[i];
    B+=post[i]*post[i];
  }

  A=todB(&A);
  B=todB(&B);

  return(B-A);
}

long _ve_envelope_search(vorbis_dsp_state *v){
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=vi->codec_setup;
  vorbis_info_psy_global *gi=&ci->psy_g_param;
  envelope_lookup *ve=((backend_lookup_state *)(v->backend_state))->ve;
  long i,j,k;

  /* make sure we have enough storage to match the PCM */
  if(v->pcm_storage>ve->storage){
    ve->storage=v->pcm_storage;
    for(i=0;i<ve->ch*4;i++)
      ve->filtered[i]=_ogg_realloc(ve->filtered[i],ve->storage*sizeof(*ve->filtered[i]));
  }

  /* catch up the highpass to match the pcm */
  for(i=0;i<ve->ch;i++){
    float *pcm=v->pcm[i];
    float *filtered0=ve->filtered[i*4];
    float *filtered1=ve->filtered[i*4+1];
    float *filtered2=ve->filtered[i*4+2];
    float *filtered3=ve->filtered[i*4+3];
    IIR_state *iir0=ve->iir+i*4;
    IIR_state *iir1=ve->iir+i*4+1;
    IIR_state *iir2=ve->iir+i*4+2;
    IIR_state *iir3=ve->iir+i*4+3;
    int flag=1;
    for(j=ve->current;j<v->pcm_current;j++){
      filtered0[j]=IIR_filter(iir0,pcm[j]);
      filtered1[j]=IIR_filter_Band(iir1,pcm[j]);
      filtered2[j]=IIR_filter_Band(iir2,pcm[j]);
      filtered3[j]=IIR_filter_Band(iir3,pcm[j]);
      if(pcm[j])flag=0;
    }
    if(flag && ve->current+64<v->pcm_current){
      IIR_reset(iir0);
      IIR_reset(iir1);
      IIR_reset(iir2);
      IIR_reset(iir3);
    }

  }

  ve->current=v->pcm_current;

  {
    int flag=-1;
    long centerW=v->centerW;
    long beginW=centerW-ci->blocksizes[v->W]/4;
    /*long endW=centerW+ci->blocksizes[v->W]/4+ci->blocksizes[0]/4;*/
    long testW=centerW+ci->blocksizes[v->W]/4+ci->blocksizes[1]/2+ci->blocksizes[0]/4;
    if(v->W)
      beginW-=ci->blocksizes[v->lW]/4;
    else
      beginW-=ci->blocksizes[0]/4;

    if(ve->mark>=centerW && ve->mark<testW)return(0);
    if(ve->mark>=testW)return(1);

    if(v->W)
      j=ve->cursor;
    else
      j=centerW-ci->blocksizes[0]/4;
    
    while(j+ve->winlength*3/2<=v->pcm_current){
      if(j>=testW)return(1);
      ve->cursor=j;

      for(i=0;i<ve->ch;i++){
	for(k=0;k<4;k++){
	  float *filtered=ve->filtered[i*4+k]+j;
	  float *filtered2=ve->filtered[i*4+k]+j+ve->winlength/2;
	  float m=_ve_deltai(ve,filtered-ve->winlength,filtered);
	  float mm=_ve_deltai(ve,filtered2-ve->winlength,filtered2);
	  
	  if(m>gi->preecho_thresh[k] || m<gi->postecho_thresh[k]){
	    if(j<=centerW){
	      ve->prevmark=ve->mark=j;
	    }else{
	      /* if a quarter-short-block advance is an even stronger
		 reading, set *that* as the impulse point. */
	      if((m>0. && mm>m) || (m<0. && mm<m))
		flag=j+ve->winlength/2;
	      else
		if(flag<0)flag=j;
	    }
	  }
	}
      }
      
      if(flag>=0){
 	ve->prevmark=ve->mark;
	ve->mark=flag;
	if(flag>=testW)return(1);
	return(0);
      }
      
      j+=ve->winlength/2;
    }
  }
 
  return(-1);
}

int _ve_envelope_mark(vorbis_dsp_state *v){
  envelope_lookup *ve=((backend_lookup_state *)(v->backend_state))->ve;
  vorbis_info *vi=v->vi;
  codec_setup_info *ci=vi->codec_setup;
  long centerW=v->centerW;
  long beginW=centerW-ci->blocksizes[v->W]/4;
  long endW=centerW+ci->blocksizes[v->W]/4;
  if(v->W){
    beginW-=ci->blocksizes[v->lW]/4;
    endW+=ci->blocksizes[v->nW]/4;
  }else{
    beginW-=ci->blocksizes[0]/4;
    endW+=ci->blocksizes[0]/4;
  }

  if(ve->prevmark>=beginW && ve->prevmark<endW)return(1);
  if(ve->mark>=beginW && ve->mark<endW)return(1);
  return(0);
}

void _ve_envelope_shift(envelope_lookup *e,long shift){
  int i;
  for(i=0;i<e->ch*4;i++)
    memmove(e->filtered[i],e->filtered[i]+shift,(e->current-shift)*
	    sizeof(*e->filtered[i]));
  e->current-=shift;
  if(e->prevmark>=0)
    e->prevmark-=shift;
  if(e->mark>=0)
    e->mark-=shift;
  e->cursor-=shift;
}


