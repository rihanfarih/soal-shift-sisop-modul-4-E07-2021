#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#define SIZE 100000
#define ARR_SIZE 100000
 
static  const  char *dirpath = "/home/nizar/Downloads";
static const char *lognomer4 = "/home/nizar/SinSeiFS.log";
static const char *lognomer1 = "/home/nizar/Documents/sisopshift4/log1.log";
 
struct data{
    char path1[100];
    char path2[100];
};

struct rx {
    char DIR[SIZE];    
    int CHIPER;    
};

struct rx rx_directory[ARR_SIZE];
int rx_last_idx = 0;


char* atbash(char message[]) {
    char msg[1024] ;
    strcpy(msg, message) ;
    int x=strlen(msg);
    int i ;
    for(i = 0 ; i < x ; i++) {
        if (msg[i] >= 'A' && msg[i] <= 'Z') 
            msg[i] = 'Z' - (msg[i] - 'A') ;
        else if (msg[i] >= 'a' && msg[i] <= 'z') 
            msg[i] = 'z' - (msg[i] - 'a') ;
 
    }
    char* cipher = msg ;
    return cipher ;
}
 
// Bikin log
void makeLog(char *sys_call, struct data data){
    FILE * LOGFILE1 = fopen(lognomer1, "a");
    FILE * LOGFILE4 = fopen(lognomer4,"a");
	time_t now;
	time ( &now );
	struct tm * timeinfo = localtime (&now);
	
    if(strcmp(sys_call,"RENAME")==0){
        fprintf(LOGFILE4, "INFO::%d%02d%02d-%02d:%02d:%02d:%s::/%s::/%s\n",timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, sys_call, data.path1, data.path2);
        fprintf(LOGFILE1, "%s : %s -> %s\n", sys_call, data.path1, data.path2);	
    }else if(strcmp(sys_call,"MKDIR")==0 ){
    	fprintf(LOGFILE1, "%s : %s\n", sys_call, data.path1);
        fprintf(LOGFILE4, "INFO::%d%02d%02d-%02d:%02d:%02d:%s::/%s::/%s\n",timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, sys_call, data.path1, data.path2);
    }else if(strcmp(sys_call,"RMDIR")==0 || strcmp(sys_call,"UNLINK")==0){
        fprintf(LOGFILE4, "WARNING::%d%02d%02d-%02d:%02d:%02d:%s::/%s::/%s\n",timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, sys_call, data.path1, data.path2);
    }else{
        fprintf(LOGFILE4, "INFO::%d%02d%02d-%02d:%02d:%02d:%s::/%s::%s\n",timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, sys_call, data.path1, data.path2);
    }
    fclose(LOGFILE1);
    fclose(LOGFILE4);
    return;
}
 
// Untuk (Path Fuse Relatif) --> (Path Asli (/home/nizar/Downloads))
// /home/nizar/modul4/cobaa (Path Fuse Absolut) --> / (Path Fuse Relatif)
char* prosesPath(char* path) {
    char fpath[1024] ;
    bzero(fpath, 1024) ;
    int x, cekk = 0, cekaz;
 
    // Cek Apakah ada /AtoZ_ atau nggak...
    char *atoz;
    if (strcmp(path, "/") != 0) {
        atoz= strstr(path, "/AtoZ_");
        if (atoz) {
            // Jika ada, set cekk = 1 
            cekk = 1 ;
            cekaz = 1 ;
            // Geser pointer kekanan -> / nya ga kena
            atoz++ ;
        }
    }
     if(strcmp(path, "/") == 0)
    {
        path = dirpath;
        sprintf(fpath,"%s",path);
    } else if (cekk) {
        char realPath[1024] ;
        bzero(realPath, 1024) ;
        strncpy(realPath, path, strlen(path) - strlen(atoz)) ;
 
        //Antisipasi alamat asli keubah
        char t[1024] ;
        strcpy(t, atoz) ;
 
        char* encName;
        char* cut = t;
 
        char temp[1024] ;
 
        //Flag loop
       x = 0 ;
        while ((encName = strtok_r(cut, "/", &cut))) {
            bzero(temp, 1024) ;
            if(x == 0) {
                strcpy(temp, encName) ;
                // Sambungkan realpath
                strcat(realPath, temp) ;
                x = 1 ;
                continue ;
            }
 
            // Cek tipe data
            char isFullPath[1024] ;
            bzero(isFullPath, 1024) ;
            strcpy(isFullPath, realPath) ;
            strcat(isFullPath, "/") ; 
            strcat(realPath, "/") ;
            strcat(isFullPath, encName) ;
 
            if (strlen(isFullPath) == strlen(path)) {
                char pathFolder[1024] ;
                sprintf(pathFolder, "%s%s%s", dirpath, realPath, encName) ;
 
                DIR *dp = opendir(pathFolder);
                if (!dp) {
                    char *dot = strchr(encName, '.') ;
                    char fileName[1024] ;
                    bzero(fileName, 1024) ;
                    //Kalau ada extension
                    if (dot) {
                        // Untuk dapetin nama file nya doang
                        strncpy(fileName, encName, strlen(encName) - strlen(dot)) ;
                        strcpy(fileName, atbash(fileName)) ;
                        strcat(fileName, dot) ;
                    }
                    //Kalau ga ada extension
                    else {
                        strcpy(fileName, encName) ;
                        strcpy(fileName, atbash(fileName)) ;
                    }
                    strcat(realPath, fileName) ;
                    printf("%s\n", encName) ;
                }
                //sukses dibuka = benar ini folder
                else {
                    closedir(dp) ;
                    char folderName[1024] ;
                    bzero(folderName, 1024) ;
                    strcpy(folderName, encName) ;
                    strcpy(folderName, atbash(folderName)) ;
                    strcat(realPath, folderName) ;
                }
            }
            else {
                //Menyambungkan realpath dengan nama folder
                char folderName[1024] ;
                bzero(folderName, 1024) ;
                strcpy(folderName, encName) ;
                strcpy(folderName, atbash(folderName)) ;
                strcat(realPath, folderName) ;
            }
 
        }
        sprintf(fpath, "%s%s", dirpath, realPath) ; 
    }
    else sprintf(fpath, "%s%s",dirpath,path) ;
 
    char* return_fpath = fpath ;
    return return_fpath ;
}

//fungsi encode dengan viginere cipher
char *vignereEncrypt(char *str, char *key) {    
    char *str_copy = malloc((strlen(str)+1) * sizeof(char));
    sprintf(str_copy, "%s", str);
    char temp[SIZE]; sprintf(temp, "%s", str);

    int i = 0, curKey = 0;
    for(i = 0; i < strlen(str_copy); i++) {
        if(str_copy[i] >= 'a' && str_copy[i] <= 'z') {
            str_copy[i] = str_copy[i] - 'a' + 'A';
        }
    }

    for(int i = 0; i < strlen(str_copy); i++) {
        if(curKey == strlen(key)) curKey = 0;

        if(str_copy[i] >= 'A' && str_copy[i] <= 'Z')
            str_copy[i] = ((str_copy[i] + key[curKey]) % 26);
            
        if(temp[i] >= 'a' && temp[i] <= 'z')
            str_copy[i] += 'a';
        else if(temp[i] >= 'A' && temp[i] <= 'Z')
            str_copy[i] += 'A';
        else
            curKey--;
        
        curKey++;
    }

    str_copy[strlen(str)] = 0;
    return str_copy;
}
char* vignereDecrypt(char *str, char *key) {    
    char *str_copy = malloc((strlen(str)+1) * sizeof(char));
    sprintf(str_copy, "%s", str);
    char temp[SIZE]; sprintf(temp, "%s", str);

    int i = 0, curKey = 0;
    for(i = 0; i < strlen(str_copy); i++) {
        if(str_copy[i] >= 'a' && str_copy[i] <= 'z') {
            str_copy[i] = str_copy[i] - 'a' + 'A';
        }
    }

    for(int i = 0; i < strlen(str_copy); i++) {
        if(curKey == strlen(key)) curKey = 0;

        if(str_copy[i] >= 'A' && str_copy[i] <= 'Z') {
            str_copy[i] = str_copy[i] - key[curKey];

            if(str_copy[i] < 0)
                str_copy[i] += 26;
        }

        if(temp[i] >= 'a' && temp[i] <= 'z')
            str_copy[i] += 'a';
        else if(temp[i] >= 'A' && temp[i] <= 'Z')
            str_copy[i] += 'A';
        else
            curKey--;
        
        curKey++;
    }

    str_copy[strlen(str)] = 0;
    return str_copy;
}

//fungsi encode biner nomor 3
//ubah jadi lowercase insensitive
void ambilBiner(char *fname, char *bin, char *lowercase){
	int idAkhir = extensionId(fname);
	int idAwal = pemisahId(fname, 0);
	int i;
	
	for(i=idAwal; i<idAkhir; i++){
		if(isupper(fname[i])){
			bin[i] = '1';lowercase[i] = fname[i] + 32;
		}
		else{
			bin[i] = '0';lowercase[i] = fname[i];
		}
	}
	bin[idAkhir] = '\0';
	
	for(; i<strlen(fname); i++){
		lowercase[i] = fname[i];
	}
	lowercase[i] = '\0';
}

//encrypyt binary no 3
void encryptBinary(char *filepath){
	chdir(filepath);
	DIR *dp;
	struct dirent *dir;
	struct stat lol;
	dp = opendir(".");
	if(dp == NULL) return;
	
	char dirPath[1000];
	char filePath[1000];
	char filePathBinary[1000];
	
    while ((dir = readdir(dp)) != NULL){
		if (stat(dir->d_name, &lol) < 0);
		else if (S_ISDIR(lol.st_mode)){
			if (strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0) continue;sprintf(dirPath,"%s/%s",filepath, dir->d_name);encryptBinary(dirPath);
		}else{
			sprintf(filePath,"%s/%s",filepath, dir->d_name);
			char bin[1000], lowercase[1000]; ambilBiner(dir->d_name, bin, lowercase);
			int dec = bin2dec(bin);
			sprintf(filePathBinary,"%s/%s.%d",filepath,lowercase,dec); rename(filePath, filePathBinary);
		}
	}
    closedir(dp);
}
//decrypt binary no 3
void decryptBinary(char *filepath){
	chdir(filepath);
	DIR *dp;
	struct dirent *dir;
	struct stat lol;
	dp = opendir(".");
	if(dp == NULL) return;
	
	char dirPath[1000];
	char filePath[1000];
	char filePathDecimal[1000];
	
    while ((dir = readdir(dp)) != NULL){
		if (stat(dir->d_name, &lol) < 0);
		else if (S_ISDIR(lol.st_mode)){
			if (strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..") == 0) continue;sprintf(dirPath,"%s/%s",filepath, dir->d_name);decryptBinary(dirPath);
		}
		else{
			sprintf(filePath,"%s/%s",filepath, dir->d_name);
			char fname[1000], bin[1000], normalcase[1000], clearPath[1000];
			
			strcpy(fname, dir->d_name);
			char *ext = strrchr(fname, '.');
			int dec = convertDec(ext+1);
			for(int i=0; i<strlen(fname)-strlen(ext); i++) clearPath[i] = fname[i];
			
			char *ext2 = strrchr(clearPath, '.');
			dec2bin(dec, bin, strlen(clearPath)-strlen(ext2));getDecimal(clearPath, bin, normalcase);sprintf(filePathDecimal,"%s/%s",filepath,normalcase);rename(filePath, filePathDecimal);
		}
	}
    closedir(dp);
}

//ubah desimal
void getDecimal(char *fname, char *bin, char *normalcase){
	int idAkhir = extensionId(fname);
	int idAwal = pemisahId(fname, 0);
	int i;
	
	for(i=idAwal; i<idAkhir; i++){
		if(bin[i-idAwal] == '1') normalcase[i-idAwal] = fname[i] - 32;
		else normalcase[i-idAwal] = fname[i];
	}
	
	for(; i<strlen(fname); i++){
		normalcase[i-idAwal] = fname[i];
	}
	normalcase[i-idAwal] = '\0';
}

/* XMP Field */
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi)
{
    // path = /abcde
    char fpath[1000];
    bzero(fpath, 1000) ;
    int cekaz = 0 ;
    strcpy(fpath, prosesPath(path)) ;
 
    int res = 0 ;
    DIR *dp;
    struct dirent *de;
 
    (void) offset;
    (void) fi;
 
    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;
 
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
 
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            res = (filler(buf, de->d_name, &st, 0)) ;
        }
        else if (cekaz) {
            if (de->d_type & DT_DIR) {
                char temp[1024] ;
                bzero(temp, 1024) ;
                strcpy(temp, de->d_name) ;
                strcpy(temp, atbash(temp)) ;
                printf("readdir->%s\n", temp) ;
                res = (filler(buf, temp, &st, 0));
            }
            else {
                // cek ekstensi
                char* ext = strchr(de->d_name, '.') ;
                char fileName[1024] ;
                bzero(fileName, 1024) ;
                // kalau ada ekstensi
                if (ext) {
                    strncpy(fileName, de->d_name, strlen(de->d_name) - strlen(ext)) ;
                    strcpy(fileName, atbash(fileName)) ;
                    strcat(fileName, ext) ;
                }
                // kalau gaada
                else {
                    strcpy(fileName, de->d_name) ;
                    strcpy(fileName, atbash(fileName)) ;
                }
                res = (filler(buf, fileName, &st, 0));
            }
        }
        else res = (filler(buf, de->d_name, &st, 0));
 
        if(res!=0) break;
    }
 
    closedir(dp);
    return 0;
}
 
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    char fpath[1000];
    bzero(fpath, 1000) ;
    strcpy(fpath, prosesPath(path)) ;
 
    int fd;
    int res;
 
    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;
 
    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
 
    close(fd);
    return res;
}
 
static int xmp_getattr(const char *path, struct stat *stbuf)
{
    char fpath[1000];
    bzero(fpath, 1000) ;
    strcpy(fpath, prosesPath(path)) ;
 
    int res;
    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
 
    return 0;
}
 

static int xmp_rename(const char *from, const char *to){
    char fullFrom[1000],fullTo[1000];
 
    if(strcmp(from,"/") == 0)
	{
		from=dirpath;
		sprintf(fullFrom,"%s",from);
	}
	else sprintf(fullFrom, "%s%s",dirpath,from);
 
    if(strcmp(to,"/") == 0)
	{
		to=dirpath;
		sprintf(fullTo,"%s",to);
	}
	else sprintf(fullTo, "%s%s",dirpath,to);
 
    char *oldname = strrchr(fullFrom,'/')+1;
    char *newname = strrchr(fullTo,'/')+1;
 
	int res;
    printf("rename from = %s to = %s\n",fullFrom, fullTo);
	res = rename(fullFrom, fullTo);
	if (res == -1)
		return -errno;
 
    struct data input_data;
    strcpy(input_data.path1,fullFrom);
    strcpy(input_data.path2,fullTo);

    makeLog("RENAME",input_data);
	return 0;
}
 
 
static int xmp_mkdir(const char *path, mode_t mode)
{
    char* lastSlash = strchr(path, '/') ;
    if (strstr(lastSlash, "/AtoZ_")) {
        struct data inputdata;
        sprintf(inputdata.path1, "%s%s", dirpath, path);
        sprintf(inputdata.path2,NULL);
        makeLog("MKDIR", inputdata);
    }
 
    char fpath[1024] ;
    bzero(fpath, 1024) ;
    strcpy(fpath, prosesPath(path)) ;
    int res;
 
    res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;

    struct data inputdata;
    sprintf(inputdata.path1, "%s",fpath) ;
    sprintf(inputdata.path2,NULL);
    makeLog("MKDIR", inputdata);
    return 0;
}
 
 
static int xmp_mknod(const char *path, mode_t mode, dev_t rdev){
    char fpath[1000];
    if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
 
	int res;
 
    printf("mknod fpath = %s\n",fpath);

	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fpath, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;
 
    struct data input_data;
    strcpy(input_data.path1,"MKNOD");
    strcpy(input_data.path2,fpath);
    makeLog("MKNOD",input_data);
	return 0;
}
 
 

 
static int xmp_unlink(const char *path){

    char file[100];
    char fpath[1024] ;

    
    	
    if (strcmp(path, "/") == 0){
        path = dirpath;
        sprintf(fpath, "%s", path);
    }else{
        sprintf(fpath, "%s%s", dirpath, path);
        char* lastSlash = strchr(path, '/') ;
        if (strstr(lastSlash, "/AtoZ_")) {
           sprintf(fpath,"%s",prosesPath(path));
        }
    }

	int res;
 
    char *filename = strrchr(fpath,"/")+1;
 
	res = unlink(fpath);
	if (res == -1)
		return -errno;
 
    struct data data2;
    strcpy(data2.path1,fpath);
    strcpy(data2.path2,NULL);
    makeLog("UNLINK",data2);
	return 0;
}
 
 
 
 
 
static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .rename = xmp_rename,
    .mkdir = xmp_mkdir,
    .unlink = xmp_unlink,
    .mknod = xmp_mknod,
    .rename = xmp_rename
};
 
/* End XMP Field */
 
int  main(int  argc, char *argv[])
{
 
    umask(0);
 
    return fuse_main(argc, argv, &xmp_oper, NULL);
}