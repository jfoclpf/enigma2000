// Enigma 2000

// Six phases file encoder

// Made by joao.pimentel.ferreira@gmail.com

//	I don't care about code efficiency
//	It just matters that the program can really encrypt well
//	As much more redundacies better, more time take to decode using this proogram,
//unless someone can erase them

#include <assert.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
using namespace std;

void rotate_cube_matrix(char *** &,int,int,char,int);
void xor_vect(char *, char *,int);						//vect1^vect2 and put the result in vect1
short len_str(char *);									//size of string
int strsum(char*);										//sum all the charachters of the string
void fase1a(int, char* , char*);
void fase1b(int, int,char* ,char* ,char );
void fase1c(char*);
void fase2a(char* ,char* ,char );
void fase2b(char* ,long int ,char );
void fase3(char* , char* , int ,char);
void fase4(char* , long int );
void fase5(char* ,char* ,int, char,long int );
void rotate_cube_matrix(char *** & ,int,int ,char ,int );
void fase6(char*);
long GetFileSize(std::string filename);

int main(int argc, char *argv[] ) {

	if (argc<4 || len_str(argv[2])<8 || len_str(argv[2])>16 || (strcmp("c",argv[3]) && strcmp("d",argv[3])) ){
		cout << "Ex:" << endl << "coder [file to encode] [password (min 8 char, max 16)] [mode ('c'oder 'd'ecoder)]";
		return 0;
	}

	short int N=len_str(argv[2]);			//size of the password
	char * password = new char[N+1];
	strcpy(password,argv[2]);				//to complicate
	password[N+1]=argv[2][N];

	char mode=argv[3][0];					//mode (coder or decoder)

	fstream is( argv[1], ios::binary | ios::in | ios::out);
	if(!is){
		cout << "File " << argv[1] << " doesn't exist." ;
		return 0;
	}

	long int file_size=GetFileSize(argv[1]); //size of the file

	if(file_size<=N){
		cout << "The information to encode can't be less or equal the size of the passowrd";
		return 0;
	}

	int BLOCO=N+2+ int((strsum(argv[2])%N)/2);		 //size รณf the BLOCK in blocks

	if ((file_size%N)==0){					 //to ensure confusion in the process of
		password[N]=strsum(argv[2]);		 //codification on the sucessive cycles
		N++;								 //when the file is reversed
	}
											 //(N<BLOCK)
	is.close();

	int i,k,Ncycles;
	 __int8 n,aux,pos [6];
	for(i=0;i<6;i++)                         //6 phases sequency
		pos[i]=i+1;
	for(i=0;i<6;i++){
		n= __int8(password[i])%(6-i);
		aux=pos[i];
		pos[i]=pos[i+n];
		pos[i+n]=aux;
	}
	for(i=5;i>=0;i--){
		n= __int8(password[N-1-i])%(i+1);
		aux=pos[i];
		pos[i]=pos[i-n];
		pos[i-n]=aux;
	}
													 //The sequncy of phases depends on the passowrd
	for(i=0;i<6;i++){
		switch(mode=='c' ? pos[i] : pos[5-i]){
		case 1:
			Ncycles=int(BLOCO*N)/8;					 //Number of cycles
			if (Ncycles%2==0)						 //Odd number because complicates the final message
				Ncycles++;
			if(mode=='c')
				for(k=0;k<Ncycles;k++){
					fase1a(N, argv[1],password);
					fase1b(N,BLOCO,argv[1],password,mode);
					fase1c(argv[1]);
				}
			else
				for(k=0;k<Ncycles;k++){
					fase1c(argv[1]);
					fase1b(N,BLOCO,argv[1],password,mode);
					fase1a(N,argv[1],password);
				}
			break;

		case 2:
			if(mode=='c'){
				fase2a(argv[1],password,mode);
				fase2b(argv[1],file_size,mode);
			}
			else{
				fase2b(argv[1],file_size,mode);
				fase2a(argv[1],password,mode);
			}
			break;

		case 3:
			fase3(argv[1],password,N,mode);
			break;

		case 4:
			fase4(argv[1],file_size);
			break;

		case 5:
			fase5(argv[1],password,N,mode,file_size);
			break;

		case 6:
			fase6(argv[1]);
			break;

		default:
			assert(false);
		}
	}

	return 0;
}


//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************

void xor_vect(char * vect1, char *vect2,int n){ //vect1^vect2 and put the result in vect1
	for(int ct=0;ct<n;ct++)
		vect1[ct]=vect1[ct]^vect2[ct];
}

short len_str(char * cp){  //size of string
	short aux=0;
	while(cp[aux]!='\0'){
		aux++;
	}
	return aux;
}

int strsum(char* cp){ //sum all the charachters of the string

	int ind=0,sum=0;
	while (cp[ind]!='\0'){
		sum+=cp[ind];
		ind++;
	}
	return sum;
}

//-----Phase 1a of codification----- It Codifies successives blocks of the message ----------------
//-- using the password string (xor) --------------------------------------------------------------
void fase1a(int N, char * filename, char * password){

	fstream is( filename, ios::binary | ios::in | ios::out);
	char* array_data=new char[N];
	while (1){

		is.read(array_data,N);

		if(is.eof()){				//codifies the missing piece of the message (file)
			int lct=is.gcount();
			is.close();

			fstream isa( filename, ios::binary | ios::in | ios::out);
			isa.seekp(-lct,ios::end);

			xor_vect(array_data,password,lct-2);

			isa.write(array_data,lct);
			isa.close();

			break;
		}

		xor_vect(array_data,password,N);
		is.seekp(-N,ios::cur);
		is.write(array_data,N);
	}
	delete array_data;
}//-----------------------------


//----Phase 1b------------Change the blocks positions using the password ---------------
//-------------------- Blocks size depends on the password -----------------------------
//-- N = size of the password --- BLOCO = size of the blocks ----- N<BLOCO<=2N ---------
void fase1b(int N, int BLOCO,char* filename,char* password,char mode){

	int i;
	char** data_matrix=new char*[BLOCO]; //create the matrix
	for (i=0;i<BLOCO;i++)				 //where we store the data to encode
		data_matrix[i]=new char[N];

	//set the new positions to N bytes blocks
	__int8* pos= new __int8[BLOCO];
	for(i=0;i<BLOCO;i++)
		pos[i]=i;

	 __int8 n,aux;									//N<BLOCO
	for(i=0;i<N;i++){
		n= __int8(password[i])%(BLOCO-i);		//arrange from beggining
		aux=pos[i+n];
		pos[i+n]=pos[i];
		pos[i]=aux;
	}
	for(i=0;i<N;i++){
		n= __int8(password[N-i-1])%(BLOCO-i);	//arrange from the end
		aux=pos[BLOCO-i-n-1];
		pos[BLOCO-i-n-1]=pos[BLOCO-i-1];
		pos[BLOCO-i-1]=aux;
	}
	//----------------------------------------

	fstream is(filename, ios::binary | ios::in | ios::out);
	while(1){

		//insert in the new positions
		for (i=0;i<BLOCO;i++)
			is.read(data_matrix[mode=='c'?pos[i]:i],N);

		if(is.eof()){
			is.close();
			break;
		}

		is.seekp(-(BLOCO*N),ios::cur);

		for (i=0;i<BLOCO;i++)
			is.write(data_matrix[mode=='c'?i:pos[i]],N);
	}


	for (i=0;i<BLOCO;i++)				 //where we store the data to encode
		delete [] data_matrix[i];
	delete [] data_matrix;
	delete [] pos;

}//--------------------------


//-----Phase 1c --------------Invert the message (file) ------------------------
void fase1c(char* filename){

	int fh_i=_open(filename, _O_RDWR );
	_setmode(fh_i,_O_BINARY);			//set to binary mode
	int fh_f=_open(filename, _O_RDWR );	//get second handle
	_setmode(fh_f,_O_BINARY);			//set to binary mode
	_lseek(fh_f,-3,SEEK_END);           //because the 2 last bytes of the file aren't used

	char datai,dataf;
	while(_tell(fh_i)<_tell(fh_f)){

		_read(fh_i,&datai,1);
		_lseek(fh_i,-1,SEEK_CUR);

		_read(fh_f,&dataf,1);
		_lseek(fh_f,-1,SEEK_CUR);
		_write(fh_f,&datai,1);
		_lseek(fh_f,-2,SEEK_CUR);

		_write(fh_i,&dataf,1);
	}
	_close(fh_i);
	_close(fh_f);

}//----End of the inversion -------------

//----- Phase 2a ----------------------------------------------------------------------------------
//-- Codifies (xor) byte [n] with byte [n-1] ------------------------------------------------------
void fase2a(char* filename,char* password,char mode){

	int fh=_open(filename, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode

	//fase2a
	__int8 data1,data2,X;
	X=(__int8(strsum(password)%255));

	_read(fh,&data1,1);
	_lseek(fh,-1,SEEK_CUR);
	data2=data1^X;
	_write(fh,&data2,1);
	if(mode=='d') data1=data2;
	while(!_eof(fh)){

		_read(fh,&data2,1);
		data2=data2^data1;
		_lseek(fh,-1,SEEK_CUR);
		_write(fh,&data2,1);
		if(mode=='c')
			data1=data2;
		else
			data1=data2^data1;
	}

	_close(fh);

}

//------- Phase 2b --------------------------------------------------------------------------
//-------------- Rotate in blocks of 2 bytes depending on the file size ---------------------
void fase2b(char* filename,long int file_size,char mode){

	int fh=_open(filename, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode

	int size_i=sizeof( int);

	 int data;

	int br=file_size%(size_i*8);
	if (br==0)
		br++;

	while(1){

		_read(fh,&data,size_i);
		if(_eof(fh))
			break;

		_lseek(fh,-size_i,SEEK_CUR);
		data=mode=='c'?_rotl(data,br):_rotr(data,br);
		_write(fh,&data,size_i);
	}
	_close(fh);
}

//---- Phase3 ------ It Inverts and codifes -------------------------------------------------------
void fase3(char* filename, char* password, int N,char mode){

	if (mode=='c')
		fase1c(filename);

	int fh=_open(filename, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode
	char data;

	long int index=1;

	while(!_eof(fh)){
		_read(fh,&data,1);

		data=(index%(N+ __int8(password[N-3])))==0 ?
			data^password[N-2] : data^password[N-1];

		_lseek(fh,-1,SEEK_CUR);
		_write(fh,&data,1);

		index++;
	}
	_close(fh);

	if (mode=='d')
		fase1c(filename);

}

//---- Phase4 -- Change the last byte by the first one --------------------------------------------
//---- If the message size is even it also changes the middle bytes -------------------------------
void fase4(char* filename, long int file_size){

	int fh=_open(filename, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode

	char beg,end;
	_read(fh,&beg,1);						//Changes the first by the last
	_lseek(fh,-3,SEEK_END);
	_read(fh,&end,1);
	_lseek(fh,-3,SEEK_END);
	_write(fh,&beg,1);
	_lseek(fh,0,SEEK_SET);
	_write(fh,&end,1);

	if (file_size%2==0){

		long int mid_pos= int(file_size/2-1);
		_lseek(fh,mid_pos,SEEK_SET);
		_read(fh,&beg,1);
		_read(fh,&end,1);
		_lseek(fh,-2,SEEK_CUR);
		_write(fh,&end,1);
		_write(fh,&beg,1);

	}

	_close(fh);
}


//---- Phase5 -- The cube phase -------------------------------------------------------------------
//	It puts the message in one cube wich the length of each side is equal to the length of --------
//the passowrd. Then rotate each matrix in the 3 dimensions. The rotations depend on the ----------
//password ----------------------------------------------------------------------------------------

void fase5(char* filename,char* password,int N, char mode, long int file_size){

	int i,j,k;
	char*** data_cube = new char** [N];
	for(i=0;i<N;i++){
		data_cube[i] = new char* [N];
		for(j=0;j<N;j++)
			data_cube[i][j] = new char [N];
	}

	 __int8 ** Rot_mat = new  __int8 * [N];
	for(i=0;i<N;i++)
		Rot_mat[i]=new  __int8[3];		//Number of rotations matrix (3 -> X Y Z)

	for(i=0;i<N;i++){
		Rot_mat[0][i]=( __int8(password[i])%4);
		Rot_mat[1][i]=(( __int8(password[i])%7)%4);
		Rot_mat[2][i]=((( __int8(password[i])%11)%7)%4);
	}

	int fh=_open(filename, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode

	while(1){

		for(i=0;i<N;i++)
			for(j=0;j<N;j++)
				for(k=0;k<N;k++){
					if(_eof(fh))
						goto stop;
					_read(fh,&data_cube[i][j][k],1);
				}

		if(mode=='c'){
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,Rot_mat[0][i],'X',i);
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,Rot_mat[0][i],'Y',i);
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,Rot_mat[0][i],'Z',i);
		}

		if(mode=='d'){
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,-Rot_mat[0][i],'Z',i);
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,-Rot_mat[0][i],'Y',i);
			for(i=0;i<N;i++)
				rotate_cube_matrix(data_cube,N,-Rot_mat[0][i],'X',i);
		}

		_lseek(fh,-(N*N*N),SEEK_CUR);

		for(i=0;i<N;i++)
			for(j=0;j<N;j++)
				for(k=0;k<N;k++){
					if(_eof(fh))
						goto stop;
					_write(fh,&data_cube[i][j][k],1);
				}

	}

	for(i=0;i<N;i++){
		for(j=0;j<N;j++)
			delete [] data_cube[i][j];
		delete [] data_cube[i];
	}
	delete [] data_cube;

	stop:
	_close(fh);

}

//Rotate a matrix by n times (n = 0 or 1 or 2 or 3) in the cube -----------------
//-- Reversed clockwise direction -----------------------------------------------
//-- N -> size of the cube, C->coord.to rotate(X Y Z), cor-> coord. of rotation -
void rotate_cube_matrix(char *** & p_c,int N,int n,char C,int cor){

	assert(cor<N);

	if(n!=0){

		int i,j;

		char** data_matrix=new char*[N]; //create the matrix
		for (i=0;i<N;i++)				 //where we store the data to encode
			data_matrix[i]=new char[N];

		char** p_m=new char*[N];		 //create the matrix
		for (i=0;i<N;i++)				 //where we store the data to encode
			p_m[i]=new char[N];

		switch(C){
		case 'X':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_m[i][j]=p_c[cor][i][j];
			break;

		case 'Y':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_m[i][j]=p_c[i][cor][j];
			break;

		case 'Z':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_m[i][j]=p_c[i][j][cor];
			break;
		default:assert(false);
		}

		if(n==2 || n==-2)
			for (i=0;i<N;i++)
				for(j=0;j<N;j++)
					data_matrix[i][j]=p_m[N-i-1][N-j-1];

		if(n==1 || n==-3)
			for (i=0;i<N;i++)
				for(j=0;j<N;j++)
					data_matrix[N-j-1][i]=p_m[i][j];

		if(n==3 || n==-1)
			for (i=0;i<N;i++)
				for(j=0;j<N;j++)
					data_matrix[j][N-i-1]=p_m[i][j];

		switch(C){
		case 'X':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_c[cor][i][j]=data_matrix[i][j];
			break;

		case 'Y':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_c[i][cor][j]=data_matrix[i][j];
			break;

		case 'Z':
			for(i=0;i<N;i++)
				for(j=0;j<N;j++)
					p_c[i][j][cor]=data_matrix[i][j];
			break;
		default:assert(false);
		}

		for (i=0;i<N;i++)				 //Deleting both temporary matrix
			delete [] data_matrix[i];
		delete [] data_matrix;

		for (i=0;i<N;i++)
			delete [] p_m[i];
		delete [] p_m;

	}
}//--- End of phase 5, The cube phase -------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-- Phase 6 -- Change the first bit of the msg :-) -----------------------------------------------
void fase6(char* file_name){

	int fh=_open(file_name, _O_RDWR );
	_setmode(fh,_O_BINARY);					//set to binary mode

	char data;

	_read(fh,&data,1);
	data=data^(0x80);
	_lseek(fh,-1,SEEK_CUR);
	_write(fh,&data,1);

	_close(fh);
}

long GetFileSize(std::string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}
