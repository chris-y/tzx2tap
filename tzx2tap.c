#pragma output CLIB_MALLOC_HEAP_SIZE = -1
///////////////////////////////////////////////////////////////////////////////
// TZX to TAP converter
//                                                                       v0.13b
// (c) 1997 Tomaz Kac
//
// Watcom C 10.0+ specific code... Change file commands for other compilers

//
// Ported to ZX Spectrum Next (NextZXOS) by Chris Young 2020 
//
//

#define O_BINARY 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>

#define MAJREV 1         // Major revision of the format this program supports
#define MINREV 03        // Minor revision -||-

unsigned char fhi,fho;
uint32_t flen;
char *mem;
char buf[256];
uint32_t pos, p;
uint32_t len;
long block;
int longer,custom,only,dataonly,direct,not_rec,snap,call_seq;
char tzxbuf[10]={ 'Z','X','T','a','p','e','!', 0x1A, 1, 00 };
uint32_t start;

uint32_t Get2(char *mem) { return(mem[0]+(mem[1]*256UL)); }
uint32_t Get3(char *mem) { return(mem[0]+(mem[1]*256UL)+(mem[2]*256UL*256UL)); }
uint32_t Get4(char *mem) { return(mem[0]+(mem[1]*256UL)+(mem[2]*256UL*256UL)+(mem[3]*256UL*256UL*256UL)); }

uint32_t FileLength(unsigned char fh);
void Error(char *errstr);
void ChangeFileExtension(char *str,char *ext);
uint32_t read_file(unsigned char fh, char *mem, uint32_t seek);
void convert_data(unsigned char fhi, unsigned char fho, uint32_t posn, uint32_t len);

int main(int argc, char *argv[])
{
  long loop_start = 0;
  int loop_count = 0;

  printf("\nZXTape Utilities\nTZX to TAP Converter v0.13b\n");
  printf("NextZXOS ver by Chris Young\ngithub.com/chris-y/tzx2tap\n");
  if(argc<2|| argc>3)
    {
    printf("\nUsage: TZX2TAP INPUT.TZX [OUTPUT.TAP]\n");
    exit(0);
    }

  if(argc==2) 
    {  
    strcpy(buf,argv[1]); 
    ChangeFileExtension(buf,"tap"); 
    }
  else      
    strcpy(buf,argv[2]);

  fhi = esx_f_open(argv[1], ESX_MODE_READ);

  if(fhi==255) 
    Error("Input file not found!");

  fho = esx_f_open(buf, ESX_MODE_WRITE | ESX_MODE_OPEN_CREAT_NOEXIST);

  if(fho==255)
    Error("unable to create output file");

  flen=FileLength(fhi);
  mem=(char *) malloc(100);

  if(mem==NULL) 
   Error("Not enough memory to load input file!");

  esx_f_read(fhi,mem,10); mem[7]=0;

  if(strcmp(mem,"ZXTape!")) 
    { 
    free(mem); 
    Error("File is not in ZXTape format!"); 
    }

  printf("\nZXTape file revision %d.%02d\n",mem[8],mem[9]);

  if(!mem[8]) 
    Error("Development versions of ZXTape format are not supported!");

  if(mem[8]>MAJREV) 
    printf("\nWarning: Some blocks may not be recognised and used!\n");

  if(mem[8]==MAJREV && mem[9]>MINREV) 
    printf("\nWarning: Some of the data might not be properly recognised!\n");

  pos=block=longer=custom=only=dataonly=direct=not_rec=snap=call_seq=0;

  /* read 100 bytes */
  start = read_file(fhi, mem, 0);
  start = 0; /* pos is always off by ten */

  printf("\nConverting...");

  while(pos<flen-10)
    {
    pos++;
    p = pos - start;

    printf(".");

    switch(mem[p-1])
      {
      case 0x10: len=Get2(&mem[p+0x02]);
                 convert_data(fhi, fho, pos+0x02, 2);
                 convert_data(fhi, fho, pos+0x04, len);

                 pos+=len+0x04;
                 start = read_file(fhi, mem, pos);
                 block++;
                 break;
      case 0x11: len=Get3(&mem[p+0x0F]);
                 if(len<65536)
                   {
                   convert_data(fhi, fho, pos+0x0F, 2);
                   convert_data(fhi, fho, pos+0x12, len);
                   block++;
                   }
                 else 
                   longer=1;
                 custom=1;
                 pos+=len+0x12;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x12: only=1;
                 pos+=0x04;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x13: only=1;
                 pos+=(mem[p+0x00]*0x02)+0x01;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x14: len=Get3(&mem[p+0x07]);
                 if(len<65536)
                   {
                   convert_data(fhi, fho, pos+0x07, 2);
                   convert_data(fhi, fho, pos+0x0A, len);
                   block++;
                   }
                 else 
                   longer=1;
                 dataonly=1;
                 pos+=len+0x0A;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x15: direct=1;
                 pos+=Get3(&mem[p+0x05])+0x08;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x20: pos+=0x02;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x21: pos+=mem[p+0x00]+0x01;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x22: break;
      case 0x23: pos+=0x02;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x24: pos+=0x02;
                 loop_start=pos;
                 loop_count=Get2(&mem[p+0x00]);
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x25: if(loop_count > 0) {
                   pos = loop_start;
                   loop_count--;
                 }
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x26: pos += (Get2(&mem[p+0x00])*2)+0x02;
                 start = read_file(fhi, mem, pos);
                 call_seq = 1;
                 break;
      case 0x27: call_seq = 1;
                 break;
      case 0x30: pos+=mem[p+0x00]+0x01;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x31: pos+=mem[p+0x01]+0x02;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x32: pos+=Get2(&mem[p+0x00])+0x02;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x33: pos+=(mem[p+0x00]*0x03)+0x01;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x34: pos+=0x08;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x35: pos+=Get4(&mem[p+0x10])+0x14;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x40: pos+=Get3(&mem[p+0x01])+0x04;
                 snap = 1;
                 start = read_file(fhi, mem, pos);
                 break;
      case 0x5A: pos+=0x09;
                 start = read_file(fhi, mem, pos);
                 break;
      default:   pos+=Get4(&mem[p+0x00]+0x04);
                 start = read_file(fhi, mem, pos);
                 not_rec=1;
      }
    }

  printf("\n\n");

  if(custom) 
    printf("-- Warning: Custom Loading blocks were converted!\n");

  if(longer) 
    printf("-- Warning: Over 64k long Custom Loading blocks were *not* converted!\n");

  if(only) 
    printf("-- Warning: Some Pure Tone and/or Sequence of Pulses blocks encountered!\n");

  if(dataonly) 
    printf("-- Warning: Data Only blocks were converted!\n");

  if(direct) 
    printf("-- Warning: Direct Recording blocks were encountered!\n");

  if(call_seq) 
    printf("-- Warning: Call sequence blocks were encountered!\n");

  if(snap)
    printf("Note: Embedded snapshot not extracted\n");

  if(not_rec) 
    printf("-- Warning: Some blocks were NOT recognised !\n");

  printf("Succesfully converted %d blocks\n",block);
  esx_f_close(fhi);
  esx_f_close(fho);
  free(mem);

  return 0;
}

// Changes the File Extension of String *str to *ext
void ChangeFileExtension(char *str,char *ext)
{
  int n;
  
  n=strlen(str); 

  while(str[n]!='.') 
    n--;

  n++; 
  str[n]=0; 
  strcat(str,ext);
}

// Determine length of file
uint32_t FileLength(unsigned char fh)
{
  struct esx_stat es;
  
  if(esx_f_fstat(fh, (struct esx_stat *)&es)) {
    Error("unable to stat file");
    return 0;
  }
  return(es.size);
}

uint32_t read_file(unsigned char fh, char *mem, uint32_t seek)
{
  uint32_t posn = seek;
  esx_f_seek(fh, 10 + posn, ESX_SEEK_SET);
  esx_f_read(fh, mem, 100);
  return posn;
}

void convert_data(unsigned char fhi, unsigned char fho, uint32_t posn, uint32_t len)
{
  char *buf;
  uint32_t bytes_read = 0;
  uint32_t bytes_to_read = 0;

  buf=(char *) malloc(1024);

  if(buf==NULL) 
    Error("Not enough memory to convert");

  esx_f_seek(fhi, 10 + posn, ESX_SEEK_SET);

  while(bytes_read < len) {
    if((len - bytes_read) <= 1024) {
      bytes_to_read = (len - bytes_read);
    } else {
      bytes_to_read = 1024;
    }

    esx_f_read(fhi, buf, bytes_to_read);
    esx_f_write(fho, buf, bytes_to_read); 

    bytes_read += bytes_to_read;
  }

  free(buf);
}

// exits with an error message *errstr
void Error(char *errstr)
{
  printf("\n-- Error: %s\n",errstr);
  exit(0);
}
