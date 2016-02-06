//A very simple tetris clone, made in C++ with SDL library.
#include <fstream>	//we read how the tetris block looks like from file
#include <SDL/SDL.h>	//for the SDL, obvoiusly
#include <SDL/SDL_ttf.h>	//we write out some text
#include <cstdlib>	//for the random numbers, and the sprintf
#include <ctime>		//to seed the random number generator
#include <iostream>
#include <string>

//some global constants
const int WIDTH=12;	//how many block is in a row
const int HEIGHT=24;	//how many block is in a cloumn
const int BLOCK_SIZE=20;	//how big is a block (in pixel)


bool addToplist(TTF_Font* font,int p);	//if the point is in the toplist, it will add, plus additionally ask, if we want a new game (implementation at the bottom
void displayToplist(TTF_Font* font); //display the 10 most point in order



void rotateleft(int element[4][4],bool h)	//we rotate a 4x4 (or 3x3 if h) matrix to the left
{
	char tmp[4][4];
	if(!h)	//if 4x4
	{
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				tmp[3-j][i]=element[i][j]; //just swap the rows and coloumns to a tmp matrix (draw it, if you don't understand)
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				element[i][j]=tmp[i][j];	//and copy back to the original
	}else	//if 3x3
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				tmp[2-j][i]=element[i][j];	//do the same, just different datas
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				element[i][j]=tmp[i][j];
		for(int i=0;i<4;i++)
		{
			element[3][i]=0;	//in this case, we need to make 0's to the bottom row or most right coloumn
			element[i][3]=0;
		}
	}
}



void fillelement(int element[8][4][4],int &k)	//we fill the elements array with all of the tetris blocks from file, k is the number (7 or 8)
{
	std::ifstream in("tetris.dat");	//read from this file
	int tmp[16];	//to this array
	k=0;	//we currently read 0 blocks in
	while(!in.eof())	//while we are not at the end of the file
	{
		for(int i=0;i<16;i++)
		{
			in >> tmp[i];	//read the 16 integers to this array
		}
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				element[k][i][j]=tmp[i*4+j];	//and add to the end of the elements matrix
		k++;	//we read one
	}
	k--;	//we have a plus line (which only contain the eof character, I guess)
}



//we don't have to make this 3 line, every time, we want to blit a surface
void blitSurface(SDL_Surface* source,int sourcex,int sourcey,int sourcew,int sourceh,SDL_Surface* dest,int destx,int desty,int destw,int desth)
{
	SDL_Rect rec1={sourcex,sourcey,sourcew,sourceh};
	SDL_Rect rec2={destx,desty,destw,desth};
	SDL_BlitSurface(source,&rec1,dest,&rec2);
}



//we set the current block 2D array to an element of the elements 3D array
void setCurrentblock(int elements[8][4][4],int block[4][4],int elem)
{
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			block[i][j]=elements[elem][i][j];
}



//The most complicated function in this whole program
//the current block is our current block, the posx,posy is the current position, the table is the whole table
bool checkCollision(int currentblock[4][4],int posx,int posy,int table[HEIGHT][WIDTH])
{
	//if the block is out of the screen in the right
	if(posx>8)
	{
		for(int j=0;j<4;j++)
		{
			for(int i=3;i>=WIDTH-posx;i--)
			{
				//we go through every 4 line, and check the element in the end, which is out of the screen
				//if it's 0, than it's nothing, but as soon, as we find something, which is not zero, then one part of the block is
				//out of the screen, so we return with 1
				if(currentblock[j][i]!=0)
				{
					return 1;
				}
			}
		}
	}
	
	if(posx<0)	//if our block is out of the screen in the left, we do almost the same thing, just in the oposite direction
	{
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<-posx;j++)
			{
				if(currentblock[i][j]!=0)
				{
					return 1;
				}
			}
		}
	}
	//now check, if it is down, the principle is the same, just in this case, we start from the bottom of the block
	if(posy>=20)
	{
		for(int i=0;i<4;i++)
		{
			for(int j=3;j>HEIGHT-posy-1;j--)
			{
				if(currentblock[j][i]!=0)
				{
					return 1;
				}
			}	
		}
	}
	//now check, if we collided with another block in the table, the principle is similar, we go from tho bottom of the block, check, if the block
	//has some part there, if it has, check, if the table has some part there, if so, we return with 1, because there was a collision
	//probably, you can just go through this matrix in a normal way, but it may be a little bit more efficient (1,2 ns :)
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			if(posy+i<HEIGHT && posx+j<WIDTH)
				if(table[posy+i][posx+j]!=0 && currentblock[i][j]!=0)
					return 1;
	return 0;	//if nothing has returned, return with false
}



int checkful(int table[HEIGHT][WIDTH])	//return a row number, if the row is full of numbers (-1, if there was a 0 in the row)
{
	//self-explanotary
	bool b;
	for(int i=HEIGHT-1;i>=0;i--)
	{
		b=true;
		for(int j=0;j<WIDTH;j++)
		{
			if(table[i][j]==0)
			{
				b=false;
				break;
			}
		}
		if(b)
			return i;
	}
	return -1;
}


//This 2 overloaded function very useful, it's write out a text (or a number in case of the overloaded function) to a specific x,y coordinate with
//the specific font and with the r,g,b color
void writeText(TTF_Font* font,int x,int y,const char* text,int r,int g,int b)
{
		SDL_Color color={r,g,b};	//write out the points in the next 6 lines, nothing to explain here
		SDL_Surface* pointsurface=TTF_RenderText_Solid(font,text,color);
		SDL_Rect rec={x,y};
		SDL_BlitSurface(pointsurface,NULL,SDL_GetVideoSurface(),&rec);		
		SDL_FreeSurface(pointsurface);
}

void writeText(TTF_Font* font,int x,int y,int num,int r,int g,int b)
{
		char text[200];
		SDL_Color color={r,g,b};	//write out the points in the next 6 lines, nothing to explain here
		sprintf(text,"%d",num);
		SDL_Surface* pointsurface=TTF_RenderText_Solid(font,text,color);
		SDL_Rect rec={x,y};
		SDL_BlitSurface(pointsurface,NULL,SDL_GetVideoSurface(),&rec);		
		SDL_FreeSurface(pointsurface);
}

//I added this initialization, so we can restart the game, whenever we want
void initGame(int* blockx,int* blocky,bool* h,int elements[8][4][4],int table[HEIGHT][WIDTH],int currentblock[4][4],int* normalspeed,int* points,int* deletedlines,int* nextblock,int* speed,int db)
{
	(*blockx)=4;	//the starting coordinate of our falling block is 4,0
	(*blocky)=0;
	int ran=rand()%(db+1);	//we pick  random block
	setCurrentblock(elements,currentblock,ran);		//set it to current block
	(*h)=(ran>2);	//the 1st and 2nd block is 4x4 all others are 3x3
	for(int i=0;i<HEIGHT;i++)
		for(int j=0;j<WIDTH;j++)
			table[i][j]=0;	//null out the game-table
	(*normalspeed)=500;	//half a second between every time, the block moves down one
	(*points)=0;
	(*deletedlines)=0;
	(*nextblock)=rand()%(db+1);	//we pick a random next block
	(*speed)=(*normalspeed);	//our current speed is the same as our normal speed (if we press the down arrow, the speed will increase, while
														//the normalspeed remains the same)
}


int main(int argc,char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);	//init the SDL
	SDL_Surface* screen=SDL_SetVideoMode(BLOCK_SIZE*WIDTH+BLOCK_SIZE*10,BLOCK_SIZE*HEIGHT,32,SDL_SWSURFACE);	//and our screen
	TTF_Init();	//the TTF as well, cus we want to write out stuff
	TTF_Font* font=TTF_OpenFont("air.ttf",12);
	Uint32 start;	//the start time, to limit FPS
	bool running=true;	//is the program still running?
	SDL_Event event;	//What event has happened?
	srand(time(0));	//seed the random number generator
	int db=0;	//how many blocks do we have (init 0)?
	int elements[8][4][4];	//we store all of the blocks in this 3D array
	int table[HEIGHT][WIDTH];	//This is our whole game-table, all of the blocks, which already put down is stored here
	int currentblock[4][4];	//our current falling block
	fillelement(elements,db);	//load the blocks
	SDL_Surface* blocks=SDL_LoadBMP("blocks.bmp");	//load the image, which stores, the part of the images
	SDL_SetColorKey(blocks,SDL_SRCCOLORKEY,SDL_MapRGB(screen->format,255,0,255));	//The purple color should be invisible
	int blockx;	//the init position of the falling block
	int blocky;
	bool h;	//if it's the 1. or 2. block, we need 4x4, else 3x3
	Uint32 lastmove=SDL_GetTicks();	//how often should the block come down one
	Uint32 lastkey=SDL_GetTicks();	//how often should react the block, when the key is pressed (without release)
	bool keys[2]={0,0};	//if left arrow pressed, than keys[0]=1 if right arrow is pressed keys[1]=1
	int points;
	int normalspeed;	//how quick the tetris blocks fall (in this case once every 0.5 seconds)
	int speed=500;	//the speed, the game currently running (the speed increase, when you press the down arrow)
	const int FPS=30;	//how many FPS the game will run (this effect minimally, how quick the blocks fall)
	int nextblock;	//what is the next block?
	int deletedlines;	//how much lines we already deleted
	int movingspeed=400;	//if we hold down the left/right arrow, how often (150ms) we want to move the block
	int quickmovingspeed=30;
	bool mousepointing=false;	//do we pointing to the toplist text?
	int tmpx,tmpy;	//the location of the mouse cursor
	//we initialize the game (set every parameter to default)
	initGame(&blockx,&blocky,&h,elements,table,currentblock,&normalspeed,&points,&deletedlines,&nextblock,&speed,db);
	int moved=0;
	while(running)	//while the game running
	{
		start=SDL_GetTicks();	//get the current time (for FPS limitation)
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:	//if escape is pressed, escape
							running=false;
							break;
						case SDLK_UP:	//if up arrow is pressed
							rotateleft(currentblock,h);	//rotate the block
							if(checkCollision(currentblock,blockx,blocky,table))	//and check if there is a collision
								for(int i=0;i<3;i++)
									rotateleft(currentblock,h);	//if there was a collision, rotate back (rotate 4 times, is like if you haven't done anything)
									
							break;
						case SDLK_LEFT:
							keys[0]=1;
							break;
						case SDLK_RIGHT:
							keys[1]=1;
							break;
						case SDLK_DOWN:
							speed=10;	//if down key is pressed, speed up a little bit
							break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_DOWN:
							speed=normalspeed;	//if you released the down arrow, set back the speed
							break;
						case SDLK_LEFT:
							keys[0]=0;
							moved=0;
							break;
						case SDLK_RIGHT:
							keys[1]=0;
							moved=0;
							break;
					}
					break;
				case SDL_QUIT:
					running=false;
					break;
				case SDL_MOUSEMOTION:
					//if we moved the mouse
					tmpx=event.motion.x;	//get the coordinates
					tmpy=event.motion.y;
					//if we are pointing to the square, which contain the toplist text
					if(tmpx>BLOCK_SIZE*WIDTH+2 && tmpx<BLOCK_SIZE*WIDTH+80 && tmpy>80+BLOCK_SIZE*5 && tmpy<80+BLOCK_SIZE*5+20)
						mousepointing=true;	//make this boolean true
					else
						mousepointing=false;	//else false
					break;
				case SDL_MOUSEBUTTONDOWN:
				//if we hit the mousebutton
					tmpx=event.button.x;
					tmpy=event.button.y;
					//on the toplist text
					if(tmpx>BLOCK_SIZE*WIDTH && tmpx<BLOCK_SIZE*WIDTH+80 && tmpy>80+BLOCK_SIZE*5 && tmpy<80+BLOCK_SIZE*5+20)
						displayToplist(font);	//display it
					break;
			}
		}
		//LOGIC
		
		//if the collision happens, and the block is at the top of the screen, than game is over
		if(checkCollision(currentblock,blockx,blocky,table) && blocky==0)
		{
			if(addToplist(font,points))	//try to add out points to the toplist, if addtoplist returns 1, restart the game
				initGame(&blockx,&blocky,&h,elements,table,currentblock,&normalspeed,&points,&deletedlines,&nextblock,&speed,db);
			else
				running=false;	//else we exit
		}

		//if we exceeded the time, how often should it go down, than move it down, and set back the time
		if(SDL_GetTicks()-lastmove>speed)
		{
			blocky++;
			lastmove=SDL_GetTicks();
		}
		//if (left) key was pressed, and last time, when we moved the block is more than 200ms, than move it again
		if((keys[0] && moved>=2 && SDL_GetTicks()-lastkey>quickmovingspeed) || (keys[0] && moved==0) || (keys[0] && moved==1 && SDL_GetTicks()-lastkey>movingspeed))
		{
			blockx--;	//move
			moved++;
			lastkey=SDL_GetTicks();	//set back the time
			if(checkCollision(currentblock,blockx,blocky,table)==1)//if there is a collision
				blockx++;			//move back
		}else if((keys[1] && moved>=2 && SDL_GetTicks()-lastkey>quickmovingspeed) || (keys[1] && moved==0) || (keys[1] && moved==1 && SDL_GetTicks()-lastkey>movingspeed))	//same with right arrow
		{
			blockx++;
			moved++;
			lastkey=SDL_GetTicks();
			if(checkCollision(currentblock,blockx,blocky,table)==1)
				blockx--;		
		}
		
		
		//RENDER
		SDL_FillRect(screen,&screen->clip_rect,SDL_MapRGB(screen->format,0,0,0));	//clear the screen
		
		for(int i=0;i<24;i++)	//render out the table
			for(int j=0;j<12;j++)
			{
				if(!table[i][j])	//if a value=0, than don't do anything, else draw the corresponding block (1 is the first block on the images, 2 is the 2...)
					continue;
				else
					blitSurface(blocks,(table[i][j]-1)*BLOCK_SIZE,0,BLOCK_SIZE,BLOCK_SIZE,screen,j*BLOCK_SIZE,i*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE);
			}
		//render the falling block
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
			{
				if(currentblock[i][j])	//if not "empty", draw the corresponding block
					blitSurface(blocks,(currentblock[i][j]-1)*BLOCK_SIZE,0,BLOCK_SIZE,BLOCK_SIZE,screen,blockx*BLOCK_SIZE+j*BLOCK_SIZE,blocky*BLOCK_SIZE+i*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE);
			}
		//here I check the collision, first I move the object down
		blocky++;
		if(checkCollision(currentblock,blockx,blocky,table))//and if there is a collision
		{
			blocky--;	//move back
			for(int i=0;i<4;i++)
				for(int j=0;j<4;j++)
				{
					if(currentblock[i][j]!=0)
						table[blocky+i][blockx+j]=currentblock[i][j];	//and draw the block to the table matrix (except the 0), so we handle it as fallen block, it's not falling anymore
				}
			blocky=0;	//and generate a new block the same way, as we did in the beginning of the main
			blockx=4;
			setCurrentblock(elements,currentblock,nextblock);
			h=(nextblock>2);
			nextblock=rand()%(db+1);
		}else
			blocky--;	//if there was no collision, move the object back (else it will go every 2nd line)
		
		//this while loop will go as long, as we have full lines
		while(1)
		{
			int k=checkful(table);
			if(k==-1)
				break;
			deletedlines++;
			//if we have full lines
			for(int i=k;i>0;i--)
				for(int j=0;j<WIDTH;j++)
					table[i][j]=table[i-1][j];	//delete them, by move everything above it a line down
			for(int i=0;i<HEIGHT;i++)
				table[0][i]=0;	//and 0 out the most top line (usually this has no effect, except, if there is a block-part in the top row)
			points+=(550-normalspeed)/10;
			if((points%50)==0 && normalspeed>0)	//if the points are dividable by 50 (50,100,150...), speed up the game
			{
				if(normalspeed>50)
					normalspeed-=50;
				else if(normalspeed>0)
					normalspeed-=10;
			}
		}

		//render the menu
		SDL_Rect rec={BLOCK_SIZE*WIDTH,0,screen->clip_rect.w,screen->clip_rect.h};	//the background of the tetris is black (if you want
		SDL_FillRect(screen,&rec,SDL_MapRGB(screen->format,50,50,50));							//you can change it to an image)

		rec.x=BLOCK_SIZE*WIDTH+10;	//this is the part next to the tetris (the menu)
		rec.w=4*BLOCK_SIZE;
		rec.y=20;
		rec.h=4*BLOCK_SIZE;
		SDL_FillRect(screen,&rec,SDL_MapRGB(screen->format,100,100,100));	//fill it with gray
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
			{
				if(elements[nextblock][i][j])	//if not "empty", draw the corresponding block
				{
					blitSurface(blocks,(elements[nextblock][i][j]-1)*BLOCK_SIZE,0,BLOCK_SIZE,BLOCK_SIZE,screen,(BLOCK_SIZE*WIDTH)+j*BLOCK_SIZE+20,20+i*BLOCK_SIZE,BLOCK_SIZE,BLOCK_SIZE);
				}
			}
		//draw all of the menutext (huhh, it's good, that I have this function, else it would be a 4,5 times that amount of line)
		writeText(font,BLOCK_SIZE*WIDTH,0,"NEXT BLOCK",255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH,20+BLOCK_SIZE*5,"POINTS: ",255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH+150,20+BLOCK_SIZE*5,points,255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH,40+BLOCK_SIZE*5,"CURRENT SPEED:",255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH+150,40+BLOCK_SIZE*5,normalspeed,255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH,60+BLOCK_SIZE*5,"DELETED LINES: ",255,255,255);
		writeText(font,BLOCK_SIZE*WIDTH+150,60+BLOCK_SIZE*5,deletedlines,255,255,255);
		rec.x=BLOCK_SIZE*WIDTH+2;	//the toplist square
		rec.y=80+BLOCK_SIZE*5;
		rec.w=80;
		rec.h=20;
		if(!mousepointing)	//if we are not pointing to the toplist square, fill it with
		{
			SDL_FillRect(screen,&rec,SDL_MapRGB(screen->format,0,0,0));	//black
		}else
		{
			SDL_FillRect(screen,&rec,SDL_MapRGB(screen->format,255,0,0));	//else red
		}
			writeText(font,BLOCK_SIZE*WIDTH+5,80+BLOCK_SIZE*5,"TOPLIST",255,255,255);	//and write the toplist text

			
		SDL_Flip(screen);	//show evrything o the real screen
		if(1000.0/FPS>SDL_GetTicks()-start)	
			SDL_Delay(1000.0/FPS-(SDL_GetTicks()-start));	//regulate the FPS
	}
	SDL_FreeSurface(blocks);	//delete and close everything
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}



bool addToplist(TTF_Font* font,int p)
{
	std::ifstream in("toplist.dat");	//open the toplist
	char name[11][100];	//there are at max 10 name in the toplist, plus our current player's name
	int point[11];			//and point
	int i=0;	//number of lines in the toplist
	std::string tmpname;	//the name of the player
	while(!in.eof())
	{
		in >> name[i] >> point[i];	//while we are not finished,
		if(name[0]!='\0')
			i++;
		if(i==10)	//or reached 10, read in the names and points
			break;
	}
	i--;	//we go over one (cause if we read 3 things, i==3, but the indexing starts from 0, so i=2)
	SDL_Event event;
	if(point[i]<p || i<9)	//because the list is ordered, if we have more point, than the last, or the list is not enough long
	{
		//we add the player toplist
		bool isRunning=true;	//did we press escape (exit the program)?
		bool shift=false;			//Is the shift key id down?
		bool ready=false;			//Do we ready enter our name (did we press enter key)?
		while(isRunning && !ready)
		{
			SDL_Rect rec={0,(BLOCK_SIZE*HEIGHT)/2-10,SDL_GetVideoSurface()->w,50};	//fill a rectangle at the center of the screen with black
			SDL_FillRect(SDL_GetVideoSurface(),&rec,SDL_MapRGB(SDL_GetVideoSurface()->format,0,0,0));
			writeText(font,0,(BLOCK_SIZE*HEIGHT)/2-10,"CONGRATULATION, YOU GET INTO THE TOPLIST",255,255,255);	//write out a text
			writeText(font,0,(BLOCK_SIZE*HEIGHT)/2,tmpname.c_str(),255,255,255);	//and the characters, which the user already entered
			SDL_Flip(SDL_GetVideoSurface());	//show it
			SDL_WaitEvent(&event);	//and wait, until something happen
			if(event.type==SDL_KEYDOWN)
			{
				if((event.key.keysym.sym>=SDLK_a && event.key.keysym.sym<=SDLK_z))	//if we pressed a key from a to z, then
				{
					if(shift)	//if we pressed shift
						tmpname+=event.key.keysym.sym-32;	//we use the capital letters (the difference between a and A is 32 in the ASCII table)
					else
						tmpname+=event.key.keysym.sym;	//the keysym is the same as the ASCII code, so we add it to the end of the string
						
				}else if(event.key.keysym.sym>=SDLK_0 && event.key.keysym.sym<=SDLK_9)	//if key 1 to 9
				{
					tmpname+=event.key.keysym.sym;	//same
				}else if(event.key.keysym.sym==SDLK_RSHIFT || event.key.keysym.sym==SDLK_LSHIFT)	//if we pressed the shift key (left or right)
				{
					shift=true;
				}else if(event.key.keysym.sym==SDLK_BACKSPACE && tmpname.size()!=0)	//if we pressed backspace, and we have characters
				{
					tmpname.erase(tmpname.end()-1);	//delete the last character
				}else if(event.key.keysym.sym==SDLK_ESCAPE)	//if escape
				{
					isRunning=false;	//exit
					break;
				}else if(event.key.keysym.sym==SDLK_RETURN && tmpname.size()!=0)	//if enter and we have characters
				{
					ready=true;	//we are ready
					break;
				}
			}else if(event.type==SDL_KEYUP)
			{
				if(event.key.keysym.sym==SDLK_RSHIFT || event.key.keysym.sym==SDLK_LSHIFT)	//if we released shift
				{
					shift=false;
				}			
			}

		}
		if(ready)
		{
			strcpy(name[i],tmpname.c_str());	//add the name at the end of the array
			point[i]=p;	//same with points
			//then sort them
			for(int j=0;j<=i;j++)
			{
				for(int k=j;k<=i;k++)
				{
					if(point[j]<point[k])
					{
						char tmp[100];
						strcpy(tmp,name[j]);
						strcpy(name[j],name[k]);
						strcpy(name[k],tmp);
						int tmp2=point[j];
						point[j]=point[k];
						point[k]=tmp2;
					}
				}
			}
			std::ofstream out("toplist.dat");
			//finally, write out 10 of them (1 may left out :)
			for(int j=0;j<=i;j++)
			{
				out << name[j] << " " << point[j] << "\n";
			}
			out.flush();	//we make sure, that we write out (probably not needed)
			//do we want to playe again?
			while(1)
			{
				SDL_Rect rec={0,(BLOCK_SIZE*HEIGHT)/2-10,SDL_GetVideoSurface()->w,50};
				SDL_FillRect(SDL_GetVideoSurface(),&rec,SDL_MapRGB(SDL_GetVideoSurface()->format,0,0,0));
				writeText(font,SDL_GetVideoSurface()->w/2-50,SDL_GetVideoSurface()->h/2-10,"AGAIN? (y/n)",255,255,255);
				SDL_Flip(SDL_GetVideoSurface());
				SDL_WaitEvent(&event);
				if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_y)	//if y is pressed, we do
					return 1;
				if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_n)	//else we don't
					return 0;
				if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE)
					return 0;
				//else just write out the question again
			}
		}
	}else{
		while(1)
		{
			//if we are not in the toplist, we ask if the player want to play again (probably, we could make this, and the last
			//same question to one, if we want, make the code a few line shorter)
			SDL_Rect rec={SDL_GetVideoSurface()->w/2-50,SDL_GetVideoSurface()->h/2-10,50,10};
			SDL_FillRect(SDL_GetVideoSurface(),&rec,SDL_MapRGB(SDL_GetVideoSurface()->format,0,0,0));
			writeText(font,SDL_GetVideoSurface()->w/2-50,SDL_GetVideoSurface()->h/2-10,"YOU LOST, AGAIN? (y/n)",255,255,255);
			SDL_Flip(SDL_GetVideoSurface());
			SDL_WaitEvent(&event);
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE)
				return 0;
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_y)
				return 1;
			if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_n)
				return 0;
		}
	}
}

void displayToplist(TTF_Font* font)	//we display the toplist
{
	std::ifstream in("toplist.dat");
	char name[10][100];
	int point[10];
	int i=0;
	while(1)	//read in from the toplist.dat file
	{
		in >> name[i] >> point[i];
		if(in.eof())
			break;
		if(name[0]!='\0')
			i++;
		if(i==10)
			break;
	}

	SDL_Event event;

	while(1)
	{
		//draw a black rectangle almost the whole screen
		SDL_Rect rec={10,10,SDL_GetVideoSurface()->w-20,SDL_GetVideoSurface()->h-20};
		SDL_FillRect(SDL_GetVideoSurface(),&rec,SDL_MapRGB(SDL_GetVideoSurface()->format,0,0,0));
		
		for(int j=0;j<i;j++)
		{
			//and write out the name and point row by row
			writeText(font,15,10+j*20,name[j],255,255,255);
			writeText(font,300,10+j*20,point[j],255,255,255);
		}
		writeText(font,25,SDL_GetVideoSurface()->h-50,"PRESS ESCAPE TO RETURN TO THE GAME",255,255,255);
		SDL_Flip(SDL_GetVideoSurface());
		SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE)	//if escape is pressed, return to the game
			return;
	}

}
