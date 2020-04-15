#pragma output CLIB_MALLOC_HEAP_SIZE = -1
///////////////////////////////////////////////////////////////////////////////
// TZX2TAP for ZX Spectrum Next (NextZXOS) by Chris Young 2020 
// Based on TZX to TAP converter v0.13b (c) 1997 Tomaz Kac
//

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arch/zxn.h>
#include <arch/zxn/esxdos.h>
#include <arch/zxn/sysvar.h>
#include <z80.h>

#define PROGVER "1.3beta"

#define MAX_HEADER_SIZE 0x14 // Size of largest block header
#define MAJREV 1         // Major revision of the format this program supports
#define MINREV 20        // Minor revision -||-

#define RTM_28MHZ 3 // from manual

static unsigned char old_cpu_speed;

static uint32_t Get2(char *mem) { return(mem[0]+(mem[1]*256UL)); }
static uint32_t Get3(char *mem) { return(mem[0]+(mem[1]*256UL)+(mem[2]*256UL*256UL)); }
static uint32_t Get4(char *mem) { return(mem[0]+(mem[1]*256UL)+(mem[2]*256UL*256UL)+(mem[3]*256UL*256UL*256UL)); }

// exits with an error message *errstr
static void Error(int eno)
{
  ZXN_NEXTREGA(REG_TURBO_MODE, old_cpu_speed);
  exit(eno);
}

// Changes the File Extension of String *str to *ext
static void ChangeFileExtension(char *str,char *ext)
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
static uint32_t FileLength(unsigned char fh)
{
  struct esx_stat es;

  errno = 0;
  if(esx_f_fstat(fh, (struct esx_stat *)&es)) {
    Error(errno);
  }
  return(es.size);
}

// read next header from input and return the position within the file
static uint32_t read_file(unsigned char fh, char *mem, uint32_t posn)
{
  esx_f_seek(fh, posn, ESX_SEEK_SET);
  esx_f_read(fh, mem, MAX_HEADER_SIZE);
  return posn;
}

// read chunks of 1K from input and write to output
static int convert_data(unsigned char fhi, unsigned char fho, uint32_t posn, uint32_t len)
{
  char *buf;
  uint32_t bytes_read = 0;
  uint32_t bytes_to_read = 0;

  buf=(char *) malloc(1024);

  if(buf==NULL) {
    printf("\nNot enough memory");
    Error(0); //ERRB_4_OUT_OF_MEMORY
  }

  esx_f_seek(fhi, posn, ESX_SEEK_SET);

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
  return 0;
}

int main(int argc, char *argv[])
{
  unsigned char fhi,fho;
  uint32_t flen;
  char *mem;
  char buf[256];
  uint32_t pos, p, oldpos = 0;
  uint32_t len;
  long block;
  bool longer,custom,only,dataonly,direct,not_rec,snap,call_seq,deprecated;
  bool verbose = false;
  bool help = false;
  bool list = false;
  char tzxbuf[10]={ 'Z','X','T','a','p','e','!', 0x1A, 1, 00 };
  uint32_t start;
  long loop_start = 0;
  uint32_t loop_count = 0;
  int err = 0;
  int i;
  int converted = 0;
  char conv[3]={0x20, 0x2b, 0x2a};
  char *src = NULL;
  char *dst = NULL;

  printf("TZX2TAP for NextZXOS v%s\nby Chris Young 2020\ngithub.com/chris-y/tzx2tap\n", PROGVER);
  printf("Based on ZXTape Utilities\nTZX to TAP Converter v0.13b\n");

  if(argc>1) {
    for(i=1;i<argc;i++) {
      if(strcmp(argv[i], "-v") == 0) {
        verbose = true;
      } else if(strcmp(argv[i], "-l") == 0) {
        list = true;
        verbose = true; /* list implies verbose */
      } else if(strcmp(argv[i], "-h") == 0) {
        help = true;
      } else {
        if(src==NULL) {
          src = argv[i];
        } else if(dst==NULL) {
          dst = argv[i];
        } else {
          help = true;
        }
      }
    }
  } else {
    help = true;
  }

  if((help==true) || (src==NULL) || ((list==true) && (dst!=NULL))) {
    printf("\nUsage:\n.TZX2TAP [ARGS] IN.TZX [OUT.TAP]\n");
    printf("\nWhere ARGS are:\n");
    printf("  -h Show this help\n");
    printf("  -l List\n");
    printf("  -v Verbose\n");
    exit(0);
  }

  if(dst==NULL) {
    strcpy(buf,src); 
    ChangeFileExtension(buf,"tap"); 
  } else {
    strcpy(buf,dst);
  }

  old_cpu_speed = ZXN_READ_REG(REG_TURBO_MODE);
  ZXN_NEXTREG(REG_TURBO_MODE, RTM_28MHZ);

  if(list==false) {
    z80_bpoke(23692, 50);
  }

  errno = 0;
  fhi = esx_f_open(src, ESX_MODE_READ);

  if(errno) 
    Error(errno);

  if(list==false) {
    errno = 0;
    fho = esx_f_open(buf, ESX_MODE_WRITE | ESX_MODE_OPEN_CREAT_NOEXIST);

    if(errno) {
      if(errno == ESX_EEXIST) {
        printf("\nError: Output file exists\nProbably already converted!\n");
        Error(0);
      } else {
        Error(errno);
      }
    }
  }

  flen=FileLength(fhi);
  mem=(char *) malloc(MAX_HEADER_SIZE);

  if(mem==NULL) {
    printf("\nNot enough memory");
    Error(0); //ERRB_4_OUT_OF_MEMORY
  }

  esx_f_read(fhi,mem,10); mem[7]=0;

  if(strcmp(mem,"ZXTape!")) { 
    free(mem);
    Error(ESX_EWRTYPE); 
  }

  printf("\nZXTape file revision %d.%02d\n",mem[8],mem[9]);

  if(!mem[8]) {
    free(mem);
    printf("Error: TZX dev ver not supported\n");
    Error(ESX_EWRTYPE);
  }

  if(mem[8]>MAJREV) 
    printf("\nWarning: Some blocks may not be recognised and used\n");

  if(mem[8]==MAJREV && mem[9]>MINREV) 
    printf("\nWarning: Some of the data might not be properly recognised\n");

  pos = 10;
  block=0;
  longer=custom=only=dataonly=direct=not_rec=snap=call_seq=deprecated=false;

  if(list==false) printf("\nConverting...");

  if(verbose) {
    printf("\nID Len  ");
  }

  while(pos<flen) {
    start = read_file(fhi, mem, pos);
    pos++;
    p = pos - start;

    if(verbose) {
      printf("\n%02x", mem[p-1]);
      oldpos = pos;
      strcpy(buf, "");
      converted = 0;
    } else {
      printf(".");
    }

    switch(mem[p-1])
      {
      case 0x10: len=Get2(&mem[p+0x02]);
                 if(list==false) {
                   err = convert_data(fhi, fho, pos+0x02, 2);
                   if(err) {
                     free(mem);
                     Error(err);
                   }
                   err = convert_data(fhi, fho, pos+0x04, len);
                   if(err) {
                     free(mem);
                     Error(err);
                   }
                 }
                 pos+=len+0x04;
                 if(verbose) {
                   converted = 1;
                   strcpy(buf, "Standard data");
                 }
                 block++;
                 break;
      case 0x11: len=Get3(&mem[p+0x0F]);
                 if(len<65536) {
                   if(list==false) {
                     err = convert_data(fhi, fho, pos+0x0F, 2);
                     if(err) {
                       free(mem);
                       Error(err);
                     }
                     err = convert_data(fhi, fho, pos+0x12, len);
                     if(err) {
                       free(mem);
                       Error(err);
                     }
                   }
                   if(verbose) converted = 2;
                   block++;
                 }
                 else {
                   longer=true;
                   if(verbose) converted = 0;
                 }
                 custom=true;
                 pos+=len+0x12;
                 if(verbose) strcpy(buf, "Turbo data");
                 break;
      case 0x12: only=true;
                 pos+=0x04;
                 if(verbose) strcpy(buf, "Pure tone");
                 break;
      case 0x13: only=true;
                 pos+=(mem[p+0x00]*0x02)+0x01;
                 if(verbose) strcpy(buf, "Pulse sequence");
                 break;
      case 0x14: len=Get3(&mem[p+0x07]);
                 if(len<65536) {
                   if(list==false) {
                     err = convert_data(fhi, fho, pos+0x07, 2);
                     if(err) {
                       free(mem);
                       Error(err);
                     }
                     err = convert_data(fhi, fho, pos+0x0A, len);
                     if(err) {
                       free(mem);
                       Error(err);
                     }
                   }
                   if(verbose) converted = 2;
                   block++;
                 }
                 else {
                   longer=true;
                   if(verbose) converted = 0;
                 }
                 dataonly=true;
                 pos+=len+0x0A;
                 if(verbose) strcpy(buf, "Custom data");
                 break;
      case 0x15: direct=true;
                 pos+=Get3(&mem[p+0x05])+0x08;
                 if(verbose) strcpy(buf, "Direct recording");
                 break;
      case 0x16:
      case 0x17: deprecated = true;
                 pos+=Get4(&mem[p+0x00]+0x04);
                 if(verbose) sprintf(buf, "C64 %s", mem[p-1]==0x16 ? "standard" : "turbo");
                 break;
      case 0x18: 
      case 0x19: only = true;
                 pos+=Get4(&mem[p+0x00]+0x04);
                 if(verbose) sprintf(buf, "%s", mem[p-1]==0x18 ? "CSW recording" : "General data");
                 break;
      case 0x20: pos+=0x02;
                 if(verbose) sprintf(buf, "Pause for %ldms", Get2(&mem[p+0x00]));
                 break;
      case 0x21: pos+=mem[p+0x00]+0x01;
                 if(verbose) sprintf(buf, "Group start: %.*s", mem[p+0x00]>11 ? 11 : mem[p+0x00], &mem[p+0x01]);
                 break;
      case 0x22: if(verbose) strcpy(buf, "Group end");
                 break;
      case 0x23: pos+=0x02;
                 if(verbose) sprintf(buf, "Jump %ld blocks", Get2(&mem[p+0x00]));
                 break;
      case 0x24: pos+=0x02;
                 if(list==false) {
                   loop_start=pos;
                   loop_count=Get2(&mem[p+0x00]);
                 }
                 if(verbose) {
                   converted = 1;
                   sprintf(buf, "Loop start, count=%ld", loop_count);
                 }
                 break;
      case 0x25: if(list==false) {
                   if(loop_count > 0) {
                     pos = loop_start;
                     loop_count--;
                   }
                 }
                 if(verbose) {
                   converted = 1;
                   sprintf(buf, "Loop end, count=%ld", loop_count);
                 }
                 break;
      case 0x26: pos += (Get2(&mem[p+0x00])*2)+0x02;
                 call_seq = true;
                 if(verbose) strcpy(buf, "Call sequence");
                 break;
      case 0x27: call_seq = true;
                 if(verbose) strcpy(buf, "Call sequence return");
                 break;
      case 0x28: pos += Get2(&mem[p+0x00])+0x02;
                 if(verbose) sprintf(buf, "Select, items=%ld", mem[p+0x02]);
                 break;
      case 0x2A: pos+=Get4(&mem[p+0x00]+0x04);
                 if(verbose) strcpy(buf, "Stop tape in 48K mode");
      case 0x2B: pos+=Get4(&mem[p+0x00]+0x04);
                 if(verbose) sprintf(buf, "Set signal level %s", mem[p+0x04] == 0 ? "low" : "high");
                 break;
      case 0x30: pos+=mem[p+0x00]+0x01;
                 if(verbose) sprintf(buf, "Desc: %.*s", mem[p+0x00]>18 ? 18 : mem[p+0x00], &mem[p+0x01]);
                 break;
      case 0x31: pos+=mem[p+0x01]+0x02;
                 if(verbose) sprintf(buf, "Message: %.*s", mem[p+0x01]>15 ? 15 : mem[p+0x01], &mem[p+0x02]);
                 break;
      case 0x32: pos+=Get2(&mem[p+0x00])+0x02;
                 if(verbose) strcpy(buf, "Archive info");
                 break;
      case 0x33: pos+=(mem[p+0x00]*0x03)+0x01;
                 if(verbose) strcpy(buf, "Hardware type");
                 break;
      case 0x34: pos+=0x08;
                 deprecated = true;
                 if(verbose) strcpy(buf, "Emulation info");
                 break;
      case 0x35: pos+=Get4(&mem[p+0x10])+0x14;
                 if(verbose) sprintf(buf, "Custom info: %.*s", 10, &mem[p+0x00]);
                 break;
      case 0x40: pos+=Get3(&mem[p+0x01])+0x04;
                 snap = true;
                 deprecated = true;
                 if(verbose) sprintf(buf, "Snapshot: %s", mem[p+0x00] == 0 ? "Z80" : "SNA");
                 break;
      case 0x5A: pos+=0x09;
                 if(verbose) strcpy(buf, "Glue block");
                 break;
      default:   pos+=Get4(&mem[p+0x00]+0x04);
                 not_rec=true;
                 if(verbose) strcpy(buf, "UNKNOWN");
      }

      if(verbose) {
          printf("%c", conv[converted]);
        if(pos > oldpos) {
          printf("%04lx %s", pos - oldpos, buf);
        } else {
          printf("0000 %s", buf);
        }
      }
    }

  printf("\n\n");

  if(list==false) {

    if(custom) 
      printf("Warning: Custom Loading blocks  were converted\n\n");

    if(longer) 
      printf("Warning: Over 64k long Custom   Loading blocks not converted\n\n");

    if(only) 
      printf("Warning: Sequence of Pulses and/or Pure Tone blocks encountered\n\n");

    if(dataonly) 
      printf("Warning: Data Only blocks were  converted\n\n");

    if(direct) 
      printf("Warning: Direct Recording blockswere encountered\n\n");

    if(call_seq) 
      printf("Warning: Call sequence blocks   were encountered\n\n");

    if(deprecated) 
      printf("Warning: Deprecated blocks were encountered\n\n");

    if(snap)
      printf("Note: Embedded snapshot(s) not  extracted\n\n");

    if(not_rec) 
      printf("Warning: Some blocks were NOT   recognised\n\n");

    printf("Converted %d blocks\n",block);

    esx_f_close(fho);
  }

  esx_f_close(fhi);

  free(mem);

  ZXN_NEXTREGA(REG_TURBO_MODE, old_cpu_speed);

  return 0;
}

