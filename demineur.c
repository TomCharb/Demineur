#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <locale.h>
#include <wchar.h>

#define _X_OPEN_SOURCE_EXTENDED
#define LOGOX 47
#define LOGOY 5

int SIZEX = 20;
int SIZEY = 10;
int MINES = 15;
char **display_grid;	//What is on the screen
char **grid;			//The numbers and the mines
char **flags;			//Flag storing table
int **revealed;		//Informations about the state of each slot : visible or not

char logo[LOGOY][LOGOX]={
"  ____                 _                       ",
" |  _ \\  ___ _ __ ___ (_)_ __   ___ _   _ _ __ ",
" | | | |/ _ \\ '_ ` _ \\| | '_ \\ / _ \\ | | | '__|",
" | |_| |  __/ | | | | | | | | |  __/ |_| | |   ",
" |____/ \\___|_| |_| |_|_|_| |_|\\___|\\__,_|_|   "
};

int pos_x=0;	//x coordinate of the cursor
int pos_y=0;	//y coordinate of the cursor
int sel=FALSE;	//When m is pressed
int state=0;	//State of the blinking cursor
int win=0;		//The win condition : 0 unknown, 1 win, 2 lose 
int sel_menu=0; //Selection of the menu items
int screen=0;	//0 menu, 1 game, 2 end, 3 options
int num_flg=15; //Number of flags available
clock_t t1, t2;	//Time references for the blinking cursor
time_t st_time;
time_t end_time;

void initGrids(void){	//Initialise the grids
	display_grid = (char **)malloc(SIZEY * sizeof(char *));	//What is on the screen
	grid = (char **)malloc(SIZEY * sizeof(char *));;			//The numbers and the mines
	flags = (char **)malloc(SIZEY * sizeof(char *));;			//Flag storing table
	revealed = (int **)malloc(SIZEY * sizeof(int *));;		//Informations about the state of each slot : visible or not
	
	for(int i = 0; i < SIZEY; i++) { //Create the tables
        display_grid[i] = (char *)malloc(SIZEX * sizeof(char));
		grid[i] = (char *)malloc(SIZEX * sizeof(char));
		flags[i] = (char *)malloc(SIZEX * sizeof(char));
		revealed[i] = (int *)malloc(SIZEX * sizeof(int));
	}
	
	for(int i=0;i<SIZEY;i++){
		for(int j=0;j<SIZEX;j++){
			display_grid[i][j]='~';
			grid[i][j]='0';
			flags[i][j]='0';
			revealed[i][j]=0;
		}
	}
	return;
}

void freeMem(void) {
    for (int i = 0; i < SIZEY; i++) {
        free(display_grid[i]);
		free(grid[i]);
		free(flags[i]);
		free(revealed[i]);
    }
	free(display_grid);
	free(grid);
	free(flags);
	free(revealed);
}

void placeMines(void){	//Place the mines on the grid
	int minesPlaced=0;
	while(minesPlaced < MINES){
		int x = rand()%SIZEX;
		int y = rand()%SIZEY;
		if(grid[y][x]!='*'){
			grid[y][x] = '*';
			minesPlaced++;
		}
	}
	return;
}

void calcNumbers(void){	//Calculates the numbers of the grid
	for (int y = 0; y < SIZEY; y++) { //Check each slot of the grid
        for (int x = 0; x < SIZEX; x++) {
			if (grid[y][x] == '*'){	//If it is a mine no need to count
				continue;
			}
        	int count = 0;
           		for (int j = -1; j <= 1; j++) {	//Check the 8 slot around the slot
               		for (int i = -1; i <= 1; i++) {
                   		int nx = x + i;
                   		int ny = y + j;
                   		if (nx >= 0 && nx < SIZEX && ny >= 0 && ny < SIZEY && grid[ny][nx] == '*'){
							count++;
						}
					}
				}
			grid[y][x] = '0' + count;	//Add the number in the slot
		}
	}
	return;
}

void reveal(int x, int y){	//Reveal the slots
	if (x < 0 || x >= SIZEX || y < 0 || y >= SIZEY || revealed[y][x]){
		return;
	}
	revealed[y][x] = 1;
	display_grid[y][x] = grid[y][x];
	if (display_grid[y][x] == '0') {
		for (int j = -1; j <= 1; j++) {
			for (int i = -1; i <= 1; i++) {
				if (i != 0 || j != 0){
					reveal(x + i, y + j);
				}
			}
		}
	}
	return;
}

int checkWin() {	//Check if the player has won
	for (int y = 0; y < SIZEY; y++) {
		for (int x = 0; x < SIZEX; x++) {
			if (!revealed[y][x] && grid[y][x] != '*') {
				return 0;
			}
		}
	}
	return 1;
}

void drawLogo(int size){
	int offset = (size-LOGOX)/2;
	for(int j=0;j<LOGOY;j++){
		for(int i=0;i<LOGOX;i++){
			mvprintw(j,i+offset,"%c",logo[j][i]);
		}
	}
	if(sel_menu==0){
		attron(COLOR_PAIR(2));
		mvprintw(2*LOGOY-1,(size-5)/2,"JOUER");
		attroff(COLOR_PAIR(2));
		mvprintw(2*LOGOY+1,(size-7)/2,"OPTIONS");
		mvprintw(2*LOGOY+3,(size-7)/2,"QUITTER");
	}
	else if(sel_menu==1){
		mvprintw(2*LOGOY-1,(size-5)/2,"JOUER");
		attron(COLOR_PAIR(2));
		mvprintw(2*LOGOY+1,(size-7)/2,"OPTIONS");
		attroff(COLOR_PAIR(2));
		mvprintw(2*LOGOY+3,(size-7)/2,"QUITTER");
	}
	else if(sel_menu==2){
		mvprintw(2*LOGOY-1,(size-5)/2,"JOUER");
		mvprintw(2*LOGOY+1,(size-7)/2,"OPTIONS");
		attron(COLOR_PAIR(2));
		mvprintw(2*LOGOY+3,(size-7)/2,"QUITTER");
		attroff(COLOR_PAIR(2));
	}
	refresh();
	return;
}

void drawOption(int size){
	int offset = (size-LOGOX)/2;
	for(int j=0;j<LOGOY;j++){
		for(int i=0;i<LOGOX;i++){
			mvprintw(j,i+offset,"%c",logo[j][i]);
		}
	}
	mvprintw(2*LOGOY+1,(size-9)/2,"SIZE = %d",SIZEY);
	refresh();
	return;
}


void draw(int size){	//Display the game on the screen
	int x_off=1+(size-SIZEX)/2;
	int y_off=2+LOGOY+1;
	
	for(int j=0;j<LOGOY;j++){ //Draw logo
		for(int i=0;i<LOGOX;i++){
			mvprintw(j,i+(size-LOGOX)/2,"%c",logo[j][i]);
		}
	}
	for(int i=0;i<=SIZEX;i++){	//Draw the horizontal borders
		mvprintw(y_off-1,i+x_off,"\u2588");
		mvprintw(SIZEY+y_off,i+x_off,"\u2588");
	}
	for(int i=1;i<=SIZEY+2;i++){	//Draw the vertical borders
		mvprintw(i+y_off-2,x_off-1,"\u2588");
		mvprintw(i+y_off-2,SIZEX+x_off,"\u2588");
	}
	attron(COLOR_PAIR(7));
	mvprintw(y_off-2,x_off-1,"%.2f  ",difftime(end_time,st_time)); //Time counter
	attroff(COLOR_PAIR(7));
	attron(COLOR_PAIR(5));
	mvprintw(y_off-2,SIZEX+x_off-2,"%d\u2691 ",num_flg); //Flag counter
	attroff(COLOR_PAIR(5));
	for(int j=0;j<SIZEY;j++){	//Draw the grid
		for(int i=0;i<SIZEX;i++){
			if(j==pos_y && i==pos_x){	//Position of the cursor
				if(state==1){
					attron(COLOR_PAIR(1));
					if(display_grid[j][i]=='0'){
						mvprintw(j+y_off,i+x_off,"%c",' ');
					}
					else if(display_grid[j][i]=='*'){
						mvprintw(j+y_off,i+x_off,"¤");
					}
					else if(display_grid[j][i]=='f'){
					mvprintw(j+y_off,i+x_off,"\u2691");
					}
					else if(display_grid[j][i]!='0'){
						mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
					}
					attroff(COLOR_PAIR(1));
				}
				else{
					attron(COLOR_PAIR(2));
					if(display_grid[j][i]=='0'){
						mvprintw(j+y_off,i+x_off,"%c",' ');
					}
					else if(display_grid[j][i]=='*'){
						mvprintw(j+y_off,i+x_off,"¤");
					}
					else if(display_grid[j][i]=='f'){
						mvprintw(j+y_off,i+x_off,"\u2691");
					}
					else if(display_grid[j][i]!='0'){
						mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
					}
					attroff(COLOR_PAIR(2));
				}
			}
			else if(display_grid[j][i]=='0'){
				mvprintw(j+y_off,i+x_off,"%c",' ');
			}
			else if(display_grid[j][i]=='1'){
				attron(COLOR_PAIR(3));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(3));
			}
			else if(display_grid[j][i]=='2'){
				attron(COLOR_PAIR(4));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(4));
			}
			else if(display_grid[j][i]=='3'){
				attron(COLOR_PAIR(5));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(5));
			}
			else if(display_grid[j][i]=='4'){
				attron(COLOR_PAIR(6));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(6));
			}
			else if(display_grid[j][i]=='5'){
				attron(COLOR_PAIR(7));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(7));
			}
			else if(display_grid[j][i]=='6'){
				attron(COLOR_PAIR(8));
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
				attroff(COLOR_PAIR(8));
			}
			else if(display_grid[j][i]=='*'){
				mvprintw(j+y_off,i+x_off,"¤");
			}
			else if(display_grid[j][i]=='f'){
				attron(COLOR_PAIR(5));
				mvprintw(j+y_off,i+x_off,"\u2691");
				attroff(COLOR_PAIR(5));
			}
			else{
				mvprintw(j+y_off,i+x_off,"%c",display_grid[j][i]);
			}
		}
	}
	refresh();
	return;
}

void revealAll(void){
	for(int j=0;j<SIZEY;j++){	//Draw the grid
		for(int i=0;i<SIZEX;i++){
			display_grid[j][i]=grid[j][i];	
		}
	}
	return;
}

void handle_menu(int ch){
	if(ch==KEY_UP){
		switch(sel_menu){
			case 0: sel_menu=2; break;
			case 1: sel_menu=0; break;
			case 2: sel_menu=1; break;
		}
	}
	else if(ch==KEY_DOWN){
		switch(sel_menu){
			case 0: sel_menu=1; break;
			case 1: sel_menu=2; break;
			case 2: sel_menu=0; break;
		}
	}
	else if(ch=='\n'){
		if(sel_menu==0){
			screen=1;
		}
		else if(sel_menu==1){
			screen=3;
		}
		else if(sel_menu==2){
			screen=2;
		}
	}
	return;
}

void handle_option(int ch){
	if(ch == KEY_UP){
		if(SIZEY<32){
			SIZEY++;
		}
		SIZEX=2*SIZEY;
		MINES=(int)(10*SIZEX*SIZEY/100);
		num_flg=MINES;
	}
	else if(ch == KEY_DOWN){
		if(SIZEY>10){
			SIZEY--;
		}
		SIZEX=2*SIZEY;
		MINES=(int)(10*SIZEX*SIZEY/100);
		num_flg=MINES;
	}
	else if(ch == '\n'){
		screen=0;
	}
}

void handle_input(int ch){	//Executed when a key is pressed
	int newx=pos_x;
	int newy=pos_y;
	
	switch (ch){
		case KEY_UP: newy--; break;
		case KEY_DOWN: newy++; break;
		case KEY_RIGHT: newx++; break;
		case KEY_LEFT: newx--; break;
		case '\n': sel=TRUE; break;
		case 'f':
			if(display_grid[pos_y][pos_x] != 'f' && num_flg !=0){
				flags[pos_y][pos_x]=display_grid[pos_y][pos_x];
				display_grid[pos_y][pos_x]='f';
				num_flg--;
			}
			else if(display_grid[pos_y][pos_x] == 'f'){
				display_grid[pos_y][pos_x]=flags[pos_y][pos_x];
				num_flg++;
			}
			break;
	}
	
	if(ch !='\n'){
		if(newx != pos_x && 0 <= newx && newx < SIZEX){
			pos_x=newx;
			state=0;
		}
		if(newy != pos_y && 0 <= newy && newy < SIZEY){
			pos_y=newy;
			state=0;
		}
	}
	return;
}

int main(void){
	setlocale(LC_ALL,"");
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr,TRUE);
	curs_set(0);
	nodelay(stdscr,TRUE);
	init_pair(1,COLOR_WHITE,COLOR_BLACK);
	init_pair(2,COLOR_BLACK,COLOR_WHITE);
	init_pair(3,COLOR_BLUE,COLOR_BLACK);
	init_pair(4,COLOR_GREEN,COLOR_BLACK);
	init_pair(5,COLOR_RED,COLOR_BLACK);
	init_pair(6,COLOR_CYAN,COLOR_BLACK);
	init_pair(7,COLOR_MAGENTA,COLOR_BLACK);
	init_pair(7,COLOR_YELLOW,COLOR_BLACK);
	init_pair(8,COLOR_BLACK,COLOR_CYAN);
	int ch;
	int row, col, row_tmp, col_tmp;
	getmaxyx(stdscr, row_tmp, col_tmp);
	
	while((ch=getch())!='q'){	//Title screen
		getmaxyx(stdscr, row, col);
		handle_menu(ch);
		if(screen==1){
			break;
		}
		if(screen==2){
			clear();
			endwin();
			return 0;
		}
		if(screen==3){
			clear();
			while((ch=getch())!='q'){
				if(screen == 0){ //Size selection has been done
					break;
				}
				else{
					handle_option(ch);
					if(row_tmp !=row || col_tmp != col){
						clear();
					}
					row_tmp=row;
					col_tmp=col;
					if(row>LOGOY*4 && col>LOGOX){
						drawOption(col);
					}
					else{
						mvprintw(0,0,"Terminal trop petit pour jouer");
					}
				}
			}
			clear();
			screen=0;
		}
		
		if(row_tmp !=row || col_tmp != col){
			clear();
		}
		row_tmp=row;
		col_tmp=col;
		if(row>LOGOY*4 && col>LOGOX){
			drawLogo(col);
		}
		else{
			mvprintw(0,0,"Terminal trop petit pour jouer");
		}
	}
	if(screen==0){	//Hit q on the main menu
		clear();
		endwin();
		return 0;
	}
		
	srand(time(NULL));
	initGrids();
	placeMines();
	calcNumbers();
	clear();
	draw(col);
	t1=clock();	
	time(&st_time);
	while((ch=getch())!='q'){	//Game loop
		time(&end_time);
		handle_input(ch);
		getmaxyx(stdscr, row, col);
		
		if(sel==TRUE){
			sel=FALSE;
			if(grid[pos_y][pos_x]=='*'){	//You lose
				win=2;
				reveal(pos_x,pos_y);
				break;
			}
			reveal(pos_x,pos_y);
			if(checkWin()){	//You won
				win=1;
				break;
			}
		}
		t2=clock();
		if(((double)(t2-t1)*1000/CLOCKS_PER_SEC)>500){
			t1=clock();
			state=1-state;
		}
		if(row_tmp !=row || col_tmp != col){
			clear();
		}
		row_tmp=row;
		col_tmp=col;
		draw(col);
	}
	revealAll();
	draw(col);
	int starty = (row - 3) / 2;
	int startx = (col - 10) / 2;
	WINDOW *wind = newwin(3, 12, starty, startx);
	wbkgd(wind, COLOR_PAIR(8));
	while((ch=getch())!='q'){	//End screen
		if(win==1){
			int starty = (row - 3) / 2;
			int startx = (col - 10) / 2;
			mvwprintw(wind, 1, 2, "You Win!");
			wrefresh(wind);
		}
		else if(win==2){
			mvwprintw(wind, 1, 2, "You Lose");
			wrefresh(wind);
		}
		else{
			break;
		}	
	}
	delwin(wind);
	draw(col);
	while((ch=getch())!='q'){
		
	}
	clear();
	endwin();
	freeMem();
	return 0;
}
