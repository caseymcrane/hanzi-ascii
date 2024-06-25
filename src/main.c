#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>
#include <sys/ioctl.h>
#include <math.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

int min(int x, int y)
  {
    return (x < y) ? x : y;
  }

int subpixel_print(int pixel_buf_len,int argwidth,int chengyu_len, char *subpix_symbols[], unsigned char *pixel_buf){
  
  int q1,q2,q3,q4,quadmask;

  for(int i = 0; i<pixel_buf_len-argwidth-chengyu_len-3; i+=2){
    q1 = pixel_buf[i] >> 7;
    q2 = pixel_buf[i+1] >> 7;
    q3 = pixel_buf[i+argwidth*chengyu_len] >> 7;
    q4 = pixel_buf[i+argwidth*chengyu_len+1] >> 7;
    
    quadmask = (q1 << 3) | (q2 << 2) | (q3 << 1) | q4; 

    printf("%s",subpix_symbols[quadmask]);
    
    if((i+2)%(argwidth*chengyu_len)==0){
      printf("\n ");
      i+=argwidth*chengyu_len;
    }
  
  }
  return 0;
}


int main(int argc, char *argv[]) {

  //define our array of subpixel chars counting in binary from left to right, top to bottom
  char *subpix[16] = {" ","▗","▖","▄","▝","▐","▞","▟","▘","▚","▌","▙","▀","▜","▛","█"};
  
  //get term size to set default text width
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  int term_w = w.ws_col;
  int term_h = w.ws_row;

  char *argfont = NULL;
  char *argfile = NULL;
  int argheight = min(term_h,floor(term_w/6))*2;
  int argwidth = floor(term_w/4)*2;
  int c;
  char *endptr;

  opterr = 0;

  while ((c = getopt (argc, argv, "fhwc:")) != -1)
    switch (c)
      {
      case 'f':
        argfont = optarg;
        break;
      case 'h':
        argheight = strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          fprintf(stderr, "Invalid number: %s\n", optarg);
          return 1;
        }
        break;
      case 'w':
        argwidth = strtol(optarg, &endptr, 10);
        if (*endptr != '\0') {
          fprintf(stderr, "Invalid number: %s\n", optarg);
          return 1;
        }
        break;
      case 'c':
        argfile = optarg;
        break;
      default:
        abort ();
      }

  FT_Library ft;
  FT_Error err = FT_Init_FreeType(&ft);
  if (err != 0) {
    printf("Failed to initialize FreeType\n");
    exit(EXIT_FAILURE);
  }

  FT_Face face;
  if(argfont==NULL)
  {
    err = FT_New_Face(ft, "./jpangwa.ttf", 0, &face);
  }
  else
  {
    err = FT_New_Face(ft, argfont, 0, &face);
  }
  if (err != 0) {
    printf("Failed to load face\n");
    exit(EXIT_FAILURE);
  }
  FT_Select_Charmap(face, FT_ENCODING_UNICODE);

  err = FT_Set_Pixel_Sizes(face, argwidth, argheight);
  if (err != 0) {
    printf("Failed to set pixel size\n");
    exit(EXIT_FAILURE);
  }

  FILE *ptr_file = fopen("./hz","r");
  if (!ptr_file)
    return 1;

  FT_Int32 load_flags = FT_LOAD_DEFAULT;
  FT_Int32 render_flags = FT_RENDER_MODE_NORMAL;
  
  setlocale(LC_ALL, "en_US.UTF-8"); 

  wchar_t ch;

  int chengyu_len = 4;
  //allocate the print array based on the size of the chengyu
  //(assumed to be 4 for now)
  int pixel_buf_len = argwidth * argheight * chengyu_len;
  unsigned char *pixel_buf = NULL;
  pixel_buf = malloc(pixel_buf_len * sizeof(unsigned char));

  for(int i = 0; i<pixel_buf_len;i++){
    pixel_buf[i]=0;
  }

  int charcount = 0;

  while ((ch = fgetwc(ptr_file))!=WEOF)
  {
    //check for newline characters to find the length of the chengyu
    if(ch==0xA){
      //chengyu_len = charcount;
    }
    err=FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), load_flags);
    if (err != 0) {
      printf("Failed to load glyph\n");
      exit(EXIT_FAILURE);
    }

    err = FT_Render_Glyph(face->glyph, render_flags);
    if (err != 0) {
      printf("Failed to render the glyph\n");
      exit(EXIT_FAILURE);
    }
    FT_Bitmap bitmap = face->glyph->bitmap;
    for (size_t i = 0; i < bitmap.rows; i++) {
      for (size_t j = 0; j < bitmap.width; j++) {
         unsigned char pixel_brightness =
             bitmap.buffer[i * bitmap.pitch + j];
          pixel_buf[(i*argwidth*chengyu_len) + j + (argwidth*charcount)] = pixel_brightness;
      }
    }
    charcount++;
  }
  fclose(ptr_file);

  printf("\n");
  subpixel_print(pixel_buf_len,argwidth,chengyu_len,subpix,pixel_buf);

  free(pixel_buf);
  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}
