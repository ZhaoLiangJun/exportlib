#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <getopt.h>

int Development = 0;
int Useldd = 0;
int UseCopy = 0;

int copyfile(char* src,char* dst){
  
    struct stat statbuffer;
    stat(src,&statbuffer);
    int filesize = statbuffer.st_size;
    
    if(access( src,F_OK ) != 0){
        perror("src file no found!");
        exit(1);
    }
    
    int src_fd = open(src,O_RDONLY);
    if( src_fd == -1 ){
        perror("open src file failed");
        exit(1);
    }
    
    int dst_fd = open(dst,O_CREAT | O_RDWR,0664);
    if(dst_fd == -1){
        perror("open dst file failed");
        exit(1);        
    }
    
    ftruncate( dst_fd,filesize );
    
    void* src_mm;
    char* dst_mm;
    
    src_mm = mmap(NULL,filesize,PROT_READ,MAP_SHARED,src_fd,0);
    if(src_mm == MAP_FAILED){
        perror("mmap1 failed");
        exit(1);
    }

    dst_mm = mmap(NULL,filesize,PROT_READ | PROT_WRITE,MAP_SHARED,dst_fd,0);
    if(dst_mm == MAP_FAILED){
        perror("mmap2 failed");
        exit(1);
    }
    
    memcpy( dst_mm,src_mm,filesize );
    
    munmap( dst_mm,filesize );
    munmap( src_mm,filesize );
    close(dst_fd);
    close(src_fd);
    
    return 0;
}

void export(const char* command)
{
  FILE* info;
  int num = 0;
  
  if( (info = popen(command,"r")) != NULL){
      char line[4096];
       
      char cwd_path[256] = {0};
      getcwd(cwd_path,sizeof(cwd_path));
      strcat( cwd_path,"/lib" );
      
      if(access(cwd_path,F_OK) == 0){
          char rmcmd[256] = {0};
          strcpy(rmcmd,"rm -rf ");
          strcat(rmcmd,cwd_path);
          system(rmcmd);
      }
                      
      if(access(cwd_path,F_OK) != 0){
          mkdir( cwd_path,0777 );
      }
      
      chdir(cwd_path);
      
      while(1){
          int success = fscanf(info,"%[^\n]\n",line);
          if(success == EOF) break;
          //printf( "%s\n",line );
          char* p = line;
          if((p = strstr( p,"=>")) !=NULL ){
              //printf( "%s\n",p );
              char* name = p - 1;
              *name = '\0';
              name = line;
              if((p = strchr( p,'/')) !=NULL ){
                  //printf( "%s\n",p );
                  char* first = p;
                  if((p = strchr( p,'(')) !=NULL ){
                      //printf( "%s\n",p );
                      char* finish = p - 1;
                      *finish = '\0';
                      printf( "%d = ",num += 1 );
                      printf( "%s ",first );
                      
                      struct stat statbuf;
                      lstat(first,&statbuf);
                      
                      int islink = 0;
                      char realname[256] = {0};
                      if( S_ISLNK(statbuf.st_mode) ){
                          //printf("filename is symlink!\n");
                          if( (readlink(first,realname,sizeof(realname))) == -1){
                              perror("read symlink failed");
                              exit(1);
                          }
                          islink = 1;
                          //printf("%s\n",realname);
                      }else{
                          strcpy(realname,name);
                          islink = 0;
                      }
                      getcwd(cwd_path,sizeof(cwd_path));
                      strcat( cwd_path,"/" );
                      strcat( cwd_path,realname );
                      printf( "%s\n",cwd_path );
                      
                      copyfile( first,cwd_path );
                      
                      if(islink){
                          char devlib[256] = {0};
                      
                          if(UseCopy){
                            copyfile(realname,name);
                            if(Development){
                                strcpy(devlib,realname);
                                if((p = strstr( devlib,".so")) !=NULL){
                                    p = p+3;
                                    if((memcmp(p,".",1)) == 0){
                                        *p = '\0';
                                        copyfile(realname,devlib);  
                                    }                     
                                }                     
                            }
                          }else{
                            if(access( name,F_OK ) == 0){
                                printf("%s file Exists!",name);
                                continue;
                            }
                            if(symlink(realname,name) == -1){
                                perror("realname syslink error");
                                exit(1);
                            }
                            if(Development){
                                strcpy(devlib,realname);
                                if((p = strstr( devlib,".so")) !=NULL){
                                    p = p+3;
                                    if((memcmp(p,".",1)) == 0){
                                        *p = '\0';
                                        //printf( "%s\n",devlib );
                                        if(symlink(realname,devlib) == -1){
                                            perror("devlib syslink error");
                                            exit(1);
                                        }   
                                    }                     
                                }                     
                            }
                          }
                      }
                  }
              }
          }
      }
      pclose(info);
  }
}

void usage(const char* arg){
    printf("Usage :\n\
           Export Library List!\n\
           \t%s [-c] [-x(0|1|2)] [-d] -p program\n\n\
           \t%s -c : SymLink Use Copy Command![Optional]\n\
           \t%s -x[0|1|2] : Export Use : {0:ldd|1: Built-in | 2: /lib/ld-linux.so.2 } [default : ldd] [Optional]\n\
           \t%s -d : Export Development Library![Optional]\n\
           \t%s -p Program : Export Runtime Library![Required]\n",arg,arg,arg,arg,arg);

}

int main(int argc,char** argv)
{
    int opt = 0;
    char* string = "d::x::c::p:";
    char command[256] = {0};
    
    while((opt=getopt(argc,argv,string))!=-1){
        switch(opt){
            case 'c':
                UseCopy = 1;
              break;
            case 'x':
                Useldd = atoi(optarg);
                if(Useldd > 2)
                    Useldd = 0;
              break;
            case 'd':
                Development = 1;
              break;
            case 'p':
                //printf("%c\n",opt);
                if(access(optarg,F_OK) == 0){
                    memset(command,0,sizeof(command));
                    
                    char cwd_path[256] = {0};
                    getcwd(cwd_path,sizeof(cwd_path));
                    strcat( cwd_path,"/bin" );
      
                    if(access(cwd_path,F_OK) == 0){
                        char rmcmd[256] = {0};
                        strcpy(rmcmd,"rm -rf ");
                        strcat(rmcmd,cwd_path);
                        system(rmcmd);
                    }
                      
                    if(access(cwd_path,F_OK) != 0){
                        mkdir( cwd_path,0777 );
                    }
                    if((memcmp( optarg,"/",1 )) == 0){
                        char* cmdname;
                        if((cmdname = strrchr( optarg,'/')) !=NULL ){
                            cmdname = cmdname + 1;
                            strcat(cwd_path,"/");
                            strcat(cwd_path,cmdname);
                            //printf("%s\n",cwd_path);
                        }
                    }else{
                            strcat(cwd_path,"/");
                            strcat(cwd_path,optarg);                    
                    }
                    
                    copyfile(optarg,cwd_path);
                    
                    if(Useldd == 0){
                        strcpy( command,"ldd " );
                    }else if(Useldd == 1){
                        strcpy( command,"eval LD_TRACE_LOADED_OBJECTS=1 ");       
                    }else if(Useldd == 2){
                        strcpy( command,"/lib/ld-linux.so.2 --list ./");
                    }
                    strcat(command,optarg);

                    export(command);
                    
                    chdir("..");
                    char* shell = "#!/bin/bash\n\nexport PATH=bin:$PATH\nexport LD_LIBRARY_PATH=lib:$LD_LIBRARY_PATH\n";
                    int fd;
                    if((fd = open("env.sh",O_CREAT | O_RDWR,0777))==-1){
                        perror("No open file");
                        exit(1);
                    }
                    write(fd,shell,strlen(shell));
                    close(fd);
                    chmod( argv[0],0777);
                    chmod("env.sh",0777);
                     
                }else{
                    printf("%s Program No Found!\n",optarg);
                }

              break;
            default:
                usage(argv[0]);
        }
    
    }
    //printf("%d\n",optind);
    if(optind < 2){
        usage(argv[0]);
    }
    
    return 0;

}

