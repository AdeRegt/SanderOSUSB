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
#define MAX_DRAWABLE_COUNT 10

#define SHAPE_TASKBAR 1
#define SHAPE_BUTTON 2

struct DrawableComponent{
    int x;
    int y;
    int shape;
    int w;
    int h;
    unsigned long info;
    unsigned long onclick;
    unsigned char active;
    unsigned char selected;
} __attribute__ ((packed));

struct DrawableProgram{
    unsigned char is_active;
    unsigned char name[10];
    unsigned char always0;
    unsigned char needs_to_be_drawn;
    unsigned char has_focus;
    struct DrawableComponent drawable[10];
    Register registers;
    unsigned long data;
} __attribute__ ((packed));

struct DrawableProgram programs[MAX_PROGRAM_COUNT];

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
                        }else if(dw.has_focus==1){
                            if(cmp->shape==SHAPE_BUTTON){
                                for(int w = 0 ; w < cmp->w ;w++){
                                    for(int h = 0 ; h < cmp->h ;h++){
                                        putpixel(cmp->x+w,TASKBAR_HEIGHT + 5 + cmp->y+h,COLOR_YELLOW);
                                    }
                                }
                                drawstringat((char*)cmp->info,cmp->x,TASKBAR_HEIGHT + 5 + cmp->y,0);
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
                                        
                                        ((struct DrawableProgram*)&programs)[i].has_focus = 0;
                                        ((struct DrawableProgram*)&programs)[z-1].has_focus = 1;
                                        debugf("Switch to task %x (original=%x) \n",z-1,i);
                                        if(dw.registers.eflags!=0){
                                            debugf("return to previous state\n");
                                            r->gs = dw.registers.gs; 
                                            r->fs = dw.registers.fs; 
                                            r->es = dw.registers.es;
                                            r->ds = dw.registers.ds;
                                            r->edi = dw.registers.edi;
                                            r->esi = dw.registers.esi;
                                            r->ebp = dw.registers.ebp;
                                            r->esp = dw.registers.esp;
                                            r->ebx = dw.registers.ebx;
                                            r->edx = dw.registers.edx;
                                            r->ecx = dw.registers.ecx;
                                            r->eax = dw.registers.eax;
                                            r->eip = dw.registers.eip;
                                            r->cs = dw.registers.cs;
                                            r->eflags = dw.registers.eflags;
                                            r->useresp = dw.registers.useresp;
                                            r->ss = dw.registers.ss;
                                        }else{
                                            debugf("apply new state\n");
                                            r->eip = dw.registers.eip;
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
        pointer[0x00] = switch_endian16( 0b1000000000000000 );
        pointer[0x01] = switch_endian16( 0b1100000000000000 );
        pointer[0x02] = switch_endian16( 0b1110000000000000 );
        pointer[0x03] = switch_endian16( 0b1111000000000000 );
        pointer[0x04] = switch_endian16( 0b1111100000000000 );
        pointer[0x05] = switch_endian16( 0b1111110000000000 );
        pointer[0x06] = switch_endian16( 0b1111111000000000 );
        pointer[0x07] = switch_endian16( 0b1111111100000000 );
        pointer[0x08] = switch_endian16( 0b1111111110000000 );
        pointer[0x09] = switch_endian16( 0b1100110011000000 );
        pointer[0x0a] = switch_endian16( 0b1000011000100000 );
        pointer[0x0b] = switch_endian16( 0b0000001100000000 );
        pointer[0x0c] = switch_endian16( 0b0000000110000000 );
        pointer[0x0d] = switch_endian16( 0b0000000011000000 );
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

void browser2();

unsigned long handlebrowserclick(struct DrawableComponent *a){
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    unsigned char *filename = (unsigned char*) a->info;
    unsigned char *path = (unsigned char*) program->data;
    char* res;
    if(path[0]=='@'){
        res = sprintf("%c@",filename[0]);
    }else{
        res = sprintf("%s/%s",path,filename);
    }
    program->data = (unsigned long)res;
    return (unsigned long)&browser2;
}

void browser2(){
    struct DrawableProgram* program = (struct DrawableProgram*)&programs;
    char *browserpath = dir((char*)program[0].data);
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