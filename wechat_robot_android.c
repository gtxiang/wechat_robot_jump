#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <math.h>

#define co_efficence 1.392
#define piece_body_width 70
#define duration_efficient 1.39
#define piece_base_height_half 20

int width, height;

void u_sleep();
double  get_jump_distance(char *filename);
int command(char *cmd);
int shot_screen();
int max(int a, int b);
int auto_jump(double duration);

int Command(char *cmd)
{
    FILE *fd;
    fd= popen(cmd,"r");
    if(!fd)
       return -1;
    
    char result[1024];
    while(fgets(result, sizeof(result), fd)!= NULL){
         printf("INFO:%s\n", result);
    }
    pclose(fd);
    return 0;
}


int shot_screen()
{
    char *shot = "adb shell screencap -p /sdcard/autojump.png";
    char *pull = "adb pull /sdcard/autojump.png .";
    if ( Command(shot) !=0) return -1;
    if ( Command(pull) !=0) return -2;
    return 0;
}

int max(int a , int b )
{
    if ( a > b )
       return a;
    else
       return b;
}

int auto_jump(double duration)
{
    int duration_time = (int) ( duration * duration_efficient );
    duration_time = max( duration_time, 200 );
#ifdef DEBUG 
    printf("duration time :%d\n", duration_time);
#endif
    int ran_num = 0;
    char cmd[100];
    srand(time(NULL));
    ran_num = rand()%101;
    int left = width/2 - ran_num, top = 1584*(height/1920) - ran_num;
    sprintf(cmd, "adb shell input swipe %d %d %d %d %d",left, top, left, top, duration_time);
    if ( Command(cmd)!=0)
       return -1;
    return 0;
}


double  get_jump_distance(char *filename)
{
    FILE *file = fopen(filename,"rb");
    if(!file)
    {
       printf("open png file fail\n");
       return -1;
    }
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, file);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    
    height = png_get_image_height(png_ptr, info_ptr);
    width  = png_get_image_width(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    
    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

    long piece_x = 0, piece_y = 0;
    long piece_x_sum=0, piece_x_c=0, piece_y_max=0, board_x=0, board_y=0;
    long scan_board_x = width/8, scan_board_y=0;
    
    int i ,j;
    int scan_start_height = height/3;
    int scan_end_height = (height/3)*2;
    for (i=scan_start_height; i<= scan_end_height; i+=50)
    {
        unsigned char check_pix_1 = row_pointers[i][0], check_pix_2 = row_pointers[i][1], check_pix_3 = row_pointers[i][2];
        for ( j=4; j < (4*width); j+=4 ) 
        {
            unsigned char temp1_pix_1 = row_pointers[i][j], temp1_pix_2 = row_pointers[i][j+1], temp1_pix_3 = row_pointers[i][j+2] ;
            if ( temp1_pix_1 != check_pix_1 || temp1_pix_2 != check_pix_2 || temp1_pix_3 != check_pix_3 )
            {
                scan_board_y = i - 50;
                break;
            }
        }
        if (scan_board_y)
            break;
     }
#ifdef DEBUG
     printf("scan_board_y :%ld\n", scan_board_y);
     printf("scan_board_x:%ld, index1:%ld\n", scan_board_x, index1);
#endif
     long index1 = 0;
     for ( i= scan_board_y; i<=scan_end_height; i++)
     {
         index1 = scan_board_x;
         for( j= scan_board_x*4; j< (width-scan_board_x)*4; j+=4,index1++)
         {
             int temp2_pix_1 = (int)row_pointers[i][j], temp2_pix_2 = (int) row_pointers[i][j+1], temp2_pix_3 = (int) row_pointers[i][j+2];
             if ( (temp2_pix_1 >50 && temp2_pix_1<60) && ( temp2_pix_2>53 && temp2_pix_2<63 ) && ( temp2_pix_3>95 && temp2_pix_3<110) )
             {
                  piece_x_sum += index1;
                  piece_x_c++;
                  piece_y_max = max(i, piece_y_max);
             }
         }
     }
     if ( piece_x_sum == 0 || piece_x_c == 0 )
     {
          printf(" can not found x location\n");
          return -1;
     }
#ifdef DEBUG 
     printf("piece_x_sum :%ld, piece_x_c:%ld\n", piece_x_sum, piece_x_c);
#endif
     piece_x = (int) (piece_x_sum/piece_x_c);
     piece_y = piece_y_max - piece_base_height_half;
#ifdef DEBUG
     printf("piece_x:%ld\n", piece_x);
     printf("piece_y:%ld\n", piece_y);
#endif
     int board_x_start, board_x_end; 
     if ( piece_x < (width/2) )
     {
        board_x_start = piece_x;
        board_x_end = width;
     }
     else
     {
        board_x_start = 0;
        board_x_end = piece_x;
     }

     int board_x_c = 0, board_x_sum=0, index2=board_x_start;
     for( i = scan_start_height; i <= scan_end_height; i++) 
     {
         int check1_pix_1 = (int) row_pointers[i][0], check1_pix_2 = (int) row_pointers[i][1], check1_pix_3 = (int) row_pointers[i][2];
         if (board_x >0 || board_y > 0)
             break;
         board_x_sum = 0;
         board_x_c = 0;
         index2 = board_x_start;
         for ( j = board_x_start*4; j< board_x_end*4; j+=4,index2++)
         {
             int temp3_pix_1 = (int) row_pointers[i][j], temp3_pix_2 = (int) row_pointers[i][j+1], temp3_pix_3 = (int) row_pointers[i][j+2];
             int body_width;
             if ( j > piece_x )
                 body_width = j-piece_x;
             else if ( j < piece_x )
                 body_width = piece_x - j;
             else
                 body_width = 0;
             if ( body_width < piece_body_width )
                  continue; 
             if ( (abs(temp3_pix_1-check1_pix_1) + abs(temp3_pix_2-check1_pix_2) + abs(temp3_pix_3-check1_pix_3)) > 10 )
             {
                  board_x_sum += index2;
                  board_x_c++; 
             }
         }
         if ( board_x_sum )
         {
     #ifdef DEBUG
              printf("board_x_sum :%d\n", board_x_sum);
              printf("board_x_c :%d\n", board_x_c);
     #endif
              board_x = board_x_sum/board_x_c;
         }
     }
     #ifdef DEBUG
     printf ("board_x :%ld\n", board_x);
     #endif
     int last_pix_1 = (int) row_pointers[i][board_x*4], last_pix_2 = (int) row_pointers[i][board_x*4+1], last_pix_3 = (int) row_pointers[i][board_x*4+2];
     int k;
     for (k=i+274; k>=i ; k--)
     {
         int temp4_pix_1 = (int) row_pointers[k][board_x*4], temp4_pix_2 = (int) row_pointers[k][board_x*4+1], temp4_pix_3 = (int)row_pointers[k][board_x*4+2];
         if ( (abs(temp4_pix_1-last_pix_1) + abs(temp4_pix_2 - last_pix_2) + abs(temp4_pix_3 - last_pix_3)) < 10)
            break;
     }
     board_y =  (i+k)/2 ;
     
     for ( j = i ; j <= i+200; j++)
     {
         int temp5_pix_1 = (int) row_pointers[j][board_x], temp5_pix_2 = (int) row_pointers[j][board_x+1], temp5_pix_3 = (int) row_pointers[j][board_x+2];
         if ( (abs(temp5_pix_1-245)+abs(temp5_pix_2-245)+abs(temp5_pix_3-245)) == 0 )
         {
             board_y = j+10;
             break;
         }
     }
     if (board_x==0 || board_y == 0)
         return -2;
     double distance = sqrt( (abs(board_x - piece_x) * abs(board_x - piece_x)) + (abs(board_y - piece_y) * abs(board_y - piece_y)) ) ;
     printf ("haha ! I got it , board_x:%ld board_y:%ld piece_x:%ld piece_y:%ld distance:%lf\n", board_x, board_y, piece_x, piece_y, distance);
     fclose( file );
     png_destroy_read_struct(&png_ptr, &info_ptr, 0 );
     return distance; 
}



int main()
{
    while(1)
    {
        if (shot_screen())
           printf("fail on screen shot\n"); 
        
        double distance = get_jump_distance("/opt/autojump.png");
        if ( distance < 0 )
        {
            printf(" get distance fail , exit\n");
            exit(0);
        }
        if (auto_jump(distance))
           return -1;
       
        u_sleep();
    }
    return 0;
}


void u_sleep()
{
    struct timeval sleep_time;
    sleep_time.tv_sec = 1;
    sleep_time.tv_usec = 500000;
  
    select ( 1, 0, 0, 0, &sleep_time);

}
