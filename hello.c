#define FUSE_USE_VERSION 31

#include <stdio.h>
#include <errno.h>
#include <fuse.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>


 

// guarda os ficheiros criados
char files[ 256 ][ 256 ];
// indice
int file_ind = -1;

// guarda o conteúdo de cada ficheiro 
char files_content[ 256 ][ 256 ];
// indice
int file_content_ind = -1;

// guarda as diretorias criadas
char directories[ 256 ][ 256 ];
//indice
int dir_ind = -1;


// verifica se o diretorio ja foi um dos criados
int is_dir( const char *path )
{
	path++; 
	
	for ( int i = 0; i <= dir_ind; i++ )
		if ( strcmp( path, directories[ i ] ) == 0 )
			return 1;
	
	return 0;
}

// retorna o indice do ficheiro
int get_file( const char *path ){
	
	path++; 
	int file = -1;
	int i=0;
	while(file == -1 && i <= file_ind){
		if ( strcmp( path, files[ i ] ) == 0 ) file = i;
		i++;
	}

	return file;
}



// definir os atributos dos ficheiros e diretorios
static int getattr_act( const char *path, struct stat *st )
{
	
	// definição das permissoes 
	st->st_uid = getuid(); 
	st->st_gid = getgid(); 
	st->st_atime = time( NULL ); 
	st->st_mtime = time( NULL ); 
	

	if ( strcmp( path, "/" ) == 0 || is_dir(path) == 1 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; 
	}
	else if ( get_file( path ) != -1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	else
	{
		return -ENOENT;
	}
	
	return 0;
}


// ler os atributos dos ficheiros e diretorios
static int readdir_act( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	filler( buffer, ".", NULL, 0 ,0); 
	filler( buffer, "..", NULL, 0 ,0); 
	
	if ( strcmp( path, "/" ) == 0 ) 
	{
		for ( int i = 0; i <= dir_ind; i++ )
			filler( buffer, directories[ i ], NULL, 0 ,0);
	
		for ( int i = 0; i <= file_ind; i++ )
			filler( buffer, files[ i ], NULL, 0,0 );
	}
	
	return 0;
}

// realizar a leitura do ficheiro
static int read_act( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi ){
	int file_id = get_file( path );
	
	if ( file_id == -1 )
		return -1;
	
	char *content = files_content[ file_id ];
	
	memcpy( buffer, content + offset, size );
		
	return strlen( content ) - offset;
}


// adicionar o um novo diretorio
static int mkdir_act( const char *path, mode_t mode )
{
	path++;
	dir_ind++;
	strcpy( directories[ dir_ind ], path);
	
	return 0;
}

// adicionar um novo ficheiro
static int mknod_act( const char *path, mode_t mode, dev_t rdev )
{
	path++;
	file_ind++;
	strcpy( files[ file_ind ],path );
	
	file_content_ind++;
	strcpy( files_content[ file_content_ind ], "" );
	
	return 0;
}

// escrever em um ficheiro
static int write_act( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	
	int file_idx = get_file( path );
	
	if ( file_idx == -1 ) 
		return;
		
	strcpy( files_content[ file_idx ], buffer ); 
	
	return size;
}

static struct fuse_operations operations = {
    .getattr	= getattr_act,
    .readdir	= readdir_act,
    .read		= read_act,
    .mkdir		= mkdir_act,
    .mknod		= mknod_act,
    .write		= write_act,
};

int main(int argc, char *argv[])
{   
    // Create a new arguments array with the -f option, to run in foreground
    char *argv_mod[] = { argv[0], argv[1], "-f", NULL };
    return fuse_main(3, argv_mod, &operations, NULL);
    //return fuse_main(argc, argv, &hello_oper, NULL);
}
