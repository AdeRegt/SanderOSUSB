#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define addresssize 8

int main(int argc,char** argv){
	remove("../include/symbols.h");
	FILE *bestand = fopen("../symbols","r");
	FILE *writer = fopen("../include/symbols.h","w");
	if(bestand){
		fseek(bestand, 0, SEEK_END);
		size_t filelen = ftell(bestand);
		size_t lengte = 0;
		rewind(bestand);
		while(1){
			unsigned char buffer[addresssize];
			unsigned char isok = 0;
			size_t read = fread(buffer,addresssize,1,bestand);
			lengte += 5;

			for(int i = 0 ; i < 7 ; i++){
				lengte++;
				unsigned char buffer3[1];
				fread(buffer3,1,1,bestand);
			}

			lengte++;
			unsigned char buffer4[1];
			fread(buffer4,1,1,bestand);

			if(buffer4[0]=='F'){
				isok = 1;
			}

			lengte++;
			unsigned char buffer5[1];
			fread(buffer5,1,1,bestand);

			nogmaals:
			lengte++;
			unsigned char buffer6[1];
			fread(buffer6,1,1,bestand);
			if(buffer6[0]!='\t'&&buffer6[0]!='\n'){
				goto nogmaals;
			}

			if(buffer6[0]!='\n'){
				if(isok){
					fprintf(writer,"#define F_");
				}

				unsigned char buffer9[addresssize+1];
				fread(buffer9,addresssize+1,1,bestand);

				unsigned char tor = 0;
				nogmaals2:
				lengte++;
				unsigned char buffer10[1];
				fread(buffer10,1,1,bestand);
				if(buffer10[0]!='\n'){
					if(isok&&buffer10[0]!='.'&&buffer10[0]!=' '){
						tor++;
						fprintf(writer,"%c",toupper(buffer10[0]));
					}
					goto nogmaals2;
				}

				if(isok){
					if(tor<10){
						fprintf(writer,"\t");
					}
					if(tor<20){
						fprintf(writer,"\t");
					}
					fprintf(writer,"\t0x%c%c%c%c%c%c%c%c\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
				}
			}

			if(lengte>filelen){
				break;
			}
		}
		fclose(bestand);
		fclose(writer);
	}else{
		printf("ERROR: unable to open file\n");
	}
	exit(0);
}