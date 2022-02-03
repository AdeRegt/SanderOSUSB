#include "../kernel.h"


#define SCREEN_WIDTH 340
#define SCREEN_HEIGHT 200

#define TICKINTERVAL 20

#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_YELLOW 6
#define COLOR_GRAY 7
#define COLOR_DARKBLUE 8
#define COLOR_DEEPBLUE 9
#define COLOR_DEEPGREEN 10
#define COLOR_LIGHTBLUE 11

#define DESKTOP_COLOR COLOR_CYAN
#define TASKBAR_COLOR COLOR_GRAY
#define TASKBAR_HEIGHT 10

#define MAX_PROGRAM_COUNT 3
#define MAX_DRAWABLE_COUNT 20

#define SHAPE_TASKBAR 1
#define SHAPE_BUTTON 2
#define SHAPE_CONSOLE 3

struct DrawableComponent{
    volatile int x;
    volatile int y;
    volatile int shape;
    volatile int w;
    volatile int h;
    volatile unsigned long info;
    volatile unsigned long onclick;
    volatile unsigned char active;
    volatile unsigned char selected;
} __attribute__ ((packed));

struct DrawableProgram{
    volatile unsigned char is_active;
    volatile unsigned char name[10];
    volatile unsigned char always0;
    volatile unsigned char needs_to_be_drawn;
    volatile unsigned char has_focus;
    volatile struct DrawableComponent drawable[MAX_DRAWABLE_COUNT];
    volatile Register registers;
    volatile unsigned long data;
    volatile unsigned long data2;
} __attribute__ ((packed));

volatile struct DrawableProgram programs[MAX_PROGRAM_COUNT];

int timerticks = 0;
int enable_window_manager = 0;
void handle_window_manager_interrupt(Register *r){
    if(((int*)enable_window_manager)[0]!=1){
        return;
    }
    if(!isGraphicsMode()){
        return;
    }
    timerticks++;
    if(timerticks==TICKINTERVAL){
        timerticks = 0;
        
        // 
        // drawing screen methods
        // first clear the screen
        for(unsigned int x = 0 ; x < SCREEN_WIDTH ; x++){
			for(unsigned int y = 0 ; y < SCREEN_HEIGHT ; y++){
				putpixel(x,y,DESKTOP_COLOR);
			}
		}
        // then create taskbar
        for(unsigned int x = 0 ; x < SCREEN_WIDTH ; x++){
			for(unsigned int y = 0 ; y < TASKBAR_HEIGHT ; y++){
				putpixel(x,y,TASKBAR_COLOR);
			}
		}

        //
        // get hw state
        InputStatus is = getInputStatus();
        unsigned short curx = is.mouse_x;
        unsigned short cury = is.mouse_y;

        if(curx<2){
            curx = 2;
        }
        if(cury<2){
            cury = 2;
        }
        if(curx>(SCREEN_WIDTH-1)){
            curx = (SCREEN_WIDTH-1);
        }
        if(cury>(SCREEN_HEIGHT-10)){
            cury = (SCREEN_HEIGHT-10);
        }
        curset(curx,cury);

        //
        // drawing components
        struct DrawableProgram* program = (struct DrawableProgram*)&programs;
        int tow = 0;
        for(int i = 0 ; i < MAX_PROGRAM_COUNT ; i++){
            struct DrawableProgram dw = program[i];
            if(dw.is_active){
                tow += 80;
                if(dw.has_focus){
                    dw.registers.gs = r->gs; 
                    dw.registers.fs = r->fs; 
                    dw.registers.es = r->es;
                    dw.registers.ds = r->ds;
                    dw.registers.edi = r->edi;
                    dw.registers.esi = r->esi;
                    dw.registers.ebp = r->ebp;
                    dw.registers.esp = r->esp;
                    dw.registers.ebx = r->ebx;
                    dw.registers.edx = r->edx;
                    dw.registers.ecx = r->ecx;
                    dw.registers.eax = r->eax;
                    dw.registers.eip = r->eip;
                    dw.registers.cs = r->cs;
                    dw.registers.eflags = r->eflags;
                    dw.registers.useresp = r->useresp;
                    dw.registers.ss = r->ss;
                }
                for(int z = 0 ; z < MAX_DRAWABLE_COUNT ; z++){
                    struct DrawableComponent *cmp = (struct DrawableComponent*)&dw.drawable[z];
                    if(cmp->active){
                        if(cmp->shape==SHAPE_TASKBAR){
                            for(int w = 0 ; w < cmp->w ;w++){
                                for(int h = 0 ; h < cmp->h ;h++){
                                    putpixel(cmp->x+w,cmp->y+h,TASKBAR_COLOR);
                                }
                            }
                            drawstringat((char*)cmp->info,cmp->x,cmp->y,0);
                        }else if(((unsigned char*)&((struct DrawableProgram*)&programs)[i].has_focus)[0]==1){
                            if(cmp->shape==SHAPE_BUTTON){
                                for(int w = 0 ; w < cmp->w ;w++){
                                    for(int h = 0 ; h < cmp->h ;h++){
                                        putpixel(cmp->x+w,TASKBAR_HEIGHT + 5 + cmp->y+h,COLOR_YELLOW);
                                    }
                                }
                                drawstringat((char*)cmp->info,cmp->x,TASKBAR_HEIGHT + 5 + cmp->y,0);
                            }else if(cmp->shape==SHAPE_CONSOLE){
                                // debugf("shape_console: %s \n",(volatile char*)getSTDOUTBuffer());
                                drawstringat((char*)getSTDOUTBuffer(),cmp->x,TASKBAR_HEIGHT + 5 + cmp->y,0);
                            }
                        }
                        // debugf("curx=%x cury=%x butx=%x buty=%x butxw=%x butyh=%x \n",curx,cury,cmp->x,cmp->y,cmp->x+cmp->w,cmp->y+cmp->h);
                        if(cmp->x<curx&&(cmp->x+cmp->w)>curx){
                            if(cmp->y<(cury-(cmp->shape==SHAPE_TASKBAR?0:(TASKBAR_HEIGHT + 5)))&&(cmp->y+cmp->h)>(cury-(cmp->shape==SHAPE_TASKBAR?0:(TASKBAR_HEIGHT + 5)))){
                                // debugf("button touched\n");
                                if(is.mousePressed){
                                    // debugf("button pressed\n");
                                    debugf("button activated, program=%x drawable=%x \n",i,z);
                                    if(cmp->onclick){
                                        debugf("calling custom function\n");
                                        unsigned long (*sendPackage)(struct DrawableComponent *a) = (void*)cmp->onclick;
                                        unsigned long gz = sendPackage(cmp);
                                        if(gz){
                                            r->eip = gz;
                                        }
                                    }else{
                                        
                                        ((unsigned char*)&((struct DrawableProgram*)&programs)[i].has_focus)[0] = 0;
                                        ((unsigned char*)&((struct DrawableProgram*)&programs)[z-1].has_focus)[0] = 1;
                                        struct DrawableProgram bs = ((struct DrawableProgram*)&programs)[z-1];
                                        debugf("Switch to task %x (original=%x) EIP=%x \n",z-1,i,bs.registers.eip);
                                        if(bs.registers.eflags!=0){
                                            debugf("return to previous state\n");
                                            r->gs = bs.registers.gs; 
                                            r->fs = bs.registers.fs; 
                                            r->es = bs.registers.es;
                                            r->ds = bs.registers.ds;
                                            r->edi = bs.registers.edi;
                                            r->esi = bs.registers.esi;
                                            r->ebp = bs.registers.ebp;
                                            r->esp = bs.registers.esp;
                                            r->ebx = bs.registers.ebx;
                                            r->edx = bs.registers.edx;
                                            r->ecx = bs.registers.ecx;
                                            r->eax = bs.registers.eax;
                                            r->eip = bs.registers.eip;
                                            r->cs = bs.registers.cs;
                                            r->eflags = bs.registers.eflags;
                                            r->useresp = bs.registers.useresp;
                                            r->ss = bs.registers.ss;
                                        }else{
                                            debugf("apply new state\n");
                                            r->eip = bs.registers.eip;
                                        }
                                    }
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }

        //
        // drawing cursor
        
        unsigned short pointer[12];
        pointer[0x00] = ( 0b0000000000000001 );
        pointer[0x01] = ( 0b0000000000000011 );
        pointer[0x02] = ( 0b0000000000000111 );
        pointer[0x03] = ( 0b0000000000001111 );
        pointer[0x04] = ( 0b0000000000011111 );
        pointer[0x05] = ( 0b0000000000111111 );
        pointer[0x06] = ( 0b0000000001111111 );
        pointer[0x07] = ( 0b0000000011111111 );
        pointer[0x08] = ( 0b0000000111111111 );
        pointer[0x09] = ( 0b0000001100110011 );
        pointer[0x0a] = ( 0b0000010000110001 );
        pointer[0x0b] = ( 0b0000000001100000 );
        pointer[0x0c] = ( 0b0000000011000000 );
        pointer[0x0d] = ( 0b0000000110000000 );
        for(int y = 0 ; y < 12 ; y++){
            unsigned short pd = pointer[y];
            for(int x = 0 ; x < 16 ; x++){
                unsigned short tw = (pd>>x)&1;
                if(tw){
                    putpixel(curx + x,cury + y,COLOR_RED);
                }
            }
        }

    }
}

int initialiseProgramTab(char* name,unsigned long ep){
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    int i = 0;
    for(i = 0 ; i < MAX_PROGRAM_COUNT ; i++){
        if(program[i].is_active==0){
            break;
        }
    }
    struct DrawableProgram *pg = (struct DrawableProgram *)&programs[i];
    pg->is_active = 1;
    for(int z = 0 ; z < 10 ; z++){
        pg->name[z] = name[z];
        if(name[z]==0){
            break;
        }
    }
    pg->registers.eip = ep;
    struct DrawableComponent* dc = (struct DrawableComponent*) &pg->drawable[0];
    dc->active = 1;
    dc->h = 500;
    dc->shape = SHAPE_CONSOLE;
    dc->x = 1;
    dc->y = 1;
    return i;
}

void browser2();

void loadandexecprog(){
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    debugf("program loaded!\n");
    unsigned char* buffer = (unsigned char*)0x2000;
    char* pt = (char*) program[0].data2;
    if(fexists((unsigned char*)pt) && fread(pt,buffer)){
        unsigned long loc = 0;
        if(iself(buffer)){
            debugf("ELF: program is ELF!\n");
            loc = loadelf(buffer);
        }else{
            loc = 0x2000;
        }
        initialiseProgramTab(pt,loc);
    }
    for(;;);
}

unsigned long handlebrowserclick(struct DrawableComponent *a){
    volatile struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    volatile char *filename = (volatile char*) a->info;
    volatile char *path = (volatile char*) program->data;
    char* res;
    if(path[0]=='@'){
        res = sprintf("%c@",filename[0]);
    }else if(strlen((char*)path)==2){
        res = sprintf("%s%s",path,filename);
    }else{
        res = sprintf("%s/%s",path,filename);
    }
    // debugf("path=%s filename=%s res=%s \n",path,filename,res);
    int fileorfolder = 0;
    for(int i = 0 ; i < strlen((char*)filename) ; i++){
        if(filename[i]=='.'){
            fileorfolder = 1;
        }
    }
    // this is a file
    if(fileorfolder==1){
        program->data2 = (unsigned long)res;
        return (unsigned long)&loadandexecprog;
    }
    // this is a folder
    if(fileorfolder==0){
        program->data = (unsigned long)res;
        return (unsigned long)&browser2;
    }
    program->data = (unsigned long)res;
    return (unsigned long)&browser2;
}

void browser2(){
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    char *browserpath = dir((char*)program[0].data);
    debugf("browser: now opening %s \n",(char*)program[0].data);
    int l = strlen(browserpath);
    int t = 0;
    int z = 10;
    int h = 4;

    for(int i = h ; i < MAX_DRAWABLE_COUNT ; i++){
        program[0].drawable[i].active   = 0;
    }
    for(int g = 0 ; g < l ; g++){
        if(browserpath[g]==';'){
            browserpath[g] = 0;
        }
    }
    for(int i = 0 ; i < l ; i++){
        if(i==0||browserpath[i-1]==';'||browserpath[i-1]==0){
            browserpath[i-1] = 0;
            program[0].drawable[h].active   = 1;
            program[0].drawable[h].shape    = SHAPE_BUTTON;
            program[0].drawable[h].x        = t;
            program[0].drawable[h].y        = z;
            program[0].drawable[h].w        = 70;
            program[0].drawable[h].h        = 10;
            program[0].drawable[h].onclick  = (unsigned long)&handlebrowserclick;
            program[0].drawable[h].info     = (unsigned long)(browserpath+i);
            program[0].drawable[h].selected = 0;

            t+= 80;
            h++;
            if(t==320){
                t = 0;
                z += 15;
            }
        }
    }
    for(;;);
}

void init_windowmanager(){
    // lets initialise our programs
    // default program
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    program[0].is_active            = 1;
    program[0].name[0]              = 'p';
    program[0].name[1]              = 'r';
    program[0].name[2]              = 'o';
    program[0].name[3]              = 'g';
    program[0].name[4]              = 'r';
    program[0].name[5]              = 'a';
    program[0].name[6]              = 'm';
    program[0].name[7]              = 's';
    program[0].needs_to_be_drawn    = 1;
    program[0].has_focus            = 0;
    program[0].data                 = (unsigned long)"@";
    program[0].registers.eip        = (unsigned long)&browser2;

    // poweroff
    program[0].drawable[0].active   = 1;
    program[0].drawable[0].shape    = SHAPE_TASKBAR;
    program[0].drawable[0].x        = SCREEN_WIDTH-90;
    program[0].drawable[0].y        = 0;
    program[0].drawable[0].w        = 70;
    program[0].drawable[0].h        = TASKBAR_HEIGHT;
    program[0].drawable[0].onclick  = (unsigned long)&poweroff;
    program[0].drawable[0].info     = (unsigned long)"poweroff";
    program[0].drawable[0].selected = 0;
    // program 1
    program[0].drawable[1].active   = 1;
    program[0].drawable[1].shape    = SHAPE_TASKBAR;
    program[0].drawable[1].x        = (0*70);
    program[0].drawable[1].y        = 0;
    program[0].drawable[1].w        = 70;
    program[0].drawable[1].h        = TASKBAR_HEIGHT;
    program[0].drawable[1].onclick  = 0;
    program[0].drawable[1].info     = (unsigned long)&program[0].name;
    program[0].drawable[1].selected = 0;
    // program 2
    program[0].drawable[2].active   = 1;
    program[0].drawable[2].shape    = SHAPE_TASKBAR;
    program[0].drawable[2].x        = (1*70);
    program[0].drawable[2].y        = 0;
    program[0].drawable[2].w        = 70;
    program[0].drawable[2].h        = TASKBAR_HEIGHT;
    program[0].drawable[2].onclick  = 0;
    program[0].drawable[2].info     = (unsigned long)&program[1].name;
    program[0].drawable[2].selected = 0;
    // program 3
    program[0].drawable[3].active   = 1;
    program[0].drawable[3].shape    = SHAPE_TASKBAR;
    program[0].drawable[3].x        = (2*70);
    program[0].drawable[3].y        = 0;
    program[0].drawable[3].w        = 70;
    program[0].drawable[3].h        = TASKBAR_HEIGHT;
    program[0].drawable[3].onclick  = 0;
    program[0].drawable[3].info     = (unsigned long)&program[2].name;
    program[0].drawable[3].selected = 0;

    // lets go
    ((int*)enable_window_manager)[0] = 1;
}