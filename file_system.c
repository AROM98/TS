#define FUSE_USE_VERSION 31

#include <stdio.h>
#include <errno.h>
#include <fuse.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h> 
#include <Python.h>
#include <string.h>


 
char cwd[PATH_MAX];
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

// guarda o uid (user) do dono de cada ficheiro
uid_t files_uid[256];
// guarda o gid (group) do dono de cada ficheiro
gid_t files_gid[256];

// guarda o uid (user) do dono de cada diretorio
uid_t directories_uid[256];
// guarda o gid (group) do dono de cada diretorio
gid_t directories_gid[256];


// para fazer os logs
static void log_message(const char *message) {
    openlog("FUSE-LOGS", LOG_PID, LOG_USER);  // Open connection to syslog
    syslog(LOG_INFO, "%s", message);  // Log the message using syslog
    closelog();  // Close connection to syslog
}

// para correr o script python OTP
int run_python_script() {
    
	

	//char* filename = "/home/pedro/TS/TS/otp.py";
	//char* filename = "/home/kubuntuu/Desktop/TS/TS/otp.py";
	char filename[PATH_MAX];
	strcpy(filename,cwd);

	char f[] = "/otp.py";
	strcat(filename,f);
    printf("file %s\n", filename);


	Py_Initialize();
	
	// Criar uma lista de argumentos
    PyObject *argList = PyList_New(0);
    PyObject *arg = PyUnicode_FromString(cwd);
    PyList_Append(argList, arg);

	// Passar a lista de argumentos para o Python
    PySys_SetObject("argv", argList);

    // Open the Python file
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
		perror("Error opening file");
        printf("Error opening script file\n");
        return -1;
    }

	

    // Run the Python script
    // if (PyRun_SimpleFileExFlags(file, filename, 1, NULL) == -1) {
    //     printf("Error running python script\n");
	// 	return -1;
    // }
	PyRun_SimpleFile(file, filename);

    Py_Finalize();
	return 0;
}



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
	// LOGS
	//printf("getattr called\n");
    log_message("[RSYSLOG] getattr called");
	
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
	//LOGS
	//printf("readdir called\n");
    log_message("[RSYSLOG] readdir called");

    int dir_id = is_dir(path);

	// verifica se o diretorio existe
    if(dir_id == -1)
        return -ENOENT;

	struct fuse_context *context;
    context = fuse_get_context();  // Get context

    // Check if the UID matches the directory's owner
    /*
	if(context->uid != directories_uid[dir_id]){
        return -EACCES;  // Return an "Access denied" error
	}
	*/
	

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
	
	//LOGS
	//printf("read called\n");
    log_message("[RSYSLOG] read called");

	int file_id = get_file( path );
	
	if ( file_id == -1 )
		return -1;
	
	// para verificar se o user tem permissao de ler o ficheiro (se é o dono neste caso)
	struct fuse_context *context;
    context = fuse_get_context();  // Get context

    // Check if the UID matches the file's owner
    if (context->uid != files_uid[file_id]) {
        return -EACCES;  // Return an "Access denied" error
    }
	else{
		// se é o dono, então tem que se autenticar como tal.
		int flag = run_python_script();
		if (flag == -1) return -EACCES;
	}


	printf("read command executed by (owner) user %d\n", context->uid);
	log_message("[RSYSLOG] read command executed by owner");

	char *content = files_content[ file_id ];
	
	memcpy( buffer, content + offset, size );
		
	return strlen( content ) - offset;
}


// adicionar o um novo diretorio
static int mkdir_act( const char *path, mode_t mode )
{	
	//LOGS
	//printf("mkdir called\n");
    log_message("[RSYSLOG] mkdir called");

	path++;


	dir_ind++;
	strcpy( directories[ dir_ind ], path);

	// para obter o uid e o gid do diretorio criado
	struct fuse_context *context;
    context = fuse_get_context();  // Get context
    directories_uid[dir_ind] = context->uid;
    directories_gid[dir_ind] = context->gid;
	
	return 0;
}

// adicionar um novo ficheiro
static int mknod_act( const char *path, mode_t mode, dev_t rdev )
{	
	//LOGS
	//printf("mknod called\n");
	log_message("[RSYSLOG] mknod called");

	path++;
	file_ind++;
	strcpy( files[ file_ind ],path );
	
	file_content_ind++;
	strcpy( files_content[ file_content_ind ], "" );

	// para obter o uid e o gid do ficheiro criado
	struct fuse_context *context;
    context = fuse_get_context();  // Get context
    files_uid[file_ind] = context->uid;
    files_gid[file_ind] = context->gid;
	
	return 0;
}

// escrever em um ficheiro
static int write_act( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	//LOGS
	//printf("write called\n");
	log_message("[RSYSLOG] write called");
	
	int file_idx = get_file( path );

	if ( file_idx == -1 ) 
		return;
		
	// para verificar se o user tem permissao de ler o ficheiro (se é o dono neste caso)
	struct fuse_context *context;
    context = fuse_get_context();  // Get context

	// Check if the UID matches the file's owner
    if (context->uid != files_uid[file_idx]) {
        return -EACCES;  // Return an "Access denied" error
    }
	else{
		// se é o dono, então tem que se autenticar como tal.
		int flag = run_python_script();
		if (flag == -1) return -EACCES;
	}

	
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

int main(int argc, char *argv[]){   

	if (getcwd(cwd, sizeof(cwd)) == NULL) {
	
    	perror("getcwd() error");
    	return 1;
	}
    printf("Current working dir: %s\n", cwd);


    // Create a new arguments array with the -f option, to run in foreground
    char *argv_mod[] = { argv[0], argv[1], "-f", NULL };
    return fuse_main(3, argv_mod, &operations, NULL);
    //return fuse_main(argc, argv, &hello_oper, NULL);
}
