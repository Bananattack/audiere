/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2001             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *

 ********************************************************************

 function: utility main for setting entropy encoding parameters
           for lattice codebooks
 last mod: $Id$

 ********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include "bookutil.h"

static char *strrcmp_i(char *s,char *cmp){
  return(strncmp(s+strlen(s)-strlen(cmp),cmp,strlen(cmp)));
}

/* This util takes a training-collected file listing codewords used in
   LSP fitting, then generates new codeword lengths for maximally
   efficient integer-bits entropy encoding.

   command line:
   latticetune book.vqh input.vqd [unused_entriesp]

   latticetune produces book.vqh on stdout */

int main(int argc,char *argv[]){
  codebook *b;
  static_codebook *c;
  long *lengths;
  long *hits;

  int entries=-1,dim=-1,guard=1;
  FILE *in=NULL;
  char *line,*name;
  long j;

  if(argv[1]==NULL){
    fprintf(stderr,"Need a lattice codebook on the command line.\n");
    exit(1);
  }
  if(argv[2]==NULL){
    fprintf(stderr,"Need a codeword data file on the command line.\n");
    exit(1);
  }
  if(argv[3]!=NULL)guard=0;

  {
    char *ptr;
    char *filename=strdup(argv[1]);

    b=codebook_load(filename);
    c=(static_codebook *)(b->c);
    
    ptr=strrchr(filename,'.');
    if(ptr){
      *ptr='\0';
      name=strdup(filename);
    }else{
      name=strdup(filename);
    }
  }

  if(c->maptype!=1){
    fprintf(stderr,"Provided book is not a latticebook.\n");
    exit(1);
  }

  entries=b->entries;
  dim=b->dim;

  hits=_ogg_malloc(entries*sizeof(long));
  lengths=_ogg_calloc(entries,sizeof(long));
  for(j=0;j<entries;j++)hits[j]=guard;

  in=fopen(argv[2],"r");
  if(!in){
    fprintf(stderr,"Could not open input file %s\n",argv[2]);
    exit(1);
  }

  if(!strrcmp_i(argv[0],"latticetune")){
    long lines=0;
    line=setup_line(in);
    while(line){      
      long code;
      lines++;
      if(!(lines&0xfff))spinnit("codewords so far...",lines);
      
      if(sscanf(line,"%ld",&code)==1)
	hits[code]++;

      line=setup_line(in);
    }
  }

  if(!strrcmp_i(argv[0],"restune")){
    long step;
    long lines=0;
    long cols=-1;
    float *vec;
    line=setup_line(in);
    while(line){
      int code;
      if(!(lines&0xfff))spinnit("codewords so far...",lines);

      if(cols==-1){
	char *temp=line;
	while(*temp==' ')temp++;
	for(cols=0;*temp;cols++){
	  while(*temp>32)temp++;
	  while(*temp==' ')temp++;
	}
	vec=alloca(sizeof(float)*cols);
	step=cols/dim;
      }
      
      for(j=0;j<cols;j++)
	if(get_line_value(in,vec+j)){
	  fprintf(stderr,"Too few columns on line %ld in data file\n",lines);
	  exit(1);
	}
      
      for(j=0;j<step;j++){
	lines++;
	code=_best(b,vec+j,step);
      	hits[code]++;
      }

      line=setup_line(in);
    }
  }

  fclose(in);

  /* build the codeword lengths */
  build_tree_from_lengths0(entries,hits,lengths);

  c->lengthlist=lengths;
  write_codebook(stdout,name,c); 
  
  fprintf(stderr,"\r                                                     "
	  "\nDone.\n");
  exit(0);
}
