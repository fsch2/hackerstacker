/*

stacker. stacker stacker.

You need:

- 2 x  8x8 RGB-123 matrix  (http://www.hackerspaceshop.com/rgb-123.html)
- 1 x  Button preferably with background light.
- 1 x Piezzo Beeper
- 1 x strand of WS2812B ledstrip for frame border background light (optional)

I used one of those cheap chinese arduino pro micro clones.
A teensy 2.0 or 3.1 will work just as well.



GPL 2.0,  thrown together by overflo from hackerspaceshop.com / metalab.at

*/



#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>




#define DEBUG 1


//these pin definitions are valid for arduino pro MICRO (the one with USB

// where is the matrix?
#define MATRIX_PIN 8

// where is the BUTTON?
// needs to be interrupt enabled pin
#define BUTTON_PIN 2

// the led on the button
// we will dim this with PWM
#define LED_PIN 10

// the buzzer, we will drive this with PWM tone() function
#define BUZZERPIN 16

// how blinding should it be? (1-255)
#define GAME_BRIGHTNESS 40

// the second dip switch that controls wether the border  and the button should be iluminated or not
#define LED_ENABLE 10

// the third dip switch that controls if the game should be easy or hard TODO: implement this.
#define EASY_HARD 14

// the fourth  dip switch that can be used for custom input .. for example a mode that makes the game unbeatable or something.. 
#define CUSTOM_SWITCH 15

// on board led
#define ONBOARD_LED 17

// the ledstrip for the border wall enligthtment
#define BORDER_PIN 9














// OPTIONAL 


// how many light are there?
#define BORDER_PIXELCOUNT 56



/////////////////////  AAAAAAAAND ACTION!



// MATRIX DECLARATION:
// Parameter 1 = width of EACH NEOPIXEL MATRIX (not total display)
// Parameter 2 = height of each matrix
// Parameter 3 = number of matrices arranged horizontally
// Parameter 4 = number of matrices arranged vertically
// Parameter 5 = pin number (most are valid)
// Parameter 6 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the FIRST MATRIX; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs WITHIN EACH MATRIX are
//     arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns WITHIN
//     EACH MATRIX proceed in the same order, or alternate lines reverse
//     direction; pick one.
//   NEO_TILE_TOP, NEO_TILE_BOTTOM, NEO_TILE_LEFT, NEO_TILE_RIGHT:
//     Position of the FIRST MATRIX (tile) in the OVERALL DISPLAY; pick
//     two, e.g. NEO_TILE_TOP + NEO_TILE_LEFT for the top-left corner.
//   NEO_TILE_ROWS, NEO_TILE_COLUMNS: the matrices in the OVERALL DISPLAY
//     are arranged in horizontal rows or in vertical columns, respectively;
//     pick one or the other.
//   NEO_TILE_PROGRESSIVE, NEO_TILE_ZIGZAG: the ROWS/COLUMS OF MATRICES
//     (tiles) in the OVERALL DISPLAY proceed in the same order for every
//     line, or alternate lines reverse direction; pick one.  When using
//     zig-zag order, the orientation of the matrices in alternate rows
//     will be rotated 180 degrees (this is normal -- simplifies wiring).
//   See example below for these values in action.
// Parameter 7 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 pixels)
//   NEO_GRB     Pixels are wired for GRB bitstream (v2 pixels)
//   NEO_BRG     ???i also available
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA v1 pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)

// Example with three 10x8 matrices (created using NeoPixel flex strip --
// these grids are not a ready-made product).  In this application we'd
// like to arrange the three matrices side-by-side in a wide display.
// The first matrix (tile) will be at the left, and the first pixel within
// that matrix is at the top left.  The matrices use zig-zag line ordering.
// There's only one row here, so it doesn't matter if we declare it in row
// or column order.  The matrices use 800 KHz (v2) pixels that expect GRB
// color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 1, 2, MATRIX_PIN,
  NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_ROWS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);




// background illumination (optional)
Adafruit_NeoPixel border = Adafruit_NeoPixel(BORDER_PIXELCOUNT, BORDER_PIN, NEO_RGB + NEO_KHZ800);


// some globals .. maybe i clean this up later. probably not :)
int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by















void setup() {

  
  //delay(1000);
 

  Serial.begin(9600);

  // background illumination (optional)
  border.begin(); // This initializes the NeoPixel library.
  border.setBrightness(GAME_BRIGHTNESS);


  
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(GAME_BRIGHTNESS);
  
  
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZERPIN,OUTPUT);


  pinMode(BUTTON_PIN, INPUT_PULLUP);    


// dip switched

  pinMode(LED_ENABLE, INPUT_PULLUP);    
  pinMode(EASY_HARD, INPUT_PULLUP);      
  pinMode(CUSTOM_SWITCH, INPUT_PULLUP);  

 
  digitalWrite(BUZZERPIN,LOW);    
  if(!digitalRead(LED_ENABLE)) digitalWrite(LED_PIN,HIGH);    
    
  attachInterrupt(1,buttonpressed_LOW_isr,FALLING);
  
  
  run_for_the_first_time();
  


    
  
  
}


















//default do not much
int waitforactivation =1;

//default the button is not pressed
int button_pressed =0;


void buttonpressed_LOW_isr()
{
  
  
  waitforactivation=0;
  button_pressed=1;
  
  
  if(!digitalRead(LED_ENABLE)) analogWrite(LED_PIN, 255);  
    
}












//what color is each line

int m1=160;
int m2=220;





// each line has its own color

const uint32_t linecolors[] = {
  matrix.Color(255,0,255),  //0
  
  matrix.Color(m2, 0, 255),   //1
  matrix.Color(m1, 0, 255),  //2
  matrix.Color(0,0,255),   //3
  
  matrix.Color(0, m1, 255),    //4
  matrix.Color(0,m2,255),      //5
  matrix.Color(0, 255, 255),   //6
  
  matrix.Color(0, 255,m2),  //7
  matrix.Color(0, 255, m1), //8 
  matrix.Color(0, 255, 0),  //9
  
  matrix.Color(m1, 255, 0),   //10
  matrix.Color(m2, 255, 0),   //11
  matrix.Color(255, 255, 0),  //12
 
  matrix.Color(255, m2, 0),  //13
  matrix.Color(255, m1,0),   //14  
  matrix.Color(255, 0, 0)   //15

};



const uint32_t bordercolors[] = {
  border.Color(0,255,255),  //0
  
  border.Color(0, 220, 255),  //1
  border.Color(0, 120, 255),  //2
  border.Color(0,0,255),    //3
  
  border.Color(160, 0, 255),   //ok 4
  border.Color(220,0,255),    // ok 5
  border.Color(255, 0, 255),    // ok 6
 
  border.Color(255, 0,180), //ok 7
  border.Color(255, 0, 120), //ok 8
  border.Color(255,0, 0),  //ok 9
 
  border.Color(255, 120, 0),  //ok  10 
  border.Color(200, 255, 0),  // ok 11
  border.Color(255, 255, 0), // ok  12
  
  border.Color(180, 255, 0),  //okish   13
  border.Color(120, 255,0),    // okish 14
  border.Color(0,  255, 0)  //ok (red) 15

};



//should be 0
int score=0;
int rowscore=0;








// how hard will each level be?

//normal
const int linedelay_normal[] = {
  10, 15, 25, 30, 40, 50,
  60, 65, 70, 75, 80, 85,
  90, 95, 100, 105
  };




//hardcore
const int linedelay_hardcore[] = {
  15, 
  16, 17, 
  18, 19, 20, 21, 
  22, 23, 24, 25, 
  26,27, 28, 29, 30
  };



const int *linedelay;


void loop() {

  

  
  
  linedelay=linedelay_normal;
  
  
  // hardcore if dipswitch set to hard
  if(!digitalRead(EASY_HARD))  linedelay=linedelay_hardcore;
  

  
  
  
 
  for(int i=0; i<=255;i++)
  {
  delay(5);
    analogWrite(LED_PIN, i);    
  } 

  
  
  
  // fade led wait for interruptroutine to set waitforactivation to zero
   
  while(waitforactivation)  show_startup_animation(); 
  
  button_pressed=0; 
  delay(100);
   
   
  display_load_animation();

  //delay(100);
  button_pressed=0;
  run_game();  
    
}
    
    
  
  

  
  
  

  
  
  
  
  
  
  
  
  
  
  
  
  
  


////////////  THE GAME


int current_row =15;
int direction=1; // from left to right
int current_position=0;




// for testing
//const int linedelay[] = {
//200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,200
//  };










int blocksize=4;
int blockposition=0;
int moving_direction=1;
int force_blocksize=4;


int current_row_setpixels[] = {
  -1,-1,-1,-1
  };

int last_row_setpixels[] = {
  -1,-1,-1,-1
 };



void run_game()
{

  Serial.println("run_game() called");

  // needed so the startanimation wont show from run_game() .. kinda dirty
  while(!waitforactivation)
  {
  
    while(!digitalRead(BUTTON_PIN)){delay(5);} // debounce that bitchy button
    button_pressed=0;
    
    // how big is the block in this row  by default?
    
    blocksize=1;      
    if(current_row>2)  blocksize=2;    
    if(current_row>6)  blocksize=3; 
    if(current_row>10) blocksize=4;     

    if(force_blocksize<blocksize)
      blocksize=force_blocksize;
    
    if(blocksize<force_blocksize)
     force_blocksize=blocksize;  
    
    //draw backgound
    draw_background();
    
    //draw current line
    draw_blockline(current_row);
    
    //display current frame
    matrix.show();


    // TROLLMODE
    // if custom switch == on we make some random delay.. sometimes.. in the last 5 rows
//    if(!digitalRead(CUSTOM_SWITCH) && (random(1,20)==10) && (current_row<5))
//    {
//      delay(1);
//    }
//    else
//    {
      delay(linedelay[current_row]);
//    }
    
    
    
    if(button_pressed)
    {
 
      Serial.println("BUTTON pressed");
      // we wait till button is released again
      while(!digitalRead(BUTTON_PIN)){}
      
      delay(50);
      analogWrite(LED_PIN, 0);  

      // display animation
      close_current_row(current_row);
    
      //setup background
      setup_backgound(current_row);
    
      current_row--;       

      //  we reached the top ..
      if(current_row==-1)
      {
        rowscore += 100; // last row special bonus! 
        win_game();
        //current_row=15;
      }
      
      button_pressed=0; 
    } // if button pressed
    
   } // while

}













 // display animation
 void  close_current_row(int current_row)
 {
   
     Serial.println("close_current_row() called");

   
   
   // nothing happens on bottom row
   if(current_row==15)
   {
    for(int j=0;j<4;j++)
      last_row_setpixels[j]  = current_row_setpixels[j];        
   }
   

  
   
   
   
   // find out which pixels are alright here
   for(int i=0;i<4;i++)
   {
    
    int ok=0; 
    // is the pixel on position i in previous row?
    for(int j=0;j<4;j++)
    {    
       
       if(current_row_setpixels[i] == last_row_setpixels[j]  &&  (current_row_setpixels[i] != -1)) 
       { 

        ok=1;
        
       }

     
     
      }
      
    if(!ok &&  (current_row_setpixels[i] != -1))
    {



      
      kill_pixel(current_row,current_row_setpixels[i]);
      current_row_setpixels[i]=-1;
      
      //set new force_blocksize
      force_blocksize -= 1;
      
      if(force_blocksize==0) 
      {
       lose_game(); 
       return;
    
      }       
            
    } // !ok :(
     
   } // for i++  ..  
  
   rowscore = rowscore + (16-current_row);
   
   
   debug("new rowscore: ");
   debug(String(rowscore));
   debug("\n------\n");
 }





















// background frame
uint16_t matrix_background[128]={
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0

};








// shows animation of pixel that drops out..
void kill_pixel(int row, int col)
{
  
  
  
  for (int i=0;i<3;i++)
  {
    tone(BUZZERPIN,700+(i*10),50);
    matrix.drawPixel(col, row, matrix.Color(255,255,255));
    matrix.show();
    delay(50); 
    matrix.drawPixel(col, row, 0);
    matrix.show();
    delay(30);    
  
  } 
 
  for(int i=0; i<15;i++)
  {
  
    if(row+i >15) return;
     tone(BUZZERPIN,800+(i*10),50);
     matrix.drawPixel(col, row+i, matrix.Color(255,255,255));
     matrix.show();
     delay(50); 
     matrix.drawPixel(col, row+i, 0);
     matrix.show();
     delay(30);  
   
     // remove lines from background
     matrix_background[((row+i)*8)+col]=0;
   
  } 
}





























// this function is responsible for creating each line segment that moves left right and back ..
void draw_blockline(int row)
{ 
  

  
    for(int i=0;i<BORDER_PIXELCOUNT;i++){

    border.setPixelColor(i, bordercolors[row]); // Moderately bright green color.
  



  } 
if(!digitalRead(LED_ENABLE))   border.show(); // This sends the updated pixel color to the hardware.
   
  
  
  
  // moving L to R
  if(moving_direction)
  {
    for(int i=0;i<blocksize;i++)
    {   
      current_row_setpixels[i] = blockposition+i;
      matrix.drawPixel(blockposition+i, row, linecolors[row]);     
    }
  
    // increment position
    blockposition++;
  
    // reset direction
    if(blockposition>(8-blocksize-1))
    {
      tone(BUZZERPIN,500 - (10*row),50); 
      moving_direction=0;
    }
  } 
  else
  {
    
   // FROM R to L    
   for(int i=0;i<blocksize;i++)
   {
      current_row_setpixels[i] = blockposition+i;

      matrix.drawPixel(blockposition+i, row, linecolors[row]);     

   }
   
   // decrement position
   blockposition--;
   // reset direction
   if(blockposition==0)
   {
    tone(BUZZERPIN,550 - (10*row),50);  
    moving_direction=1;
   }
   
   
  }  
      
}
















// setup background 
// background is painted before we draw our lines on it and shows all the previous lines
void  setup_backgound(int current_row){
    
   for(int x=0;x<4;x++)
   {    
     int setpixel = current_row_setpixels[x];
     if( (setpixel > -1)  && (setpixel<8) )  matrix_background[(current_row*8)+setpixel]=linecolors[current_row];    
   }

  
   for(int i=0;i<4;i++)
   {
    last_row_setpixels[i] =  current_row_setpixels[i];
    current_row_setpixels[i]=-1;
   }
 
}











// fills framebuffer
void draw_background()
{
   for(int y=0;y<16;y++)
   {
    for(int x=0;x<8;x++)
    {
      // fill frame with background
      matrix.drawPixel(x, y, matrix_background[(y*8)+x]);     
    }     
   }  
}





// clear backgorund
void reset_background()
{
   for(int x=0;x<128;x++)
   {
      matrix_background[x]=0;     
   }
}





// this happens if you make it to the top!
void win_game()
{
  
  
  debug("win_game() called\n");
  
  
  
  
  
  if(!check_highscore()) show_score();
 
  
  
   show_highscore(500);
 
   finish_game();
   flash(1);  
   
   while(!button_pressed) show_sunshine_anim(); 

  
}




// this happens when you lose :(
void lose_game()
{
  
    debug("lose_game() called\n");
  

   if(!check_highscore()) show_score();
   show_highscore(500);
   finish_game();  
   flash(3); 
   
}





// reset variables, clean up.. finish a game.
void finish_game()
{

      debug("finish_game() called\n");
  
 reset_background();
 
 /// clear all buffers and variables set during game
 
 // start next round from bottom
 current_row=16;
 
 // with 4 blocks
 force_blocksize=4; 
 
 // no press ..
 button_pressed=0; 
 
  // reset block pos for net game 
  blockposition=0;

  // R-L again ..
  moving_direction=1;  

  score=0;
  rowscore=0;
 
  // fill buffers with emptyness
  for(int i=0;i<4;i++)
  {
    last_row_setpixels[i]   = 99;
    current_row_setpixels[i]= 99;
  }
 
  // display anim
  waitforactivation=1;
  
}




// calculates the score for the pixels left in the deisplay once the game finishes..
void set_pxl_score()
{
   score=0;
  
  // get score by counting pixels available on background..
  for (int i=0;i<128;i++)
  {
   if(matrix_background[i]>0)  score++;   
  }   
}






void  run_for_the_first_time()
{ 
 Serial.println("run_for_the_first_time() called");
  
 
 // this function is supposed to run ONCE.
 // when we inited the eeprom we write 1 to position 100 
 if((int) EEPROM.read(100) != 1) 
 { 
   
  pinMode(ONBOARD_LED,OUTPUT); 

  for(int i=0;i<5;i++)
  {  
   digitalWrite(ONBOARD_LED,LOW);
   digitalWrite(LED_PIN,LOW); 
   delay(100);
   digitalWrite(ONBOARD_LED,HIGH);
   digitalWrite(LED_PIN,HIGH); 
   delay(100); 
  }

  // make some noise 
  tone(BUZZERPIN,500,500); 
  delay(500); 
  tone(BUZZERPIN,600,500); 
  delay(500);   
 
 

   
   // for setup purposed we dont want this to be blinding
   matrix.setBrightness(10);
     
     
   for (int i=0;i<16;i++)
   {
    for (int j=0;j<8;j++)
    {
     matrix.drawPixel(j, i, matrix.Color(255,255,255));
    }  
   } 
    matrix.show();  
     
     
     
     
      border.setBrightness(10);  
      
 for(int i=0;i<BORDER_PIXELCOUNT;i++){

    border.setPixelColor(i, border.Color(255,255,255)); // Moderately bright green color.

  } 
      
   border.show();  
      
       


     uint32_t go=0;  
     int setup_finished=0;
    
     while(!setup_finished){
       
       
      go=millis(); 
       
      // on button longpress finish setup
      while(!digitalRead(BUTTON_PIN))
      {
       if(millis()-go >2000)
       {
         // buton pressed, flash the eeprom
         init_eeprom_on_firstrun();    
         matrix.fillScreen(0);
              
         matrix.setBrightness(GAME_BRIGHTNESS);
         button_pressed=0;
         setup_finished=1;
        
        
        return; 
         
         
       }
      }
  
  
     }

  


   } // if eeprom empty
 

}



void debug(String str)
{
  if(DEBUG)  Serial.print(str);
  
}
